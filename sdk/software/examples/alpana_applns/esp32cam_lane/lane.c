/* ============================================================================
 * lane.c v2 : lane detection for ALPANA_SOC (80x60 lane-feature image, C + FPU)
 *
 *  Input: 80x60 image from the ESP32-CAM in which paint is bright
 *         (luma + yellow boost, see the sketch). Dark tape on a light floor is
 *         handled by an automatic polarity switch.
 *
 *  1. polarity (every 8th frame): bright vs dark lane-width ridges on 2 rows
 *     (shadows and glare spots do not flip it), with hysteresis
 *  2. 12 scan rows. Per row:
 *       - local background = 33 px box mean (prefix sums)  -> shadows and
 *         uneven light do not matter, only "brighter than the road around it"
 *       - noise = mean of the negative deviations (the road, tracked per
 *         row from frame to frame), threshold = 2.5 x noise (min 12 grey
 *         levels) -> faint / faded yellow is kept, no global "half of the
 *         brightest pixel" threshold any more
 *       - [1 2 1] smoothed runs above the threshold, width limit per row,
 *         ridge test (paint must be brighter than the road on both sides,
 *         rejects shadow/sun edges)
 *  3. candidate choice: near the line predicted from the last frames (gate),
 *     else the nearest run left/right of the tracked lane centre
 *  4. least-squares line x = a + b*y per side (FPU), 1 outlier pass
 *  5. sanity (lines must not cross, width plausible once learned),
 *     temporal smoothing, a lost line is held for up to 5 frames
 *     (dashed lines, short dropouts)
 *  6. steering = weighted centre offset -> -30..+30 deg, smoothed
 *
 *  On ALPANA the per-pixel code (scan_row) and its buffers live in on-chip boot RAM
 *  (ALPANA_FAST / ALPANA_FAST_BSS, see alpana.h) because the core has no cache.
 * ========================================================================== */
#include "lane.h"
#ifdef __riscv
#include "alpana.h"
#define LFAST     ALPANA_FAST
#define LFAST_BSS ALPANA_FAST_BSS
#else
#define LFAST
#define LFAST_BSS
#endif

#define MIN_CONTRAST 12           /* grey levels: weakest marking accepted */
#define NOISE_K_NUM  5            /* threshold = 5/2 x road noise          */
#define NOISE_K_DEN  2
#define BG_R         16           /* background window: 2*16+1 = 33 px     */
#define BG_N         (2 * BG_R + 1)
#define RIDGE_K      3            /* road samples each side for the ridge test */
#define MAX_RUNS     8
#define FIT_TOL      3.0f         /* px: outlier distance from the fitted line */
#define GATE_TOP     5            /* px: search gate around the predicted line (far row) */
#define GATE_BOT     9            /*     ... (nearest row) */
#define MAXW_TOP     7            /* px: widest marking accepted (far row) */
#define MAXW_BOT     20           /*     ... (nearest row) */
#define HOLD_FRAMES  5            /* keep a lost line this many frames */
#define GAIN         1.8f         /* steering gain */
#define DEADBAND     6            /* deg: below this -> STRAIGHT */

typedef struct { float a, b; int valid, age; } line_t;
typedef struct { int16_t cx, w; } run_t;
typedef uint32_t __attribute__((may_alias)) u32a;

static float w_top, w_bot;                   /* learned lane width (px) at top/bottom rows */
static int   w_conf;                         /* how many frames the width was learned from */
static float steer_f;                        /* smoothed steering */
static float track_c[LANE_ROWS];             /* expected lane centre per scan row */
static int   lost, dark_state, dark_votes, first, pol_cnt;
static line_t Ls, Rs;

LFAST_BSS static int32_t P[LW + 1];          /* prefix sums of one row */
LFAST_BSS static uint8_t R[LW] __attribute__((aligned(4)));   /* the scan row */
static int32_t noise_row[LANE_ROWS];         /* road noise per scan row (last frame) */

