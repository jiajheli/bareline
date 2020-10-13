#include <bareline.h>

#define BL_BUFF_SZ_B 128
#define BL_LINE_SZ_B 32

int main(void) {
	char bl_buf[BL_BUFF_SZ_B];

	bl_main_loop(bl_buf, sizeof(bl_buf), BL_LINE_SZ_B);

	return 0;
}
