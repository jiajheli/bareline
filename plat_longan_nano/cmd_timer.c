#include <bareline.h>

/*************************
	command: sleep
**************************/
static int do_sleep(int argc, char **argv) {
	int sec;

	if (argc == 2) {
		sec = bl_atoi(argv[1]);
	} else {
		sec = 1;
	}

	bl_printf("Sleeping for %d second(s)...", sec);
	while (sec--) {
		udelay(1000000);
	}
	bl_putc('\n');

	return 0;
}
BL_REG_CMD(
	sleep, do_sleep, 1, "sleep for seconds",
	"seconds\n");
