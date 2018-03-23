#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>

void fastcall joystick_read(unsigned char joy);

extern uint8_t joystick_state;
#pragma zpsym("joystick_state");

#endif
