/*
 * Deterministic random stress kernels, compiled twice:
 *   - for the host (-DHOST) to produce golden checksums with exact RISC-V semantics emulated in C
 *   - for ALPANA (RV32IMAFC) where every op is forced onto the real instruction via inline asm
 */
#include <stdint.h>

static inline uint32_t xs32(uint32_t *s) { uint32_t x = *s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return *s = x; }
static inline uint32_t f2u(float f) { union { float f; uint32_t u; } x; x.f = f; return x.u; }
static inline float u2f(uint32_t u) { union { float f; uint32_t u; } x; x.u = u; return x.f; }
static inline int isnan_u(uint32_t u) { return (u & 0x7f800000u) == 0x7f800000u && (u & 0x7fffffu); }
static inline uint32_t canon(uint32_t u) { return isnan_u(u) ? 0x7fc00000u : u; }  /* RISC-V canonical NaN */
#define MIX(h, v) ((h) = ((h) ^ (uint32_t)(v)) * 0x01000193u)

#ifdef HOST
#include <math.h>
static int32_t  rv_div(int32_t a, int32_t b)  { if (!b) return -1; if (a == INT32_MIN && b == -1) return a; return a / b; }
static uint32_t rv_divu(uint32_t a, uint32_t b){ return b ? a / b : 0xffffffffu; }
static int32_t  rv_rem(int32_t a, int32_t b)  { if (!b) return a; if (a == INT32_MIN && b == -1) return 0; return a % b; }
static uint32_t rv_remu(uint32_t a, uint32_t b){ return b ? a % b : a; }
static uint32_t rv_mulh(int32_t a, int32_t b)   { return (uint32_t)(((int64_t)a * b) >> 32); }
static uint32_t rv_mulhu(uint32_t a, uint32_t b){ return (uint32_t)(((uint64_t)a * b) >> 32); }
static uint32_t rv_mulhsu(int32_t a, uint32_t b){ return (uint32_t)(((int64_t)a * (int64_t)(uint64_t)b) >> 32); }
static uint32_t rv_sra(int32_t a, uint32_t s)  { return (uint32_t)(a >> (s & 31)); }
static uint32_t f_add(uint32_t a, uint32_t b) { return canon(f2u(u2f(a) + u2f(b))); }
static uint32_t f_sub(uint32_t a, uint32_t b) { return canon(f2u(u2f(a) - u2f(b))); }
static uint32_t f_mul(uint32_t a, uint32_t b) { return canon(f2u(u2f(a) * u2f(b))); }
static uint32_t f_div(uint32_t a, uint32_t b) { return canon(f2u(u2f(a) / u2f(b))); }
static uint32_t f_sqrt(uint32_t a)            { return canon(f2u(sqrtf(u2f(a)))); }
static uint32_t f_madd(uint32_t a, uint32_t b, uint32_t c) { return canon(f2u(fmaf(u2f(a), u2f(b), u2f(c)))); }
static uint32_t f_nmsub(uint32_t a, uint32_t b, uint32_t c){ return canon(f2u(fmaf(-u2f(a), u2f(b), u2f(c)))); }
static uint32_t f_minmax(uint32_t a, uint32_t b, int mx) {
  int na = isnan_u(a), nb = isnan_u(b);
  if (na && nb) return 0x7fc00000u; if (na) return b; if (nb) return a;
  float x = u2f(a), y = u2f(b);
  if (x == y) return mx ? (a & b) : (a | b);   /* +0/-0: max -> +0, min -> -0 */
  return mx ? (x > y ? a : b) : (x < y ? a : b);
}
static uint32_t f_cmp(uint32_t a, uint32_t b) { float x = u2f(a), y = u2f(b); return (x == y) | ((x < y) << 1) | ((x <= y) << 2); }
static int32_t  f_cvt_ws(uint32_t a) {
  float x = u2f(a);
  if (isnan_u(a) || x >= 2147483648.0f) return INT32_MAX;
  if (x < -2147483648.0f) return INT32_MIN;
  return (int32_t)x;  /* rtz */
}
static uint32_t f_cvt_sw(int32_t a) { return f2u((float)a); }
static uint32_t f_sgnjx(uint32_t a, uint32_t b) { return a ^ (b & 0x80000000u); }
#else
#define R2(op) ({ uint32_t r; asm volatile(op " %0,%1,%2" : "=r"(r) : "r"(a), "r"(b)); r; })
static inline int32_t  rv_div(int32_t a, int32_t b)   { return (int32_t)R2("div"); }
static inline uint32_t rv_divu(uint32_t a, uint32_t b){ return R2("divu"); }
static inline int32_t  rv_rem(int32_t a, int32_t b)   { return (int32_t)R2("rem"); }
static inline uint32_t rv_remu(uint32_t a, uint32_t b){ return R2("remu"); }
static inline uint32_t rv_mulh(int32_t a, int32_t b)  { return R2("mulh"); }
static inline uint32_t rv_mulhu(uint32_t a, uint32_t b){ return R2("mulhu"); }
static inline uint32_t rv_mulhsu(int32_t a, uint32_t b){ return R2("mulhsu"); }
static inline uint32_t rv_sra(int32_t a, uint32_t b)  { return R2("sra"); }
#ifdef __riscv_flen
#define F2(op) ({ float r; asm volatile(op " %0,%1,%2" : "=f"(r) : "f"(u2f(a)), "f"(u2f(b))); canon(f2u(r)); })
#define F3(op) ({ float r; asm volatile(op " %0,%1,%2,%3" : "=f"(r) : "f"(u2f(a)), "f"(u2f(b)), "f"(u2f(c))); canon(f2u(r)); })
static inline uint32_t f_add(uint32_t a, uint32_t b) { return F2("fadd.s"); }
static inline uint32_t f_sub(uint32_t a, uint32_t b) { return F2("fsub.s"); }
static inline uint32_t f_mul(uint32_t a, uint32_t b) { return F2("fmul.s"); }
static inline uint32_t f_div(uint32_t a, uint32_t b) { return F2("fdiv.s"); }
static inline uint32_t f_sqrt(uint32_t a) { float r; asm volatile("fsqrt.s %0,%1" : "=f"(r) : "f"(u2f(a))); return canon(f2u(r)); }
static inline uint32_t f_madd(uint32_t a, uint32_t b, uint32_t c) { return F3("fmadd.s"); }
static inline uint32_t f_nmsub(uint32_t a, uint32_t b, uint32_t c){ return F3("fnmsub.s"); }
static inline uint32_t f_minmax(uint32_t a, uint32_t b, int mx) { return mx ? F2("fmax.s") : F2("fmin.s"); }
static inline uint32_t f_cmp(uint32_t a, uint32_t b) {
  uint32_t e, l, le; float x = u2f(a), y = u2f(b);
  asm volatile("feq.s %0,%3,%4\n flt.s %1,%3,%4\n fle.s %2,%3,%4" : "=&r"(e), "=&r"(l), "=&r"(le) : "f"(x), "f"(y));
  return e | (l << 1) | (le << 2);
}
static inline int32_t f_cvt_ws(uint32_t a) { int32_t r; asm volatile("fcvt.w.s %0,%1,rtz" : "=r"(r) : "f"(u2f(a))); return r; }
static inline uint32_t f_cvt_sw(int32_t a) { float r; asm volatile("fcvt.s.w %0,%1" : "=f"(r) : "r"(a)); return f2u(r); }
static inline uint32_t f_sgnjx(uint32_t a, uint32_t b) { float r; asm volatile("fsgnjx.s %0,%1,%2" : "=f"(r) : "f"(u2f(a)), "f"(u2f(b))); return f2u(r); }
#endif /* __riscv_flen */
#endif