static inline int row_y(int i) { return LANE_Y_BOT - i * (LANE_Y_BOT - LANE_Y_TOP) / (LANE_ROWS - 1); }

void lane_init(void) {
  for (int i = 0; i < LANE_ROWS; i++) track_c[i] = LW / 2;
  w_top = 34.0f; w_bot = 62.0f; w_conf = 0; steer_f = 0.0f;
  lost = 0; dark_state = 0; dark_votes = 0; first = 1; pol_cnt = 0;
  Ls.valid = Rs.valid = 0; Ls.age = Rs.age = 0;
  for (int i = 0; i < LANE_ROWS; i++) noise_row[i] = 4 * BG_N;
}

/* least squares x = a + b*y with one outlier-rejection pass; returns points used (0 = no line) */
LFAST static int fit_line(const int16_t *xs, const int16_t *ys, float *a, float *b) {
  int use[LANE_ROWS], m = 0;
  for (int i = 0; i < LANE_ROWS; i++) { use[i] = xs[i] >= 0; m += use[i]; }
  for (int pass = 0; pass < 2; pass++) {
    if (m < 3) return 0;
    float sy = 0, sx = 0, syy = 0, sxy = 0;
    for (int i = 0; i < LANE_ROWS; i++) if (use[i]) {
      float y = ys[i], x = xs[i];
      sy += y; sx += x; syy += y * y; sxy += x * y;
    }
    float d = m * syy - sy * sy;
    if (d < 1e-3f && d > -1e-3f) return 0;
    *b = (m * sxy - sx * sy) / d;
    *a = (sx - *b * sy) / m;
    if (pass == 0) {
      m = 0;
      for (int i = 0; i < LANE_ROWS; i++) if (use[i]) {
        float r = xs[i] - (*a + *b * ys[i]);
        if (r > FIT_TOL || r < -FIT_TOL) use[i] = 0; else m++;
      }
    }
  }
  return m;
}

/* ridge test for the run [x0, x1): mean(run) - mean(road beside it) >= thr/2 on every side that exists */
static inline __attribute__((always_inline)) int ridge_ok(int x0, int x1, int32_t thr, int dark) {
  int w = x1 - x0, sides = 0;
  int32_t in = P[x1] - P[x0], need = thr * RIDGE_K * w;   /* both sides scaled by 2*BG_N*K*w */
  int l1 = x0 - 1, l0 = l1 - RIDGE_K;                      /* left road  [l0, l1) */
  int r0 = x1 + 1, r1 = r0 + RIDGE_K;                      /* right road [r0, r1) */
  if (l0 >= 0) {
    int32_t diff = in * RIDGE_K - (P[l1] - P[l0]) * w; if (dark) diff = -diff;
    if (diff * 2 * BG_N < need) return 0;
    sides++;
  }
  if (r1 <= LW) {
    int32_t diff = in * RIDGE_K - (P[r1] - P[r0]) * w; if (dark) diff = -diff;
    if (diff * 2 * BG_N < need) return 0;
    sides++;
  }
  return sides;
}

LFAST static void add_run(int x0, int x1, int maxw, int32_t thr, int dark, run_t *runs, int *n) {
  if (x1 - x0 <= maxw && *n < MAX_RUNS && ridge_ok(x0, x1, thr, dark)) {
    runs[*n].cx = (int16_t)((x0 + x1 - 1) / 2);
    runs[*n].w  = (int16_t)(x1 - x0);
    (*n)++;
  }
}

/* one pixel: deviation from the local background, noise statistics,
   [1 2 1] smoothed value at X-1 compared with the threshold -> runs */
#define STEP(X, BG) do {                                                      \
    int32_t d_ = (int32_t)R[X] * BG_N - (BG);                                  \
    d_ = (d_ ^ m) - m;                                                         \
    if (d_ < 0) { neg -= d_; nneg++; }                                         \
    if (dpp + 2 * dp + d_ >= t4) { if (!inrun) { inrun = 1; x0 = (X) - 1; } } \
    else if (inrun) { inrun = 0; add_run(x0, (X) - 1, maxw, thr, dark, runs, &n); } \
    dpp = dp; dp = d_;                                                         \
  } while (0)

