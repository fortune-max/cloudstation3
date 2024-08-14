#pragma once

#include <cstdint>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

inline struct libevdev *dev = nullptr;
inline struct libevdev_uinput *uidev = nullptr;

void create_joystick();
void write_event(uint16_t type, uint16_t code, int32_t value);
