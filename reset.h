#ifndef RESET_H_
#define RESET_H_

#include <stdint.h>

void WaitFrame(void);

void WaitVBlank(void);

extern uint8_t FrameCount;
#pragma zpsym("FrameCount");

#endif
