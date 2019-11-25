#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1
#include <stdio.h>

#define TERM_XTERM 1

static inline void bl_putc(char c) {
	putchar(c);
	return;
}

static inline char bl_getc(void) {
	return (fgetc(stdin) & 0xff);
}

#endif
