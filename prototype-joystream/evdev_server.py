# sudo -E python3 evdev_server.py

import evdev
import time

device_capabilities = {
    evdev.ecodes.EV_KEY: [
        evdev.ecodes.BTN_SOUTH,  # A button (304)
        evdev.ecodes.BTN_EAST,  # B button (305)
        evdev.ecodes.BTN_WEST,  # Y! button (308)
        evdev.ecodes.BTN_NORTH,  # X! button (307)
        evdev.ecodes.BTN_TL,  # LB/L1 button (310)
        evdev.ecodes.BTN_TR,  # RB/R1 button (311)
        evdev.ecodes.BTN_SELECT,  # Back/Select button (314)
        evdev.ecodes.BTN_START,  # Start button (315)
        evdev.ecodes.BTN_MODE,  # Xbox button (316)
        evdev.ecodes.BTN_THUMBL,  # Left joystick button (317)
        evdev.ecodes.BTN_THUMBR,  # Right joystick button (318)
    ],
    evdev.ecodes.EV_ABS: [  # Absolute axis events
        (evdev.ecodes.ABS_X, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=16, flat=128, resolution=0)),  # left analog X axis
        (evdev.ecodes.ABS_Y, evdev.AbsInfo(value=-1, min=-32768, max=32767, fuzz=16, flat=128, resolution=0)),  # left analog Y axis
        (evdev.ecodes.ABS_Z, evdev.AbsInfo(value=0, min=0, max=1023, fuzz=0, flat=15, resolution=0)),  # L2, Z axis
        (evdev.ecodes.ABS_RX, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=16, flat=128, resolution=0)),  # right analog RX axis
        (evdev.ecodes.ABS_RY, evdev.AbsInfo(value=-1, min=-32768, max=32767, fuzz=16, flat=128, resolution=0)),  # right analog RY axis
        (evdev.ecodes.ABS_RZ, evdev.AbsInfo(value=0, min=0, max=1023, fuzz=0, flat=15, resolution=0)),  # R2, RZ axis
        (evdev.ecodes.ABS_HAT0X, evdev.AbsInfo(value=0, min=-1, max=1, fuzz=0, flat=0, resolution=0)),  # D-pad X axis
        (evdev.ecodes.ABS_HAT0Y, evdev.AbsInfo(value=0, min=-1, max=1, fuzz=0, flat=0, resolution=0)),  # D-pad Y axis
    ]
}

device = evdev.UInput(events=device_capabilities, name="Emulated Joystick")

while True:
    print("Sending A key")
    device.write(evdev.ecodes.EV_KEY, evdev.ecodes.BTN_SOUTH, 1)  # Press the A key
    device.syn()
    time.sleep(1)
    device.write(evdev.ecodes.EV_KEY, evdev.ecodes.BTN_SOUTH, 0)  # Release the A key
    device.syn()
    time.sleep(4)