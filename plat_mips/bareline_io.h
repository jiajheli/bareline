#ifndef _BARELINE_IO_H_
#define _BARELINE_IO_H__ 1

#include <printf.h>

#define TERM_VT100 1

#define O_THR (0x0)
#define O_RBR (0x0)
#define O_LSR (0x14)
#	define LSR_DR (0x01)
#	define LSR_OE (0x02)
#	define LSR_PE (0x04)
#	define LSR_FE (0x08)
#	define LSR_THRE (0x20)
#	define LSR_RFE (0x80)

#ifndef REG8
#	define REG8(addr) (*((volatile unsigned char *)addr))
#endif

#ifndef UART_BASE_ADDR
#	error EE: UART_BASE_ADDR is undefined
#endif

#define UART_REG(off) REG8(UART_BASE_ADDR +off)

static inline void bl_putc(char c) {
	while (!(UART_REG(O_LSR) & LSR_THRE)) {
		;
	}

	UART_REG(O_THR) = c;
	return;
}

static inline int uart_tstc(void) {
	return (UART_REG(O_LSR) & (LSR_RFE | LSR_FE | LSR_PE | LSR_OE | LSR_DR)) == LSR_DR;
}

static inline char bl_getc(void) {
	while (!uart_tstc());

	return UART_REG(O_RBR);
}

#endif
