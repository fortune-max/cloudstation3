# sudo -E python3 evdev_server.py

import json
import evdev
import asyncio
import websockets

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


def write_event(type, code, value):
    device.write(type, code, value)
    device.syn()


async def handle_connection(websocket, path):
    while True:
        message = await websocket.recv()
        for chunk in json.loads(message):
            write_event(chunk["type"], chunk["code"], chunk["value"])
        await websocket.send("OK")

start_server = websockets.serve(handle_connection, '0.0.0.0', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
