#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1

#include <stdint.h>
#include <printf.h>

#define TERM_VT100 1
#define BL_DEBUG 0
#define BL_CPU_TIMER_FREQ_MHZ (27)
#define MTIME_BASE 0xd1000000

#define USART_CLK_HZ (108000000)
#define USART_BAUDRATE (115200)

__attribute__ ((naked, unused))
static uint32_t BL_GET_CPU_TIMER(void) {
	__asm__ __volatile__(
		"la t0, 0xd1000000;\n"
		"lw a0, 0(t0);\n"
		"jr ra;\n");
}

void udelay(uint32_t us);

#endif
