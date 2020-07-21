#include <bareline.h>

/*************************
	command: exit
**************************/
static int do_exit(int argc, char **argv) {
	return 1;
}
BL_REG_CMD(exit, do_exit, 1, "exit bareline", NULL);

/*************************
	command: help
**************************/
static int bl_help_syntax(int ac, char** av, int cc, void **_cv) {
	bl_cmd_t **cv = (bl_cmd_t **)_cv;

	if (cc) {
		while (cc--) {
			bl_cmd_syntax(*cv++);
			if (cc) {
				bl_puts("\n");
			}
		}
	} else {
		bl_puts("EE: command not found\n");
	}

	return 0;
}

static int do_help(int argc, char **argv) {
	bl_cmd_t *cmd;

	if (argc == 2) {
		return bl_ctab_lookup(argv[1], &cmd_tab, bl_help_syntax);
	} else {
		cmd = (bl_cmd_t *)cmd_tab.start;
		while (cmd < (bl_cmd_t *)cmd_tab.end) {
			bl_printf("%s: %s\n", cmd->cmd, cmd->help);
			cmd++;
		}
		return 0;
	}
}
BL_REG_CMD(
	help, do_help, 1, "help message",
	"[cmd]\n"
	"  if `cmd' is given, show the associated syntax; otherwise list all commands");

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
		return 0;
	}

	while (i < iter) {
		if (!(i % 4)) {
			bl_printf("%s%p: ", nl, addr);
			nl = "\n";
		}
		bl_printf("%08x ", *addr);
		addr++; i++;
	}
	bl_puts("\n");

	return 0;
}
BL_REG_CMD(
	md, do_md, 2, "dump 4-byte integer(s) of given address(es)",
	"address [count=1]\n"
	"  Dump `count' integers from `address'. Default count = 1");

/*************************
	command: mw
**************************/
static int do_mw(int argc, char **argv) {
	int *addr, val;

	addr = (int *)(bl_atoi(argv[1]) & 0xfffffffc);
	val = bl_atoi(argv[2]);

	*addr = val;

	return 0;
}
BL_REG_CMD(
	mw, do_mw, 3, "write a 4-byte integer",
	"address value\n"
	"  write `value' to `address'");
