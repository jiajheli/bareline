#include <bareline.h>
#include <longan_nano_reg.h>
#include <stdint.h>

#define CTL_INPUT_FLOAT (0x1)
#define CTL_INPUT_PP (0x2)
#define CTL_OUTPUT_AFIO_PP (0x2)

#define MD_INPUT (0)
#define MD_OUTPUT_50MHZ (0x3)

void bl_putc(char c) {
	USART0_STAT_T u0stat;

	do {
		u0stat.v = RGET(USART0_STAT);
	} while (!u0stat.f.tbe);

	RRMW(USART0_DATA, data, c);
	return;
}

uint8_t bl_tstc(void) {
	return RFLD(USART0_STAT, rbne);
}

uint8_t bl_getc_nb(void) {
	return RFLD(USART0_DATA, data);
}

uint32_t
bl_uart_init(uint32_t baudrate, uint32_t base_clk_mhz) {
	uint32_t baud_int, baud_fra;

	//set USART0 tx pin attr.: output push-pull, 50mhz, gpio a #09
	//set USART0 rx pin attr.: input float, gpio a #10
	RRMW(
		GPIOA_CTL1,
		ctl10, CTL_INPUT_FLOAT,
		md10, MD_INPUT,
		ctl9, CTL_OUTPUT_AFIO_PP,
		md9, MD_OUTPUT_50MHZ);

	//disable USART0
	RRMW(USART0_CTL0, uen, 0);

	//set baud
	baud_int = USART_CLK_HZ / 16 / USART_BAUDRATE;
	baud_fra = (USART_CLK_HZ - (baud_int * 16 * USART_BAUDRATE)) / USART_BAUDRATE;
	RRMW(USART0_BAUD, intdiv, baud_int, fradiv, baud_fra);

	//8n1
	RRMW(
		USART0_CTL0,
		uen, 1,
		wl, 0,
		wm, 0,
		pcen, 0,
		perrie, 0, tbeie, 0, tcie, 0, rbneie, 0, idleie, 0,
		ten, 1, ren, 1,
		rwu, 0,
		sbkcmd, 0);

  return 0;
}
