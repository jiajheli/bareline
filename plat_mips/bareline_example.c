#include <bareline.h>

#define BL_BUFF_SZ_B 128
#define BL_LINE_SZ_B 16
#define BL_DEBUG 0

/* bl_putc wrapper for printf */
void _putchar(char character) {
	bl_putc(character);
	return;
}

int main(void) {
	char bl_buf[BL_BUFF_SZ_B];

	bl_main_loop(bl_buf, sizeof(bl_buf), BL_LINE_SZ_B);

	return 0;
}
