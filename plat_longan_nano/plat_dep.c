#include <bareline.h>

void udelay(uint32_t us) {
	const uint32_t tick_per_us = BL_CPU_TIMER_FREQ_MHZ;

	const uint32_t period = us * tick_per_us;
	const uint32_t base = BL_GET_CPU_TIMER();

	while ((BL_GET_CPU_TIMER() - base) < period) {
		;
	}

	return;
}

void *memcpy(void *d, void *s, int l) {
	char *_d = (char *)d;
	char *_s = (char *)s;

	while (l--) {
		*_d++ = *_s++;
	}

	return d;
}
