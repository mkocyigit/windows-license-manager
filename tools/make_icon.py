"""Özgün lisans simgesi üretici: kalkan + onay işareti + altın rozet. Saf Python (PIL gerekmez)."""
import math, struct, zlib, sys

def lerp(a, b, t): return a + (b - a) * t
def mix(c1, c2, t): return tuple(lerp(c1[i], c2[i], t) for i in range(3))
def clamp(v, lo=0.0, hi=1.0): return max(lo, min(hi, v))

def shield_inside(x, y, s=1.0, cx=0.5, cy=0.5):
    # (cx,cy) merkezli ölçekleme
    x = cx + (x - cx) / s
    y = cy + (y - cy) / s
    dx = abs(x - 0.5)
    top = 0.10 + 0.07 * (dx / 0.37)
    if y < top or dx > 0.37: return False
    if y <= 0.50: return True
    t = (y - 0.50) / 0.43
    if t > 1: return False
    return dx <= 0.37 * math.sqrt(max(0.0, 1 - t * t * 0.98)) if t < 1 else False

def seg_dist(px, py, ax, ay, bx, by):
    vx, vy = bx - ax, by - ay
    t = clamp(((px - ax) * vx + (py - ay) * vy) / (vx * vx + vy * vy))
    qx, qy = ax + t * vx, ay + t * vy
    return math.hypot(px - qx, py - qy)

DARK = (23, 37, 84); MID = (37, 99, 235); LIGHT = (96, 165, 250)
GOLD1 = (253, 224, 71); GOLD2 = (217, 119, 6)

C45 = math.cos(math.radians(45)); S45 = math.sin(math.radians(45))

def key_sdf(x, y, ox=0.0, oy=0.0):
    """Anahtar içinde mi? (u,v) = anahtar eksenli koordinat. Döner: (içinde, u, v)"""
    cx, cy = 0.50 + ox, 0.53 + oy
    u = (x - cx) * C45 + (y - cy) * S45
    v = -(x - cx) * S45 + (y - cy) * C45
    inside = False
    # halka (bow)
    r = math.hypot(u + 0.19, v)
    if r < 0.125 and r > 0.052: inside = True
    # gövde
    if -0.08 <= u <= 0.27 and abs(v) <= 0.034: inside = True
    # dişler
    if 0.15 <= u <= 0.195 and 0.0 <= v <= 0.105: inside = True
    if 0.225 <= u <= 0.272 and 0.0 <= v <= 0.080: inside = True
    # uç yuvarlatma
    if math.hypot(u - 0.27, v) < 0.034: inside = True
    return inside, u, v

def sample(x, y, detail):
    col = None; a = 0.0
    if shield_inside(x, y, 1.0):
        col = mix(DARK, MID, clamp((y - 0.1) / 0.8)); a = 1.0
    if shield_inside(x, y, 0.86, 0.5, 0.50):
        t = clamp((y - 0.12) / 0.75)
        col = mix(LIGHT, MID, t)
        a = 1.0
        if detail and y < 0.42:
            gl = 0.20 * (1 - (y - 0.12) / 0.30)
            col = mix(col, (255, 255, 255), clamp(gl))
    if col is None:
        return (0, 0, 0, 0)
    if detail:                                   # anahtar gölgesi
        sh, _, _ = key_sdf(x, y, -0.014, -0.020)
        if sh: col = mix(col, DARK, 0.38)
    ins, u, v = key_sdf(x, y)
    if ins:
        t = clamp((u + 0.32) / 0.60)
        kc = mix(GOLD1, GOLD2, clamp(0.15 + 0.7 * t + 0.25 * (abs(v) / 0.12)))
        if detail and v < -0.02 and u < -0.04:    # halkanın üstünde parlak şerit
            kc = mix(kc, (255, 251, 235), 0.45)
        col = kc
    return (col[0] / 255, col[1] / 255, col[2] / 255, a)

def render(size):
    ss = 4 if size <= 64 else 3
    n = size * ss
    detail = size >= 32
    rows = []
    for py in range(size):
        row = bytearray()
        for px in range(size):
            ar = ag = ab = aa = 0.0
            for sy in range(ss):
                for sx in range(ss):
                    x = (px * ss + sx + 0.5) / n
                    y = (py * ss + sy + 0.5) / n
                    r, g, b, a = sample(x, y, detail)
                    ar += r * a; ag += g * a; ab += b * a; aa += a
            cnt = ss * ss
            if aa == 0:
                row += bytes((0, 0, 0, 0))
            else:
                row += bytes((int(ar / aa * 255 + .5), int(ag / aa * 255 + .5), int(ab / aa * 255 + .5), int(aa / cnt * 255 + .5)))
        rows.append(bytes(row))
    return rows

def png(size, rows):
    raw = b"".join(b"\x00" + r for r in rows)
    def chunk(t, d): return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xffffffff)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))

def main(out):
    sizes = [16, 24, 32, 48, 64, 128, 256]
    imgs = []
    for s in sizes:
        print("render", s, flush=True)
        imgs.append((s, png(s, render(s))))
    hdr = struct.pack("<HHH", 0, 1, len(imgs))
    off = 6 + 16 * len(imgs)
    ent = b""; data = b""
    for s, p in imgs:
        ent += struct.pack("<BBBBHHII", s % 256, s % 256, 0, 0, 1, 32, len(p), off + len(data))
        data += p
    open(out, "wb").write(hdr + ent + data)
    open(out.replace(".ico", "_256.png"), "wb").write(imgs[-1][1])

main(sys.argv[1])
