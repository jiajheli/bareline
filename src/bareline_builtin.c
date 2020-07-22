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
