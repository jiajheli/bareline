#include <stdio.h>
#include <termio.h>
#include <unistd.h>
#include <bareline.h>

#define BL_BUFF_SZ_B 128
#define BL_LINE_SZ_B 16
#define BL_DEBUG 0

int main(void) {
	struct termios myterm_orig, myterm;
	char bl_buf[BL_BUFF_SZ_B];

	if (tcgetattr(STDIN_FILENO, &myterm_orig) == -1) {
		printf("EE: can't get STDIN attr\n");
		return -1;
	}
	myterm = myterm_orig;

	/* Enter raw mode */
	myterm.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	myterm.c_oflag &= ~OPOST;
	myterm.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	myterm.c_cflag &= ~(CSIZE | PARENB);
	myterm.c_cflag |= CS8;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &myterm) == -1) {
		printf("EE: can't enter noncanonical mode\n");
		return -1;
	}

	bl_main_loop(bl_buf, sizeof(bl_buf), BL_LINE_SZ_B);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &myterm_orig);

	return 0;
}
