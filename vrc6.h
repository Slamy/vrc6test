
/* Audio Processing Unit */
struct __vrc6 {
	struct {
		unsigned char duty_volume;
		unsigned char period_low;
		unsigned char period_high;
	} pulse1;
	unsigned char frequency_scaling;
};
#define VRC6             (*(struct __vrc6*)0x9000)
