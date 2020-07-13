#include <bareline.h>

#define O_THR (0x0)
#define O_RBR (0x0)
#define O_DLL (0x0)
#define O_DLH (0x4)
#define O_IER (0x4)
#define O_FCR (0x8)
#	define FCR_FIFO_EN (0x01)
#	define FCR_RBR_RESET (0x02)
#	define FCR_THR_RESET (0x04)
#	define FCR_RBR_TRIG_HI (0xc0)
#define O_LCR (0xc)
#	define LCR_7B (0x2)
#	define LCR_8B (0x3)
#	define LCR_1S (0 << 2)
#	define LCR_2S (1 << 2)
#	define LCR_NO_PARITY (0 << 3)
#	define LCR_BRK_SIG (1 << 6)
#	define LCR_DLAB_EN (1 << 7)
#define O_MCR (0x10)
#	define MCR_DTR (0x1)
#	define MCR_RTS (0x2)
#	define MCR_AFE (0x20)
#	define MCR_CLK_LX (0x40)
#define O_LSR (0x14)
#	define LSR_DR (0x01)
#	define LSR_OE (0x02)
#	define LSR_PE (0x04)
#	define LSR_FE (0x08)
#	define LSR_THRE (0x20)
#	define LSR_RFE (0x80)

#ifndef UART_BASE_ADDR
#	error EE: UART_BASE_ADDR is undefined
#endif

#define UART_REG(off) REG8(UART_BASE_ADDR +off)

void bl_putc(char c) {
	while (!(UART_REG(O_LSR) & LSR_THRE)) {
		;
	}

	UART_REG(O_THR) = c;
	return;
}

void bl_putc_rn(char c) {
	if (c == '\n') {
		bl_putc('\r');
	}

	bl_putc(c);

	return;
}

uint8_t bl_tstc(void) {
	return (UART_REG(O_LSR) & (LSR_RFE | LSR_FE | LSR_PE | LSR_OE | LSR_DR)) == LSR_DR;
}

uint8_t bl_getc_nb(void) {
	return UART_REG(O_RBR);
}

int bl_getc_to(const int to_us) {
	const int32_t poll_period_us = 10;
	int32_t tick = 0;

	do {
		if (bl_tstc()) {
			return bl_getc_nb();
		}

		udelay(poll_period_us);

		if (to_us == -1) continue;

	} while ((tick++ * poll_period_us) < to_us);

	return -1;
}

uint8_t bl_getc(void) {
	while (!bl_tstc());

	return bl_getc_nb();
}

static void
uart_set_baud(uint32_t baudrate, uint32_t base_clk_mhz) {
	uint32_t bclk_hz = base_clk_mhz * 1000 * 1000;
	uint32_t div;
	uint8_t lcr_bak;

	lcr_bak = UART_REG(O_LCR);
	UART_REG(O_LCR) = (lcr_bak | LCR_DLAB_EN);

	div = (bclk_hz + (8 * baudrate)) / (16 * baudrate);

	UART_REG(O_DLL) = (div) & 0xff;
	UART_REG(O_DLH) = (div >> 8) & 0xff;
	UART_REG(O_LCR) = lcr_bak;

	return;
}

uint32_t
bl_uart_init(uint32_t baudrate, uint32_t base_clk_mhz) {
	UART_REG(O_IER) = 0;

	uart_set_baud(baudrate, base_clk_mhz);

	UART_REG(O_LCR) = (LCR_8B | LCR_1S | LCR_NO_PARITY);
	UART_REG(O_MCR) = (MCR_DTR | MCR_RTS | MCR_CLK_LX);
	UART_REG(O_FCR) = (FCR_RBR_TRIG_HI | FCR_FIFO_EN);
	UART_REG(O_FCR) = (FCR_RBR_TRIG_HI | FCR_THR_RESET | FCR_RBR_RESET | FCR_FIFO_EN);

	return 0;
}
