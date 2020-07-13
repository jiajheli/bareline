#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1

#include <stdint.h>
#include <printf.h>

#define TERM_VT100 1
#define BL_DEBUG 0
#define BL_CPU_TIMER_FREQ_MHZ (100)

#define BL_GET_CPU_TIMER() \
	({ uint32_t __res; \
		__asm__ __volatile__("mfc0 %0, $9;" \
		                     : "=r" (__res)); \
		__res; \
	})

void udelay(uint32_t us);

#endif
