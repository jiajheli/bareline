#ifndef _BARELINE_H_
#define _BARELINE_H_ 1

#include <stdint.h>
#include <bareline_io.h>

#if defined(TERM_XTERM) && (TERM_XTERM == 1)
#	define K_BSP 0x7F
#else
/* assume vt100 */
#	define K_BSP 0x08
#endif

#define SECTION_BL_CMD __attribute__ ((section ("bl_cmd")))

#define NULL ((void *)0)

#define K_CTRL_A 0x01
#define K_CTRL_B 0x02
#define K_CTRL_C 0x03
#define K_CTRL_E 0x05
#define K_CTRL_F 0x06
#define K_CTRL_N 0x0e
#define K_CTRL_P 0x10 //CTRL+P, prev. in history

#define K_LF 0x0a
#define K_CR 0x0d
#define K_BEL 0x07
#define K_ESC 0x1b

#define K_TAB '\t'
#define K_PCM '['

#define K_UP 'A'
#define K_DOWN 'B'
#define K_RIGHT 'C'
#define K_LEFT 'D'

#define VT100_ERASE_LINE "\r\x1b[2K]"
#define VT100_CURSOR_FW "\x1b[C"

#define BL_REG_CMD(cmd, func, help)														\
	bl_cmd_t _bl_cmd_##cmd SECTION_BL_CMD = {#cmd, func, help}

#define BL_MATCH_SOME 1
#define BL_MATCH_NONE 0

typedef struct {
	short sz_b; //size of buf[]

	short latest;
	short next; //start of an empty line
	short cur;

	char buf[];
} history_t;

typedef struct {
	char *cmd;
	int (*func)(int, char **const);
	char *help;
} bl_cmd_t;

typedef int (*cmd_act_fp)(int, char**, int, void**);

typedef struct {
	char **start;
	char **end;
} cmd_tab_t;

typedef struct {
	unsigned char sz_b;
	unsigned char cursor;
	unsigned char len;
	char buf[];
} line_t;

void bl_main_loop(char *buf, int sz, unsigned char line_sz);

extern bl_cmd_t __start_bl_cmd, __stop_bl_cmd;
extern const cmd_tab_t cmd_tab;

#define bl_printf(...) printf(__VA_ARGS__)

#if defined(BL_DEBUG) && (BL_DEBUG == 1)
#	define bl_dbg_printf(...) printf(__VA_ARGS__)
#else
#	define bl_dbg_printf(...)
#endif

#ifndef REG8
#	define REG8(addr) (*((volatile unsigned char *)addr))
#endif

int bl_atoi(const char *s);
void bl_memset(void *s, char c, int n);
void *bl_memcpy(void *d, void *s, int l);
void bl_puts(char *s);

uint32_t
bl_uart_init(uint32_t baudrate, uint32_t base_clk_mhz);
uint8_t bl_tstc(void);
void bl_putc(char c);
void bl_putc_rn(char c);
uint8_t bl_getc(void);
uint8_t bl_getc_nb(void);
int bl_getc_to(const int to_us);

#endif
