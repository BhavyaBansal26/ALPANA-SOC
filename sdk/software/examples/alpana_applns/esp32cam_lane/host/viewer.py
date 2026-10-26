#!/usr/bin/env python3
"""ALPANA_SOC camera + lane-detection viewer.

Reads the UART stream from ALPANA_SOC (2.5 Mbaud; use --baud 1000000 if the
program was built with CAM_BAUD=1000000), shows the JPEG video and draws the
lanes, lane centre and steering decision that ALPANA computed on its own 80x60
lane-feature copy of each frame.

  A5 5A 'J' 'P' | len u32 | frame_no u32 | sum32 u32 | jpeg[len]
  A5 5A 'L' 'N' | len u32 | frame_no, steer, dir, flags, la, lb, ra, rb, cx_top, cx_bot (f32),
                            servo_us, lane_cycles, [thr, noise,] lx[ROWS], rx[ROWS]
                  (len 68 = v1: 10 rows, no thr/noise;  len 56+2*ROWS = v2)
  A5 5A 'G' 'Y' | 4804    | frame_no, 80x60 pixels: the image ALPANA analyses (inset, top right)
  lines starting with '#' are ALPANA status text

usage:  python3 viewer.py [/dev/ttyUSB1] [--baud 2500000] [--save DIR] [--scale 2] [--gray]
  keys:  q / Esc quit,  t toggle the 80x60 inset,  g toggle gray video
  needs: pip install pyserial opencv-python numpy
"""
import argparse, os, struct, time

import numpy as np
import serial

try:
    import cv2
except ImportError:
    cv2 = None

SYNC = b"\xA5\x5A"
DIRS = ["STRAIGHT", "LEFT", "RIGHT", "LOST"]
LW, LH, Y_TOP, Y_BOT = 80, 60, 26, 58


