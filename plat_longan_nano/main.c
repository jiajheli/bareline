#include <bareline.h>
#include <longan_nano_reg.h>
#define BL_BUFF_SZ_B 128
#define BL_LINE_SZ_B 32

static void __configure_clock(void) {
	/* CK_IRC8M/2 -> CK_PLL -> CK_SYS --AHB Prescaler--> CK_AHB -> --APB2 Prescaler--> CK_APB2 -> PCLK2 -> USART0 */
	const int pllmf = (108 / 4) - 1;

	/* set CK_PLL/CK_AHB/CK_APB2 factor before enable */
	RRMW(
		RCU_CFG0,
		pllmf_4, (pllmf >> 4) & 1,
		pllmf, (pllmf & 0xf),
		apb2psc, 0,
		ahbpsc, 0
		);

	/* enable CK_PLL */
	RRMW(
		RCU_CTL,
		pllen, 1);
	while (RFLD(RCU_CTL, pllstb) != 1) {
		;
	}

	/* set clock tree:  */
	RRMW(
		RCU_CFG0,
		pllsel, 0, //CK_IRC8M / 2 -> CK_PLL
		scs, 2); //CK_PLL -> CK_SYS (-> CK_AHB -> CK_APB2 -> PCLK2 ->USART0)
	while (RFLD(RCU_CFG0, scss) != 2) {
		;
	}

	return;
}

static void __configure_pin_mux(void) {
	//reset USART0/GPIOA/AFIO
	RRMW(
		RCU_APB2RST,
		usart0rst, 1,
		parst, 1,
		afrst, 1);
	RRMW(
		RCU_APB2RST,
		usart0rst, 0,
		parst, 0,
		afrst, 0);

	//enable clock of USART0/GPIOA/AFIO
	RRMW(
		RCU_APB2EN,
		usart0en, 1,
		paen, 1,
		afen, 1);

	return;
}

int main(void) {
	char bl_buf[BL_BUFF_SZ_B];

	__configure_clock();
	__configure_pin_mux();

	bl_uart_init(USART_BAUDRATE, USART_CLK_HZ);

	bl_main_loop(bl_buf, sizeof(bl_buf), BL_LINE_SZ_B);

	return 0;
}
