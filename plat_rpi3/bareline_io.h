#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1

#include <printf.h>

#define TERM_VT100 1
#define BL_DEBUG 0
#define BL_CPU_TIMER_FREQ_MHZ (19) //It is 19.2MHz; make it 19 to simplify

#define BL_GET_CPU_TIMER() \
	({ unsigned long long  __res; \
		__asm__ __volatile__("mrs %0, cntpct_el0;" \
		                     : "=r" (__res)); \
		__res; \
	})

#define UART_BASE_ADDR (0x3f215040)

void udelay(uint32_t us);

#endif
