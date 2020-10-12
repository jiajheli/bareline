#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <printf.h>

#define TERM_XTERM 1
#define BL_DEBUG 0

static inline void bl_putc(char c) {
	putchar(c);
	return;
}

static inline uint8_t bl_getc_nb(void) {
	return (fgetc(stdin) & 0xff);
}

static inline uint8_t bl_tstc(void) {
	return 1;
}

static inline int udelay(int us) {
	return usleep(us);
}

#endif