/* one scan row -> runs of "brighter than the local road" pixels (centre x, width).
   thr = detection threshold (x BG_N) from the previous frame's noise of this row,
   so everything is done in one pass; returns the new noise estimate in *noise_out */
LFAST static int scan_row(const uint8_t *row, int dark, int maxw, int32_t thr, run_t *runs, int32_t *noise_out) {
  /* row -> R[] (4 pixels per DDR2 load) and prefix sums P[] */
  const u32a *rw = (const u32a *)row;                       /* rows are 4-byte aligned */
  u32a *Rw = (u32a *)R;
  int32_t acc = 0;
  P[0] = 0;
  for (int k = 0; k < LW / 4; k++) {
    uint32_t w = rw[k];
    int32_t *pp = &P[4 * k + 1];
    Rw[k] = w;
    acc += w & 0xFF;         pp[0] = acc;
    acc += (w >> 8) & 0xFF;  pp[1] = acc;
    acc += (w >> 16) & 0xFF; pp[2] = acc;
    acc += w >> 24;          pp[3] = acc;
  }
  /* 33 px background: the window is clamped at the image edges */
  int32_t neg = 0, t4 = 4 * thr, m = dark ? -1 : 0;        /* d = (d ^ m) - m flips the sign */
  int nneg = 0, n = 0, inrun = 0, x0 = 0, x;
  int32_t bgL = P[BG_N], bgR = P[LW] - P[LW - BG_N];
  int32_t dpp = (int32_t)R[0] * BG_N - bgL, dp = (int32_t)R[1] * BG_N - bgL;
  dpp = (dpp ^ m) - m; dp = (dp ^ m) - m;
  if (dpp < 0) { neg -= dpp; nneg++; }
  if (dp < 0)  { neg -= dp;  nneg++; }
  for (x = 2; x <= BG_R; x++)      STEP(x, bgL);
  for (; x < LW - BG_R; x++)       STEP(x, P[x + BG_R + 1] - P[x - BG_R]);
  for (; x < LW; x++)              STEP(x, bgR);
  if (inrun) add_run(x0, LW - 1, maxw, thr, dark, runs, &n);
  *noise_out = nneg ? neg / nneg : 0;                       /* x BG_N */
  return n;
}
#undef STEP

static inline int32_t thr_from_noise(int32_t noise) {
  int32_t thr = noise * NOISE_K_NUM / NOISE_K_DEN;
  return thr < MIN_CONTRAST * BG_N ? MIN_CONTRAST * BG_N : thr;
}

static void line_update(line_t *L, int n, float a, float b) {
  if (n) {
    if (L->valid) { L->a = 0.3f * L->a + 0.7f * a; L->b = 0.3f * L->b + 0.7f * b; }
    else          { L->a = a; L->b = b; }
    L->valid = 1; L->age = 0;
  } else if (L->valid && ++L->age > HOLD_FRAMES) {
    L->valid = 0;
  }
}

static float width_at(float y) {             /* lane width model, linear in y */
  float t = (y - LANE_Y_TOP) / (float)(LANE_Y_BOT - LANE_Y_TOP);
  return w_top + t * (w_bot - w_top);
}