static const uint32_t corner[8] = { 0, 1, 0xffffffffu, 0x80000000u, 0x7fffffffu, 2, 0xfffffffeu, 0x55555555u };

static uint32_t opnd(uint32_t *s) { uint32_t r = xs32(s); return (r & 3) == 0 ? corner[(r >> 2) & 7] : xs32(s); }

/* integer ALU + M-extension, including div-by-zero and INT_MIN/-1 */
static uint32_t k_alu(uint32_t seed, int n) {
  uint32_t s = seed, h = 0x811c9dc5u;
  for (int i = 0; i < n; i++) {
    uint32_t a = opnd(&s), b = opnd(&s);
    MIX(h, a + b); MIX(h, a - b); MIX(h, a ^ b); MIX(h, a | b); MIX(h, a & b);
    MIX(h, a << (b & 31)); MIX(h, a >> (b & 31)); MIX(h, rv_sra((int32_t)a, b));
    MIX(h, (int32_t)a < (int32_t)b); MIX(h, a < b);
    MIX(h, a * b); MIX(h, rv_mulh((int32_t)a, (int32_t)b)); MIX(h, rv_mulhu(a, b)); MIX(h, rv_mulhsu((int32_t)a, b));
    MIX(h, rv_div((int32_t)a, (int32_t)b)); MIX(h, rv_divu(a, b));
    MIX(h, rv_rem((int32_t)a, (int32_t)b)); MIX(h, rv_remu(a, b));
  }
  return h;
}

