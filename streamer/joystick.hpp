struct libevdev_uinput *uidev;

void create_joystick();
void write_event(uint16_t type, uint16_t code, int32_t value);
