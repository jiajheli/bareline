#ifndef __LACHESIS_REG_MISC__
#define __LACHESIS_REG_MISC__ 1
#include <gram.h>

#ifndef _REG_RCU
#	define _REG_RCU (0x40021000)
#endif

#ifndef _REG_GPIOA
#	define _REG_GPIOA (0x40010800)
#endif

#ifndef _REG_USART0
#	define _REG_USART0 (0x40013800)
#endif

#ifndef _REG_ECLIC
#	define _REG_ECLIC (0xd2000000)
#endif

GRAM32(
	RCU_CTL, _REG_RCU, 0x0,
	RF_RSV32(23, 0);
	RFIELD32(24, 24, pllen);
	RFIELD32(25, 25, pllstb);
	RF_RSV32(31, 26);
	);

GRAM32(
	RCU_CFG0, _REG_RCU, 0x4,
	RFIELD32(1, 0, scs);
	RFIELD32(3, 2, scss);
	RFIELD32(7, 4, ahbpsc);
	RFIELD32(10, 8, apb1psc);
	RFIELD32(13, 11, apb2psc);
	RFIELD32(15, 14, adcpsc);
	RFIELD32(16, 16, pllsel);
	RFIELD32(17, 17, predv0_lsb);
	RFIELD32(21, 18, pllmf);
	RF_RSV32(28, 22);
	RFIELD32(29, 29, pllmf_4);
	RF_RSV32(31, 30);
	);

GRAM32(
	RCU_APB2RST, _REG_RCU, 0xc,
	RFIELD32(0, 0, afrst);
	RF_RSV32(1, 1);
	RFIELD32(2, 2, parst);
	RF_RSV32(13, 3);
	RFIELD32(14, 14, usart0rst);
	RF_RSV32(31, 15);
	);

GRAM32(
	RCU_APB2EN, _REG_RCU, 0x18,
	RFIELD32(0, 0, afen);
	RF_RSV32(1, 1);
	RFIELD32(2, 2, paen);
	RF_RSV32(13, 3);
	RFIELD32(14, 14, usart0en);
	RF_RSV32(31, 15);
	);

GRAM32(
	GPIOA_CTL1, _REG_GPIOA, 0x4,
	RF_RSV32(3, 0);
	RFIELD32(5, 4, md9);
	RFIELD32(7, 6, ctl9);
	RFIELD32(9, 8, md10);
	RFIELD32(11, 10, ctl10);
	RF_RSV32(31, 12);
	);

GRAM32(
	USART0_STAT, _REG_USART0, 0x0,
	RFIELD32(0, 0, perr);
	RFIELD32(1, 1, ferr);
	RFIELD32(2, 2, nerr);
	RFIELD32(3, 3, orerr);
	RFIELD32(4, 4, idlef);
	RFIELD32(5, 5, rbne);
	RFIELD32(6, 6, tc);
	RFIELD32(7, 7, tbe);
	RFIELD32(8, 8, lbdf);
	RFIELD32(9, 9, ctsf);
	RF_RSV32(31, 10);
	);

GRAM32(
	USART0_DATA, _REG_USART0, 0x4,
	RFIELD32(7, 0, data);
	RF_RSV32(31, 8);
	);

GRAM32(
	USART0_BAUD, _REG_USART0, 0x8,
	RFIELD32(3, 0, fradiv);
	RFIELD32(15, 4, intdiv);
	RF_RSV32(31, 16);
	);

GRAM32(
	USART0_CTL0, _REG_USART0, 0xc,
	RFIELD32(0, 0, sbkcmd);
	RFIELD32(1, 1, rwu);
	RFIELD32(2, 2, ren);
	RFIELD32(3, 3, ten);
	RFIELD32(4, 4, idleie);
	RFIELD32(5, 5, rbneie);
	RFIELD32(6, 6, tcie);
	RFIELD32(7, 7, tbeie);
	RFIELD32(8, 8, perrie);
	RFIELD32(9, 9, pm);
	RFIELD32(10, 10, pcen);
	RFIELD32(11, 11, wm);
	RFIELD32(12, 12, wl);
	RFIELD32(13, 13, uen);
	RF_RSV32(31, 14);
	);

GRAM32(
	CLICCFG, _REG_ECLIC, 0x0,
	RF_RSV32(0, 0);
	RFIELD32(4, 1, nlbits);
	RF_RSV32(31, 5);
	);

GRAM32(
	CLICINFO, _REG_ECLIC, 0x4,
	RFIELD32(12, 0, num_interrupt);
	RFIELD32(20, 13, version);
	RFIELD32(24, 21, clicintctlbits);
	RF_RSV32(31, 25);
	);

GRAM32(
	MTH, _REG_ECLIC, 0x8,
	RF_RSV32(23, 0);
	RFIELD32(31, 24, mth);
	);

GRAM32(
	CLICINT, _REG_ECLIC, 0x1000,
	RFIELD32(0, 0, ip);
	RF_RSV32(7, 1);
	RFIELD32(8, 8, ie);
	RF_RSV32(15, 9);
	RFIELD32(16, 16, shv);
	RFIELD32(18, 17, trig);
	RF_RSV32(23, 19);
	RFIELD32(31, 24, ctl);
	);

GRAM_DUP(CLICINT, CLICINT3, 0x0c);
GRAM_DUP(CLICINT, CLICINT7, 0x1c);

#endif
