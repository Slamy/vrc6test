#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <nes.h>
#include "vrc6.h"
#include "reset.h"
#include "joystick.h"

// define PPU register aliases

#define VRC6_PPU_BANKING_STYLE	*((uint8_t*)0xB003)
#define VRC6_CHR_SELECT			*((uint8_t*)0xB003)

// define palette color aliases
#define COL_BLACK 0x0f
#define COL_WHITE 0x20

#pragma bss-name(push, "ZEROPAGE")
size_t i;
unsigned char old_joystick_state;
unsigned char alternatingMode;

#pragma bss-name(pop)

const char TEXT[] = 
	" Slamy's VRC6 Mapper Test Tool  "
	"                                "
	"                                "
	"The internal sound chip and the "
	"VRC6 inside the Everdrive N8    "
	"should now output a pulse wave  "
	"in an alternating way.          "
	"                                "
	"It's specified that the maximum "
	"volume of both pulse waves      "
	"should be equally loud.         "
	"                                "
	"If your configuration is correct"
	"you should hear one pulse wave  "
	"with frequent stutter. This is  "
	"normal for this test. If the    "
	"volume is alternating the       "
	"external audio is too loud or   "
	"quiet. Consider visiting the    "
	"forum of krikzz.com for further "
	"tipps.                          "
	"                                "
	"                                "
	"                                "
	"                         Slamy  ";

const uint8_t PALETTE[] =
{
	COL_BLACK,	// background color
	COL_WHITE,	// background palette 0
	COL_WHITE,
	COL_WHITE,
};

void showScreen(const char *str)
{
	// load the text sprites into the background (nametable 0)
	// nametable 0 is VRAM $2000-$23ff, so we'll choose an address in the
	// middle of the screen. The screen can hold a 32x30 grid of 8x8 sprites,
	// so an offset of 0x1ca (X: 10, Y:14) puts us around the middle vertically
	// and roughly centers our text horizontally.
	PPU.vram.address = 0x20;
	PPU.vram.address = 0x40;
	while (*str)
		PPU.vram.data = (uint8_t) *(str++);
}

void showStatusBar(const char *str)
{
	PPU.vram.address = 0x23;
	PPU.vram.address = 0x82;
	while (*str)
		PPU.vram.data = (uint8_t) *(str++);
}

/**
 * main() will be called at the end of the initialization code in reset.s.
 * Unlike C programs on a computer, it takes no arguments and returns no value.
 */
void main(void)
{

	*((uint8_t*) 0xD000) = 0;
	*((uint8_t*) 0xD001) = 1;
	*((uint8_t*) 0xD002) = 2;
	*((uint8_t*) 0xD003) = 3;

	// load the palette data into PPU memory $3f00-$3f1f
	PPU.vram.address = 0x3f;
	PPU.vram.address = 0x00;
	for (i = 0; i < sizeof(PALETTE); ++i)
	{
		PPU.vram.data = PALETTE[i];
	}

	showScreen(TEXT);

	// reset scroll location to top-left of screen
	PPU.scroll = 0x00;
	PPU.scroll = 0x00;

	VRC6.frequency_scaling = 0;

	//Full volume, 440 Hz according to FamiTracker
	VRC6.pulse1.period_low = 0xfd;
	VRC6.pulse1.period_high = 0x80;
	VRC6.pulse1.duty_volume = 0x7f;

	APU.status = 0x0f;
	APU.fcontrol = 0x40;

	//Full volume, 440 Hz according to FamiTracker
	APU.pulse[0].ramp = 0x08;
	APU.pulse[0].period_low = 0xfd;
	APU.pulse[0].len_period_high = 0x00;
	APU.pulse[0].control = 0xbf;

	// enable NMI and rendering
	PPU.control = 0x80;
	PPU.mask = 0x1e;

#define DISABLE_APU		APU.pulse[0].control = 0x30;
#define ENABLE_APU		APU.pulse[0].control = 0xbf;

//#define DISABLE_VRC6	VRC6.pulse1.duty_volume = 0x70;
//#define ENABLE_VRC6		VRC6.pulse1.duty_volume = 0x7f;

#define DISABLE_VRC6	VRC6.pulse1.period_high = 0x00;
#define ENABLE_VRC6		VRC6.pulse1.period_high = 0x80;

	DISABLE_VRC6
	DISABLE_APU

	// infinite loop
	while (1)
	{
		WaitVBlank();

		if (old_joystick_state != joystick_state)
		{
			alternatingMode = (joystick_state & JOY_START_MASK);

			if (joystick_state & JOY_BTN_A_MASK)
			{
				//Enable VRC6, Disable APU
				DISABLE_APU
				ENABLE_VRC6
				showStatusBar("     VRC6");
			}
			else if (joystick_state & JOY_BTN_B_MASK)
			{
				//Enable APU, Disable VRC6
				DISABLE_VRC6
				ENABLE_APU
				showStatusBar("APU      ");
			}
			else
			{
				//Disable both
				DISABLE_VRC6
				DISABLE_APU
				showStatusBar("         ");
			}
			// reset scroll location to top-left of screen
			PPU.scroll = 0x00;
			PPU.scroll = 0x00;

			old_joystick_state = joystick_state;

		}

		if (alternatingMode)
		{

			switch (alternatingMode)
			{
			case 1:
				//Enable APU, Disable VRC6
				DISABLE_VRC6
				ENABLE_APU
				showStatusBar("APU      ");
				break;
			case 1 + 20:
				//Enable VRC6, Disable APU
				DISABLE_APU
				ENABLE_VRC6
				showStatusBar("     VRC6");
				break;
			case 1 + 20 + 20:
				alternatingMode = 0;
			}
			alternatingMode++;

			// reset scroll location to top-left of screen
			PPU.scroll = 0x00;
			PPU.scroll = 0x00;

		}

		joystick_read(0);

	}
}


