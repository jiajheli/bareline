#include <bareline.h>

/*************************
	command: exit
**************************/
static int do_exit(int argc, char **argv) {
	return 1;
}
BL_REG_CMD(exit, do_exit, "exit bareline");

/*************************
	command: help
**************************/
static int do_help(int argc, char **argv) {
	bl_cmd_t *cmd;

	cmd = (bl_cmd_t *)cmd_tab.start;
	while (cmd < (bl_cmd_t *)cmd_tab.end) {
		bl_printf("%s: %s\n\r", cmd->cmd, cmd->help);
		cmd++;
	}
	return 0;
}
BL_REG_CMD(help, do_help, "list all commands");

/*************************
	command: md
**************************/
static int do_md(int argc, char **argv) {
	int *addr;
	int iter = 1, i = 0;
	char *nl = "";

	switch (argc) {
	case 3:
		iter = bl_atoi(argv[2]);
	case 2:
		addr = (int *)(bl_atoi(argv[1]) & 0xfffffffc);
		break;
	default:
		bl_puts("md address [count]\n\r");
		return 0;
	}

	while (i < iter) {
		if (!(i % 4)) {
			bl_printf("%s%p: ", nl, addr);
			nl = "\n\r";
		}
		bl_printf("%08x ", *addr);
		addr++; i++;
	}
	bl_puts("\n\r");

	return 0;
}
BL_REG_CMD(md, do_md, "dump 4-byte word(s) of given address(es)");

/*************************
	command: mw
**************************/
static int do_mw(int argc, char **argv) {
	int *addr, val;

	if (argc != 3) bl_puts("mw address value\n\r");

	addr = (int *)(bl_atoi(argv[1]) & 0xfffffffc);
	val = bl_atoi(argv[2]);

	*addr = val;

	return 0;
}
BL_REG_CMD(mw, do_mw, "write a 4-byte word to a given address");