#if defined(HOST) || defined(__riscv_flen)
/* random float operand. sub=1 -> mostly subnormal / tiny values */
static uint32_t fopnd(uint32_t *s, int sub) {
  uint32_t r = xs32(s), m = xs32(s) & 0x7fffffu, sg = (r & 1) << 31, e;
  switch ((r >> 1) & 15) {
    case 0: return sg;                          /* +-0   */
    case 1: return sg | 0x7f800000u;            /* +-inf */
    case 2: return 0x7fc00000u | m;             /* qNaN  */
    case 3: return sg | 0x3f800000u;            /* +-1   */
    default:
      e = sub ? ((r >> 5) & 3) : 127 - 24 + ((r >> 5) % 49);
      return sg | (e << 23) | m;
  }
}

static uint32_t k_fpu(uint32_t seed, int n, int sub) {
  uint32_t s = seed, h = 0x811c9dc5u;
  for (int i = 0; i < n; i++) {
    uint32_t a = fopnd(&s, sub), b = fopnd(&s, sub), c = fopnd(&s, sub);
    MIX(h, f_add(a, b)); MIX(h, f_sub(a, b)); MIX(h, f_mul(a, b)); MIX(h, f_div(a, b));
    MIX(h, f_sqrt(a & 0x7fffffffu)); MIX(h, f_madd(a, b, c)); MIX(h, f_nmsub(a, b, c));
    MIX(h, f_minmax(a, b, 0)); MIX(h, f_minmax(a, b, 1)); MIX(h, f_cmp(a, b));
    MIX(h, f_cvt_ws(a)); MIX(h, f_cvt_sw((int32_t)b)); MIX(h, f_sgnjx(a, b));
  }
  return h;
}

#endif

/* control flow: recursion, indirect calls, data-dependent branches */
static uint32_t fib(uint32_t n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }
static uint32_t op0(uint32_t x) { return x * 3 + 1; }
static uint32_t op1(uint32_t x) { return x >> 1; }
static uint32_t op2(uint32_t x) { return x ^ 0xdeadbeefu; }
static uint32_t op3(uint32_t x) { return (x << 7) | (x >> 25); }
static uint32_t (*const ops[4])(uint32_t) = { op0, op1, op2, op3 };
static uint32_t k_ctrl(uint32_t seed, int n) {
  uint32_t s = seed, h = fib(18), x = seed;
  for (int i = 0; i < n; i++) {
    uint32_t r = xs32(&s);
    x = ops[r & 3](x);
    if (x & 1) h += x; else if (x & 2) h ^= x; else h -= x;
    switch ((r >> 2) & 7) { case 0: h += 1; break; case 1: h *= 3; break; case 2: h ^= r; break;
      case 3: h = h >> 3 | h << 29; break; case 4: h -= r; break; default: h += x; }
  }
  return h;
}
