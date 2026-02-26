import serial
import time
import re
from evdev import UInput, AbsInfo, ecodes as e

# -----------------------------
# CONFIG
# -----------------------------
PORT = "/dev/ttyACM0"
BAUD = 115200

AXIS_MIN = -32767
AXIS_MAX = 32767
DEADZONE_CM = 1.5
SCALE = 1200

def cm_to_axis(cm):
    if abs(cm) < DEADZONE_CM:
        return 0
    v = int(cm * SCALE)
    return max(AXIS_MIN, min(AXIS_MAX, v))

# -----------------------------
# CAPACITÉS DU JOYSTICK
# -----------------------------
capabilities = {
    e.EV_KEY: [e.BTN_JOYSTICK],  # obligatoire pour que Linux crée js0
    e.EV_ABS: {
        e.ABS_X: AbsInfo(0, AXIS_MIN, AXIS_MAX, 0, 0, 0),
        e.ABS_Y: AbsInfo(0, AXIS_MIN, AXIS_MAX, 0, 0, 0),
    }
}

ui = UInput(
    capabilities,
    name="UWB Virtual Joystick",
    version=3,
    bustype=0x03,  # BUS_USB → indispensable
)

print("🎮 Joystick virtuel EVDEV créé ✔")
print("   → Vérifie avec : cat /proc/bus/input/devices")

# -----------------------------
# SERIAL UWB
# -----------------------------
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

pattern = re.compile(r"REAL D1:\s*([0-9.]+)\s*REAL D2:\s*([0-9.]+)")

print("📡 Lecture UWB…")

# -----------------------------
# BOUCLE PRINCIPALE
# -----------------------------
while True:
    raw = ser.readline().decode(errors="ignore").strip()
    if not raw:
        continue

    m = pattern.search(raw)
    if not m:
        continue

    d1 = float(m.group(1))
    d2 = float(m.group(2))

    if d1 >= 65000 or d2 >= 65000:
        print("⚠️ Frame invalide ignorée")
        continue

    forward = d2 - d1
    leftRight = d1 - d2

    x = cm_to_axis(leftRight)
    y = cm_to_axis(forward)

    print(f"D1={d1:.1f} D2={d2:.1f} | X={x} Y={y}")

    ui.write(e.EV_ABS, e.ABS_X, x)
    ui.write(e.EV_ABS, e.ABS_Y, y)
    ui.syn()
import serial
import time
import re
from evdev import UInput, AbsInfo, ecodes as e

# -----------------------------
# CONFIG
# -----------------------------
PORT = "/dev/ttyACM0"
BAUD = 115200

AXIS_MIN = -32767
AXIS_MAX = 32767
DEADZONE_CM = 1.5
SCALE = 1200

def cm_to_axis(cm):
    if abs(cm) < DEADZONE_CM:
        return 0
    v = int(cm * SCALE)
    return max(AXIS_MIN, min(AXIS_MAX, v))

# -----------------------------
# CAPACITÉS DU JOYSTICK
# -----------------------------
capabilities = {
    e.EV_KEY: [e.BTN_JOYSTICK],  # obligatoire pour que Linux crée js0
    e.EV_ABS: {
        e.ABS_X: AbsInfo(0, AXIS_MIN, AXIS_MAX, 0, 0, 0),
        e.ABS_Y: AbsInfo(0, AXIS_MIN, AXIS_MAX, 0, 0, 0),
    }
}

ui = UInput(
    capabilities,
    name="UWB Virtual Joystick",
    version=3,
    bustype=0x03,  # BUS_USB → indispensable
)

print("🎮 Joystick virtuel EVDEV créé ✔")
print("   → Vérifie avec : cat /proc/bus/input/devices")

# -----------------------------
# SERIAL UWB
# -----------------------------
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

pattern = re.compile(r"REAL D1:\s*([0-9.]+)\s*REAL D2:\s*([0-9.]+)")

print("📡 Lecture UWB…")

# -----------------------------
# BOUCLE PRINCIPALE
# -----------------------------
while True:
    raw = ser.readline().decode(errors="ignore").strip()
    if not raw:
        continue

    m = pattern.search(raw)
    if not m:
        continue

    d1 = float(m.group(1))
    d2 = float(m.group(2))

    if d1 >= 65000 or d2 >= 65000:
        print("⚠️ Frame invalide ignorée")
        continue

    forward = d2 - d1
    leftRight = d1 - d2

    x = cm_to_axis(leftRight)
    y = cm_to_axis(forward)

    print(f"D1={d1:.1f} D2={d2:.1f} | X={x} Y={y}")

    ui.write(e.EV_ABS, e.ABS_X, x)
    ui.write(e.EV_ABS, e.ABS_Y, y)
    ui.syn()

