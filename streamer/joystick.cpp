#include "joystick.hpp"

void create_joystick() {
    dev = libevdev_new();
    libevdev_set_name(dev, "Emulated Joystick");

    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_SOUTH, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_EAST, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_WEST, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_NORTH, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TL, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TR, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_SELECT, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_START, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_MODE, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_THUMBL, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_THUMBR, nullptr);

    libevdev_enable_event_type(dev, EV_ABS);
    struct input_absinfo absinfo = {0, -32768, 32767, 16, 128, 0};
    libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_RX, &absinfo);

    struct input_absinfo absinfo2 = {-1, -32768, 32767, 16, 128, 0};
    libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_RY, &absinfo);

    struct input_absinfo absinfo4 = {0, 0, 1023, 0, 15, 0};
    libevdev_enable_event_code(dev, EV_ABS, ABS_Z, &absinfo2);
    libevdev_enable_event_code(dev, EV_ABS, ABS_RZ, &absinfo2);

    struct input_absinfo absinfo3 = {0, -1, 1, 0, 0, 0};
    libevdev_enable_event_code(dev, EV_ABS, ABS_HAT0X, &absinfo3);
    libevdev_enable_event_code(dev, EV_ABS, ABS_HAT0Y, &absinfo3);

    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
}

void write_event(const uint16_t type, const uint16_t code, const int32_t value) {
    libevdev_uinput_write_event(uidev, type, code, value);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}