def row_y(rows):
    return [Y_BOT - i * (Y_BOT - Y_TOP) // (rows - 1) for i in range(rows)]


def lane_len_ok(ln):
    return ln == 68 or (ln > 56 and (ln - 56) % 2 == 0 and (ln - 56) // 2 <= 32)


def parse_lane(p):
    v = struct.unpack_from("<IiII6fII", p, 0)
    if len(p) == 68:                                      # v1 packet: 10 rows
        rows, off, thr, noise = 10, 48, None, None
    else:
        rows, off = (len(p) - 56) // 2, 56
        thr, noise = struct.unpack_from("<ii", p, 48)
    lx = struct.unpack_from(f"<{rows}b", p, off)
    rx = struct.unpack_from(f"<{rows}b", p, off + rows)
    return dict(fno=v[0], steer=v[1], dir=v[2], left=bool(v[3] & 1), right=bool(v[3] & 2), dark=bool(v[3] & 4),
                lheld=bool(v[3] & 8), rheld=bool(v[3] & 16),
                la=v[4], lb=v[5], ra=v[6], rb=v[7], cxt=v[8], cxb=v[9], servo=v[10], cyc=v[11],
                thr=thr, noise=noise, lx=lx, rx=rx, ys=row_y(rows))


def draw_lane(img, L, fps):
    h, w = img.shape[:2]
    s = w / LW
    P = lambda x, y: (int(x * s + s / 2), int(y * s + s / 2))
    for i, y in enumerate(L["ys"]):                       # raw detections
        if L["lx"][i] >= 0: cv2.circle(img, P(L["lx"][i], y), max(2, int(s)), (0, 255, 255), -1)
        if L["rx"][i] >= 0: cv2.circle(img, P(L["rx"][i], y), max(2, int(s)), (255, 255, 0), -1)
    # fitted lines: red = measured in this frame, orange = held from the last frames
    if L["left"]:  cv2.line(img, P(L["la"] + L["lb"] * Y_TOP, Y_TOP), P(L["la"] + L["lb"] * Y_BOT, Y_BOT),
                            (0, 140, 255) if L["lheld"] else (0, 0, 255), 2)
    if L["right"]: cv2.line(img, P(L["ra"] + L["rb"] * Y_TOP, Y_TOP), P(L["ra"] + L["rb"] * Y_BOT, Y_BOT),
                            (0, 140, 255) if L["rheld"] else (0, 0, 255), 2)
    if L["left"] or L["right"]:
        cv2.line(img, P(L["cxt"], Y_TOP), P(L["cxb"], Y_BOT), (0, 255, 0), 2)
    cv2.line(img, (w // 2, h - 4), (w // 2, int(h * 0.80)), (128, 128, 128), 1)
    # steering arrow from the bottom centre
    ang = np.radians(L["steer"]); r = h * 0.22
    tip = (int(w / 2 + r * np.sin(ang)), int(h - 4 - r * np.cos(ang)))
    col = {0: (0, 255, 0), 1: (255, 160, 0), 2: (0, 160, 255), 3: (0, 0, 255)}[L["dir"]]
    cv2.arrowedLine(img, (w // 2, h - 4), tip, col, 4, tipLength=0.3)
    cv2.putText(img, f"{DIRS[L['dir']]}  {L['steer']:+d} deg", (8, 26), cv2.FONT_HERSHEY_SIMPLEX, 0.8, col, 2, cv2.LINE_AA)
    extra = f" | thr {L['thr']} noise {L['noise']}" if L["thr"] is not None else ""
    cv2.putText(img, f"servo {L['servo']} us | lane {L['cyc'] / 80000:.1f} ms on ALPANA | {fps:.1f} fps{extra}"
                + ("  (dark tape)" if L["dark"] else ""), (8, 48), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1, cv2.LINE_AA)


def draw_thumb(img, th, L):
    """80x60 lane-feature image that ALPANA analysed, x2, top-right, with its detections"""
    t = cv2.cvtColor(cv2.resize(th, (LW * 2, LH * 2), interpolation=cv2.INTER_NEAREST), cv2.COLOR_GRAY2BGR)
    if L:
        for i, y in enumerate(L["ys"]):
            cv2.line(t, (0, 2 * y + 1), (4, 2 * y + 1), (0, 255, 0), 1)
            if L["lx"][i] >= 0: cv2.circle(t, (2 * L["lx"][i] + 1, 2 * y + 1), 2, (0, 255, 255), -1)
            if L["rx"][i] >= 0: cv2.circle(t, (2 * L["rx"][i] + 1, 2 * y + 1), 2, (255, 255, 0), -1)
    h, w = img.shape[:2]
    if w >= LW * 2 + 4 and h >= LH * 2 + 4:
        img[2:2 + LH * 2, w - 2 - LW * 2:w - 2] = t
        cv2.rectangle(img, (w - 3 - LW * 2, 1), (w - 2, 2 + LH * 2), (0, 255, 0), 1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("port", nargs="?", default="/dev/ttyUSB1")
    ap.add_argument("--baud", type=int, default=2_500_000)
    ap.add_argument("--save", default=None)
    ap.add_argument("--scale", type=float, default=2.0)
    ap.add_argument("--gray", action="store_true", help="show the video in black & white")
    a = ap.parse_args()
    opt = dict(gray=a.gray, thumb=True)

    gui = cv2 is not None and (os.environ.get("DISPLAY") or os.environ.get("WAYLAND_DISPLAY") or os.name == "nt")
    if not gui and not a.save:
        a.save = "."
        print("no GUI display found -> saving frames to ./latest.jpg")
    if a.save:
        os.makedirs(a.save, exist_ok=True)

    ser = serial.Serial(a.port, a.baud, timeout=0.2)
    print(f"listening on {a.port} @ {a.baud} baud  (Ctrl-C to quit)")
    buf = bytearray()
    frames = bad = junk = 0
    t0 = tstat = time.time()
    win_frames = 0
    pending = None            # (fno, decoded image) waiting for its lane packet
    last_lane = None
    last_thumb = None

    def show_text(chunk):
        nonlocal junk
        for line in bytes(chunk).split(b"\n"):
            k = line.find(b"#")
            if k >= 0 and all(32 <= c < 127 or c == 13 for c in line[k:]):
                junk += k
                print("ALPANA " + line[k:].decode().strip())
            else:
                junk += len(line)

    def display(img, lane):
        if img is None:
            return False
        img = img.copy()
        if opt["gray"] and img.ndim == 3:
            img = cv2.cvtColor(cv2.cvtColor(img, cv2.COLOR_BGR2GRAY), cv2.COLOR_GRAY2BGR)
        if a.scale != 1.0:
            img = cv2.resize(img, None, fx=a.scale, fy=a.scale, interpolation=cv2.INTER_LINEAR)
        fps = frames / max(time.time() - t0, 1e-6)
        if lane:
            draw_lane(img, lane, fps)
        if opt["thumb"] and last_thumb is not None:
            draw_thumb(img, last_thumb, lane or last_lane)
        if a.save:
            cv2.imwrite(os.path.join(a.save, "latest.jpg"), img)
        if gui:
            cv2.imshow("ALPANA_SOC lane detection (ESP32-CAM -> SPI -> ALPANA -> UART)", img)
            k = cv2.waitKey(1) & 0xFF
            if k == ord("t"): opt["thumb"] = not opt["thumb"]
            if k == ord("g"): opt["gray"] = not opt["gray"]
            return k in (27, ord("q"))
        return False

    def find_sync(b):
        i = 0
        while True:
            i = b.find(SYNC, i)
            if i < 0 or len(b) < i + 4 or b[i + 2:i + 4] in (b"JP", b"LN", b"GY"):
                return i
            i += 1

    while True:
        buf += ser.read(ser.in_waiting or 1)
        while True:
            i = find_sync(buf)
            text_end = i if i >= 0 else len(buf)
            nl = buf.rfind(b"\n", 0, text_end)
            if nl >= 0:
                show_text(buf[:nl]); del buf[: nl + 1]; continue
            if i < 0:
                if len(buf) > 4096:
                    junk += len(buf) - 3; del buf[:-3]
                break
            if len(buf) < i + 8:
                break
            kind = bytes(buf[i + 2:i + 4])
            if kind == b"GY":
                (ln,) = struct.unpack_from("<I", buf, i + 4)
                if ln != LW * LH + 4:
                    del buf[: i + 1]; continue
                if len(buf) < i + 8 + ln:
                    break
                last_thumb = np.frombuffer(bytes(buf[i + 12:i + 8 + ln]), np.uint8).reshape(LH, LW)
                del buf[: i + 8 + ln]
                continue
            if kind == b"LN":
                (ln,) = struct.unpack_from("<I", buf, i + 4)
                if not lane_len_ok(ln):
                    del buf[: i + 1]; continue
                if len(buf) < i + 8 + ln:
                    break
                lane = parse_lane(bytes(buf[i + 8:i + 8 + ln]))
                del buf[: i + 8 + ln]
                last_lane = lane
                if pending is not None:
                    if display(pending[1], lane if lane["fno"] == pending[0] else None):
                        return
                    pending = None
                continue
            # JPEG packet
            if len(buf) < i + 16:
                break
            ln, fno, csum = struct.unpack_from("<III", buf, i + 4)
            if ln == 0 or ln > 200_000:
                del buf[: i + 1]; continue
            if len(buf) < i + 16 + ln:
                break
            jpg = bytes(buf[i + 16:i + 16 + ln])
            if (sum(jpg) & 0xFFFFFFFF) != csum:
                bad += 1; del buf[: i + 1]; continue
            del buf[: i + 16 + ln]
            frames += 1; win_frames += 1
            if pending is not None:                       # previous frame never got a lane packet
                if display(pending[1], last_lane):
                    return
            img = cv2.imdecode(np.frombuffer(jpg, np.uint8), cv2.IMREAD_COLOR) if cv2 is not None else None
            pending = (fno, img)
        if gui:
            cv2.waitKey(1)
        now = time.time()
        if now - tstat >= 3:
            ls = f" | lane {DIRS[last_lane['dir']]} {last_lane['steer']:+d} deg servo {last_lane['servo']} us" if last_lane else ""
            print(f"VIEWER  shown {frames} frames | {win_frames / (now - tstat):.1f} fps | "
                  f"UART bad frames {bad} | junk bytes {junk}{ls}")
            tstat = now; win_frames = 0


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
