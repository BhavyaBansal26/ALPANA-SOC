#!/usr/bin/env python3
"""Lane-detection test on the PC: synthetic colour road scenes -> the same
"lane feature" 80x60 image the ESP32 sketch makes -> ../lane.c (compiled for
the PC) -> detection rate per scan row and steering direction.

  python3 lane_test_gen.py            (needs gcc, numpy, opencv-python)

Columns: COLOUR = feature image with the yellow boost (sketch LANE_FEATURE 1)
         LUMA   = plain grayscale (LANE_FEATURE 0), for comparison
Writes lane_test_results.png (camera | what ALPANA sees | detections).
"""
import os, subprocess, sys
import numpy as np, cv2

HERE = os.path.dirname(os.path.abspath(__file__))
W, H, HOR, LW, LH, NF = 320, 240, 80, 80, 60, 12
rng = np.random.default_rng(7)
COL = dict(asphalt=(70, 70, 75), concrete=(165, 160, 150), wood=(180, 130, 80), lightfloor=(200, 200, 195),
           white=(240, 240, 240), yellow=(215, 180, 60), faded=(190, 175, 120), tape=(30, 30, 35))


def truth_x(y, side, curve, shift):
    t = (y - HOR) / (H - HOR)
    xc = W / 2 + shift * W * t + curve * W * (1 - t) ** 2
    return xc + side * (0.08 * W + 0.42 * W * t)


