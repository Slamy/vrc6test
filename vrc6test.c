/**
 * Hello, World!
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <time.h>
#include <conio.h>
#include <nes.h>
#include "vrc6.h"

// define PPU register aliases

#define VRC6_PPU_BANKING_STYLE	*((uint8_t*)0xB003)
#define VRC6_CHR_SELECT			*((uint8_t*)0xB003)

// define palette color aliases
#define COL_BLACK 0x0f
#define COL_WHITE 0x20

#pragma bss-name(push, "ZEROPAGE")
size_t i;
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

//const char TEXT[] = "orldorldorld!";

const uint8_t PALETTE[] = {
	COL_BLACK,                           // background color
	COL_WHITE, COL_WHITE, COL_WHITE, // background palette 0
};


void showScreen (const char *str, int len)
{
	return;
	// load the text sprites into the background (nametable 0)
	// nametable 0 is VRAM $2000-$23ff, so we'll choose an address in the
	// middle of the screen. The screen can hold a 32x30 grid of 8x8 sprites,
	// so an offset of 0x1ca (X: 10, Y:14) puts us around the middle vertically
	// and roughly centers our text horizontally.
	PPU.vram.address = 0x20;
	PPU.vram.address = 0x40;
	for ( i = 0; i < len; ++i ) {
		PPU.vram.data = (uint8_t) str[i];
	}
}


/**
 * main() will be called at the end of the initialization code in reset.s.
 * Unlike C programs on a computer, it takes no arguments and returns no value.
 */
void main(void) {
	

	//VRC6_PPU_BANKING_STYLE = 0x10;
	*((uint8_t*)0xD000)=0;
	*((uint8_t*)0xD001)=1;
	*((uint8_t*)0xD002)=2;
	*((uint8_t*)0xD003)=3;

	// load the palette data into PPU memory $3f00-$3f1f
	PPU.vram.address = 0x3f;
	PPU.vram.address = 0x00;
	for ( i = 0; i < sizeof(PALETTE); ++i ) {
		PPU.vram.data = PALETTE[i];
	}

	// load the text sprites into the background (nametable 0)
	// nametable 0 is VRAM $2000-$23ff, so we'll choose an address in the
	// middle of the screen. The screen can hold a 32x30 grid of 8x8 sprites,
	// so an offset of 0x1ca (X: 10, Y:14) puts us around the middle vertically
	// and roughly centers our text horizontally.
	PPU.vram.address = 0x20;
	PPU.vram.address = 0x40;
	for ( i = 0; i < sizeof(TEXT); ++i ) {
		PPU.vram.data = (uint8_t) TEXT[i];
	}

	showScreen(TEXT, sizeof(TEXT));

	// reset scroll location to top-left of screen
	PPU.scroll = 0x00;
	PPU.scroll = 0x00;

	VRC6.frequency_scaling = 0;

	VRC6.pulse1.duty_volume = 0x70;
	VRC6.pulse1.period_low = 0xab;
	VRC6.pulse1.period_high = 0x81;

	APU.pulse[0].ramp=0x08;
	APU.pulse[0].period_low=0xab;
	APU.pulse[0].len_period_high=0x09;
	APU.pulse[0].control=0xb0;

	// enable NMI and rendering
	PPU.control = 0x80;
	PPU.mask = 0x1e;

	// infinite loop
	while (1)
	{
		
		for(i=0;i<9000;i++)
		{
		}
		
		//Enable APU, Disable VRC6
		*((volatile uint8_t*)0x9000)=0x70;
		APU.pulse[0].control=0xbf;
		
		for(i=0;i<9000;i++)
		{
		}
		
		//Enable VRC6, Disable APU
		*((volatile uint8_t*)0x9000)=0x7f;
		APU.pulse[0].control=0xb0;
		
	};
};

