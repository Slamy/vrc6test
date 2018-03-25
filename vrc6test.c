#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <nes.h>
#include "vrc6.h"
#include "reset.h"
#include "joystick.h"

//Enable and disable both a FamiTracker does it
#define DISABLE_APU		APU.pulse[0].control = 0x30;
#define ENABLE_APU		APU.pulse[0].control = 0xbf;

//#define DISABLE_VRC6	VRC6.pulse1.duty_volume = 0x70;
//#define ENABLE_VRC6		VRC6.pulse1.duty_volume = 0x7f;

#define DISABLE_VRC6	VRC6.pulse1.period_high = 0x00;
#define ENABLE_VRC6		VRC6.pulse1.period_high = 0x80;

#define ALTERNATING_MODE_PERIOD 15

// define PPU register aliases
#define VRC6_PPU_BANKING_STYLE	*((uint8_t*)0xB003)
#define VRC6_CHR_SELECT			*((uint8_t*)0xB003)

// define palette color aliases
#define COL_BLACK 0x0f
#define COL_GRAY1 0x00
#define COL_GRAY2 0x10
#define COL_WHITE 0x20

#pragma bss-name(push, "ZEROPAGE")
size_t i;
unsigned char old_joystick_state;
unsigned char alternatingMode;

#pragma bss-name(pop)

const char INTRO_TEXT[] =
		"  Slamy's VRC6 Volume Test 0.1  "
		"                                "
		"According to a few sources I    "
		"could find, it's specified that "
		"the maximum volume of the pulse "
		"waves should be equally loud on "
		"the VRC6 and the internal APU.  "
		"With FamiTracker this is the    "
		"case for example.               "
		"                                "
		"This program shall help you to  "
		"verify your configuration by    "
		"outputting 440 Hz pulse waves.  "
		"Consider visiting the forum of  "
		"krikzz.com for further tipps    "
		"                                "
		"Hold A for a VRC6 pulse wave.   "
		"                                "
		"Hold B for an APU pulse wave.   "
		"                                "
		"Hold Start for an alternation   "
		"between APU and VRC6. You also  "
		"might hear a stutter. But this  "
		"a bug of this program.          "
		"                                "
		"                         Slamy  ";

const uint8_t PALETTE[] =
{
	COL_BLACK,	// background color
	COL_WHITE,	// background palette 0
	COL_WHITE,
	COL_BLACK
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

void setPalette(const char *palette, int len)
{
	// load the palette data into PPU memory $3f00-$3f1f
	PPU.vram.address = 0x3f;
	PPU.vram.address = 0x00;
	for (i = 0; i < len; ++i)
	{
		PPU.vram.data = palette[i];
	}
}

void setPaletteValue(unsigned char index, unsigned char val)
{
	//set palette value
	PPU.vram.address = 0x3f;
	PPU.vram.address = index;
	PPU.vram.data = val;

	//reset VRAM Address.
	PPU.vram.address = 0x23;
	PPU.vram.address = 0x82;

	// reset scroll location to top-left of screen
	PPU.scroll = 0x00;
	PPU.scroll = 0x00;
}

/**
 * main() will be called at the end of the initialization code in reset.s.
 * Unlike C programs on a computer, it takes no arguments and returns no value.
 */
void main(void)
{
	//linear address space for CHRRAM
	*((uint8_t*) 0xD000) = 0;
	*((uint8_t*) 0xD001) = 1;
	*((uint8_t*) 0xD002) = 2;
	*((uint8_t*) 0xD003) = 3;

	//init some registers to prepare sound registers
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

	DISABLE_VRC6
	DISABLE_APU

	setPalette(PALETTE, sizeof(PALETTE));

	showScreen(INTRO_TEXT);

	// reset scroll location to top-left of screen
	PPU.scroll = 0x00;
	PPU.scroll = 0x00;

	// enable NMI and rendering
	PPU.control = 0x00;
	PPU.mask = 0x1e;


	//some in fading ;-)
	for (i = 0; i < 3; i++)
		WaitVBlank();

	setPaletteValue(3, COL_GRAY1);

	for (i = 0; i < 6; i++)
		WaitVBlank();

	setPaletteValue(3, COL_GRAY2);

	for (i = 0; i < 6; i++)
		WaitVBlank();

	setPaletteValue(3, COL_WHITE);

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
			case 1 + ALTERNATING_MODE_PERIOD:
				//Enable VRC6, Disable APU
				DISABLE_APU
				ENABLE_VRC6
				showStatusBar("     VRC6");
				break;
			case 1 + ALTERNATING_MODE_PERIOD + ALTERNATING_MODE_PERIOD:
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