def render(k, road='asphalt', lcol='white', rcol='white', curve=0.0, shift=0.0, dashed=None, only=None,
           shadow=False, glare=False, blotch=False, noise=6, light=1.0):
    img = np.zeros((H, W, 3), np.float32); img[:] = COL[road]; img[:HOR] = (150, 170, 200)
    for side, col in ((-1, lcol), (1, rcol)):
        if only and (only == 'X' or (only == 'L' and side == 1) or (only == 'R' and side == -1)):
            continue
        for y in range(HOR + 5, H - 1):
            if dashed and (dashed == 'B' or (dashed == 'L') == (side == -1)) and ((y + 6 * k) // 18) % 2:
                continue
            th = max(2, int(2 + 10 * (y - HOR) / (H - HOR)))
            cv2.line(img, (int(truth_x(y, side, curve, shift)), y), (int(truth_x(y + 1, side, curve, shift)), y + 1), COL[col], th)
    if blotch:
        img[HOR:] += cv2.GaussianBlur(rng.normal(0, 25, (H, W)).astype(np.float32), (0, 0), 6)[HOR:, :, None]
    if shadow:
        img[HOR:, :int(W * 0.45)] *= 0.45
    if glare:
        cv2.circle(img, (int(W * 0.62), int(H * 0.72)), 22, (255, 255, 255), -1)
        img = cv2.GaussianBlur(img, (0, 0), 1.0)
    img = img * light + rng.normal(0, noise, img.shape)
    return np.clip(img, 0, 255).astype(np.uint8)


def feature(r, g, b, colour=True):           # == lane_feature() in the sketch
    r, g, b = (c.astype(np.int32) for c in (r, g, b))
    y = (r * 77 + g * 150 + b * 29) >> 8
    if not colour:
        return y
    yel = np.minimum(r, g) - b - 2 * np.maximum(0, r - g - 20)
    return np.clip(y + 2 * np.maximum(0, yel - 24), 0, 255)


def esp32_thumb(rgb, colour=True):           # colour JPEG -> 1/2 decode -> RGB565 -> feature -> (max+mean)/2
    ok, b = cv2.imencode('.jpg', rgb[..., ::-1], [cv2.IMWRITE_JPEG_QUALITY, 70])
    half = cv2.resize(cv2.imdecode(b, cv2.IMREAD_COLOR), (2 * LW, 2 * LH), interpolation=cv2.INTER_AREA)
    f = feature(half[..., 2] & 0xF8, half[..., 1] & 0xFC, half[..., 0] & 0xF8, colour).reshape(LH, 2, LW, 2)
    return ((f.max(axis=(1, 3)) + (f.sum(axis=(1, 3)) >> 2)) >> 1).astype(np.uint8)


CASES = {
    'white on asphalt': dict(),
    'yellow L + white R, asphalt': dict(lcol='yellow'),
    'yellow on concrete': dict(road='concrete', lcol='yellow', rcol='yellow'),
    'faded yellow on concrete': dict(road='concrete', lcol='faded', rcol='faded', noise=8),
    'yellow concrete curve left': dict(road='concrete', lcol='yellow', curve=-0.35),
    'yellow concrete curve right': dict(road='concrete', lcol='yellow', rcol='yellow', curve=0.35),
    'yellow + glare spot': dict(road='concrete', lcol='yellow', rcol='yellow', glare=True),
    'yellow + tree shadow': dict(road='concrete', lcol='yellow', rcol='yellow', shadow=True, curve=0.25),
    'dashed yellow L, white R': dict(lcol='yellow', dashed='L', shift=0.2),
    'dashed both, concrete, yellow': dict(road='concrete', lcol='yellow', rcol='yellow', dashed='B', curve=-0.3),
    'only yellow L, concrete, curve R': dict(road='concrete', lcol='yellow', only='L', curve=0.3),
    'dark tape on light floor': dict(road='lightfloor', lcol='tape', rcol='tape', curve=-0.35),
    'white tape on wood floor': dict(road='wood', curve=0.3),
    'low light noisy, yellow asphalt': dict(lcol='yellow', rcol='yellow', light=0.55, noise=14, curve=-0.3),
    'NO lines, blotchy concrete': dict(road='concrete', only='X', noise=10, blotch=True),
}
DIRS = ['STRAIGHT', 'LEFT', 'RIGHT', 'LOST']


def run(binary, frames):
    path = os.path.join(HERE, '_frames.raw')
    np.concatenate([f.reshape(-1) for f in frames]).tofile(path)
    out = subprocess.run([binary, path, str(len(frames))], capture_output=True, text=True).stdout.split('\n')
    os.remove(path)
    res = []
    for line in filter(str.strip, out):
        v = line.split(); n = int(v[8])
        res.append(dict(dir=int(v[0]), steer=int(v[1]), rows=[tuple(map(int, v[9 + 3 * i:12 + 3 * i])) for i in range(n)]))
    return res


def expected(kw):
    if kw.get('only') == 'X':
        return 3
    c, s = kw.get('curve', 0.0), kw.get('shift', 0.0)
    cx = lambda y: (truth_x(y * 4 + 2, -1, c, s) + truth_x(y * 4 + 2, 1, c, s)) / 8
    e = 0.6 * (cx(26) - 40) / 40 + 0.4 * (cx(58) - 40) / 40
    st = max(-30, min(30, e * 1.8 * 30))
    return 0 if abs(st) <= 6 else (1 if st < 0 else 2)


def score(res, kw):
    c, s, only = kw.get('curve', 0.0), kw.get('shift', 0.0), kw.get('only')
    hit = tot = 0
    for r in res[3:]:
        for (y, lx, rx) in r['rows']:
            for side, x in ((-1, lx), (1, rx)):
                tx = truth_x(y * 4 + 2, side, c, s) / 4
                if (only and (only == 'X' or (only == 'L') != (side == -1))) or not 0 <= tx < 80:
                    continue
                tot += 1; hit += x >= 0 and abs(x - tx) <= 3
    e = expected(kw)
    return (hit / tot if tot else None), sum(r['dir'] == e for r in res[3:]) / len(res[3:]), e


def main():
    exe = os.path.join(HERE, 'lane_test_host')
    subprocess.run(['gcc', '-O2', '-I' + os.path.join(HERE, '..'), '-o', exe, os.path.join(HERE, 'lane_test_host.c'),
                    os.path.join(HERE, '..', 'lane.c'), '-lm'], check=True)
    print(f"{'case':34s} | {'COLOUR rows':>11s} {'dir':>5s} | {'LUMA rows':>9s} {'dir':>5s} | expected")
    tiles, ok_c, ok_l = [], 0, 0
    for name, kw in CASES.items():
        rgbs = [render(k, **kw) for k in range(NF)]
        tc = [esp32_thumb(x) for x in rgbs]
        rc, rl = run(exe, tc), run(exe, [esp32_thumb(x, False) for x in rgbs])
        (hc, dc, e), (hl, dl, _) = score(rc, kw), score(rl, kw)
        ok_c += dc; ok_l += dl
        f = lambda h: '   -' if h is None else f'{h * 100:4.0f}%'
        print(f"{name:34s} | {f(hc):>11s} {dc * 100:4.0f}% | {f(hl):>9s} {dl * 100:4.0f}% | {DIRS[e]}")
        cam = cv2.resize(rgbs[-1][..., ::-1], (240, 180))
        th = cv2.cvtColor(cv2.resize(tc[-1], (240, 180), interpolation=cv2.INTER_NEAREST), cv2.COLOR_GRAY2BGR)
        for (y, lx, rx) in rc[-1]['rows']:
            for x, col in ((lx, (0, 255, 255)), (rx, (255, 255, 0))):
                if x >= 0: cv2.circle(th, (int(x * 3 + 1), int(y * 3 + 1)), 3, col, -1)
        cv2.putText(th, f"{DIRS[rc[-1]['dir']]} {rc[-1]['steer']:+d}", (4, 16), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
        cv2.putText(cam, name, (4, 16), cv2.FONT_HERSHEY_SIMPLEX, 0.42, (0, 0, 255), 1)
        tiles.append(np.hstack([cam, th]))
    n = len(CASES)
    print(f"direction correct: colour feature {ok_c / n * 100:.0f}%   plain luma {ok_l / n * 100:.0f}%")
    rows = [np.hstack(tiles[i:i + 3]) if i + 3 <= len(tiles) else np.hstack(tiles[i:] + [np.zeros_like(tiles[0])] * (i + 3 - len(tiles)))
            for i in range(0, len(tiles), 3)]
    cv2.imwrite(os.path.join(HERE, 'lane_test_results.png'), np.vstack(rows))
    print('wrote lane_test_results.png')


if __name__ == '__main__':
    main()