void lane_detect(const uint8_t img[LH][LW], lane_result_t *r) {
  run_t runs[MAX_RUNS];
  /* ---- 1. polarity (every 8th frame): on 2 rows count ridges that are brighter /
     darker than their surroundings (lane-width runs only, wide shadows do not count) ---- */
  if (first || (++pol_cnt & 7) == 0) {
    int nb = 0, nd = 0; int32_t lum = 0, n_;
    for (int i = 2; i < LANE_ROWS; i += 5) {
      int y = row_y(i), maxw = MAXW_BOT - i * (MAXW_BOT - MAXW_TOP) / (LANE_ROWS - 1);
      int32_t thr = thr_from_noise(noise_row[i]);
      nb += scan_row(img[y], 0, maxw, thr, runs, &n_);
      lum += P[LW];
      nd += scan_row(img[y], 1, maxw, thr, runs, &n_);
    }
    int dark_now = nd > nb && lum > 90 * 2 * LW;             /* dark tape needs a light floor */
    if (first) { dark_state = dark_now; first = 0; }
    else if (dark_now != dark_state) {
      if (++dark_votes >= 2) { dark_state = dark_now; dark_votes = 0; Ls.valid = Rs.valid = 0; }
    } else dark_votes = 0;
  }
  int dark = dark_state;
  r->dark_lines = dark;

  /* ---- 2./3. scan rows, choose the left/right marking ---- */
  int16_t ys[LANE_ROWS], lxs[LANE_ROWS], rxs[LANE_ROWS];
  int32_t thr_sum = 0, noise_sum = 0;
  for (int i = 0; i < LANE_ROWS; i++) {
    int y = row_y(i);
    int maxw = MAXW_BOT - i * (MAXW_BOT - MAXW_TOP) / (LANE_ROWS - 1);
    float gate = GATE_BOT - i * (float)(GATE_BOT - GATE_TOP) / (LANE_ROWS - 1);
    int32_t thr = thr_from_noise(noise_row[i]), noise;
    int n = scan_row(img[y], dark, maxw, thr, runs, &noise);
    noise_row[i] = noise;
    thr_sum += thr; noise_sum += noise;
    float pl = Ls.a + Ls.b * y, pr = Rs.a + Rs.b * y, dl = 1e9f, dr = 1e9f;
    int cc = (int)track_c[i], bl = -1, br = -1;
    for (int k = 0; k < n; k++) {                         /* runs are in ascending x */
      int cx = runs[k].cx;
      if (Ls.valid) { float d = cx - pl; if (d < 0) d = -d; if (d <= gate && d < dl) { dl = d; bl = cx; } }
      else if (cx < cc) bl = cx;                          /* last run left of centre = nearest */
      if (Rs.valid) { float d = cx - pr; if (d < 0) d = -d; if (d <= gate && d < dr) { dr = d; br = cx; } }
      else if (cx >= cc && br < 0) br = cx;               /* first run right of centre = nearest */
    }
    if (bl >= 0 && br >= 0 && bl >= br) {                 /* same run picked twice / crossed */
      if (Ls.valid && Rs.valid) { if (dl <= dr) br = -1; else bl = -1; }
      else if (bl < cc) br = -1; else bl = -1;
    }
    ys[i] = y; lxs[i] = bl; rxs[i] = br;
    r->ly[i] = y; r->lx[i] = bl; r->rx[i] = br;
  }
  r->thr   = thr_sum   / (LANE_ROWS * BG_N);
  r->noise = noise_sum / (LANE_ROWS * BG_N);

  /* ---- 4. line fits ---- */
  float la = 0, lb = 0, ra = 0, rb = 0;
  int nl = fit_line(lxs, ys, &la, &lb);
  int nr = fit_line(rxs, ys, &ra, &rb);

  /* ---- 5. sanity, temporal filter ---- */
  if (nl && nr) {
    float wb = (ra + rb * LANE_Y_BOT) - (la + lb * LANE_Y_BOT);
    float wt = (ra + rb * LANE_Y_TOP) - (la + lb * LANE_Y_TOP);
    int bad = wb < 12 || wt < 4;
    if (!bad && w_conf >= 5) bad = wb < 0.6f * w_bot || wb > 1.6f * w_bot;
    if (bad) {                                            /* keep the side with more support */
      if (nl > nr) nr = 0;
      else if (nr > nl) nl = 0;
      else {                                              /* tie: keep the one nearer its prediction */
        float el = Ls.valid ? (la + lb * LANE_Y_BOT) - (Ls.a + Ls.b * LANE_Y_BOT) : 99.0f;
        float er = Rs.valid ? (ra + rb * LANE_Y_BOT) - (Rs.a + Rs.b * LANE_Y_BOT) : 99.0f;
        if (el < 0) el = -el;
        if (er < 0) er = -er;
        if (el <= er) nr = 0; else nl = 0;
      }
    } else {                                              /* learn the lane width */
      w_bot = 0.85f * w_bot + 0.15f * wb;
      w_top = 0.85f * w_top + 0.15f * wt;
      if (w_conf < 100) w_conf++;
    }
  }
  line_update(&Ls, nl, la, lb);
  line_update(&Rs, nr, ra, rb);
  if (Ls.valid && Rs.valid && (nl == 0 || nr == 0)) {     /* a held line must not cross the measured one */
    float wb = (Rs.a + Rs.b * LANE_Y_BOT) - (Ls.a + Ls.b * LANE_Y_BOT);
    if (wb < 12) { if (nl == 0) Ls.valid = 0; else Rs.valid = 0; }
  }
  r->left_ok  = Ls.valid ? (nl ? 1 : 2) : 0;
  r->right_ok = Rs.valid ? (nr ? 1 : 2) : 0;
  r->la = Ls.a; r->lb = Ls.b; r->ra = Rs.a; r->rb = Rs.b;

  /* ---- 6. lane centre, steering ---- */
  float yt = LANE_Y_TOP, yb = LANE_Y_BOT;
  if (Ls.valid && Rs.valid) {
    r->cx_top = 0.5f * ((Ls.a + Ls.b * yt) + (Rs.a + Rs.b * yt));
    r->cx_bot = 0.5f * ((Ls.a + Ls.b * yb) + (Rs.a + Rs.b * yb));
  } else if (Ls.valid) {
    r->cx_top = Ls.a + Ls.b * yt + 0.5f * width_at(yt);
    r->cx_bot = Ls.a + Ls.b * yb + 0.5f * width_at(yb);
  } else if (Rs.valid) {
    r->cx_top = Rs.a + Rs.b * yt - 0.5f * width_at(yt);
    r->cx_bot = Rs.a + Rs.b * yb - 0.5f * width_at(yb);
  }

  if (Ls.valid || Rs.valid) {
    lost = 0;
    float e = 0.6f * (r->cx_top - LW / 2) / (LW / 2) + 0.4f * (r->cx_bot - LW / 2) / (LW / 2);
    float s = e * GAIN * STEER_MAX_DEG;
    if (s > STEER_MAX_DEG) s = STEER_MAX_DEG;
    if (s < -STEER_MAX_DEG) s = -STEER_MAX_DEG;
    steer_f = 0.6f * steer_f + 0.4f * s;
    for (int i = 0; i < LANE_ROWS; i++) {                 /* track the centre for the next frame */
      float t = (row_y(i) - yt) / (yb - yt);
      float cx = r->cx_top + t * (r->cx_bot - r->cx_top);
      if (cx < 8) cx = 8;
      if (cx > LW - 8) cx = LW - 8;
      track_c[i] = cx;
    }
  } else {
    if (++lost > 6) for (int i = 0; i < LANE_ROWS; i++) track_c[i] = LW / 2;
    steer_f *= 0.85f;                                     /* drift back to straight */
    r->cx_top = r->cx_bot = LW / 2;
  }
  r->steer_deg = (int)(steer_f >= 0 ? steer_f + 0.5f : steer_f - 0.5f);
  if (lost > 3) r->dir = DIR_LOST;
  else if (r->steer_deg >  DEADBAND) r->dir = DIR_RIGHT;
  else if (r->steer_deg < -DEADBAND) r->dir = DIR_LEFT;
  else r->dir = DIR_STRAIGHT;
}
