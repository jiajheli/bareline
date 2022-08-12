#include <bareline.h>

/*************************
	Common
**************************/
static int bl_cmd_run(int ac, char** av, int cc, void **_cv);
static int bl_ctab_cmplt(int ac, char** av, int cc, void **_cv);

const cmd_tab_t cmd_tab = {
	.start = (bl_cmd_t **)&__start_bl_cmd,
	.end = (bl_cmd_t **)&__stop_bl_cmd,
};

static inline int is_plain_char(const char c) {
	return ((c >= 32) && (c < 127));
}

static inline int is_hist_line_ind(const char c) {
	return (c & 0x80);
}

/* non-standard strcpy: returns len */
static int bl_n_strcpy(char *_dst, char *_src) {
	char *dst = _dst;
	char *src = _src;

	while (is_plain_char(*src)) {
		*dst++ = *src++;
	}
	*dst = 0;
	return (dst - _dst);
}

static int bl_get_arguments(char *cmd, char **argv) {
	int argc = 0;
	char last = 0;

	do {
		/* if cmd[i-1] is space and cmd[i] is not space */
		if (((last | ' ') == ' ') && ((*cmd | ' ') != ' ')) {
			if (argv) {
				argv[argc] = cmd;
			}
			argc++;
		}
		last = *cmd++;
	} while (last);

	return argc;
}

static inline int is_digit(char c) {
	return ((c >= '0') && (c <= '9'));
}

static inline int is_hex_digit(char c) {
	c |= 0x20; //force to lower case
	return ((c >= 'a') && (c <= 'f'));
}

long bl_atoi(const char *s) {
	long result = 0;
	int mul = 10;
	int digit;

	while (*s == '0') {
		s++;
	}

	if (*s == 'x') {
		mul = 16;
		s++;
	}

	while (is_digit(*s) || is_hex_digit(*s)) {
		if (is_digit(*s)) {
			digit = (*s & 0xf);
		} else {
			digit = ((*s & 0xf) + 9);
		}
		result = result * mul + digit;
		s++;
	}

	return result;
}

void bl_memset(void *s, char c, int n) {
	char *p = (char *)s;

	while (n--) {
		*p++ = c;
	}
	return;
}

void *bl_memcpy(void *d, void *s, int l) {
	char *_d = (char *)d;
	char *_s = (char *)s;

	while (l--) {
		*_d++ = *_s++;
	}

	return d;
}

int bl_getc_to(const int to_us) {
	const int32_t poll_period_us = 10;
	int32_t tick = 0;

	do {
		if (bl_tstc()) {
			return bl_getc_nb();
		}

		udelay(poll_period_us);

		if (to_us == -1) continue;

	} while ((tick++ * poll_period_us) < to_us);

	return -1;
}

uint8_t bl_getc(void) {
	while (!bl_tstc());

	return bl_getc_nb();
}

void bl_putc_rn(char c) {
	if (c == '\n') {
		bl_putc('\r');
	}

	bl_putc(c);

	return;
}
/* wrapper func. for printf */
void _putchar(char) __attribute__ ((alias ("bl_putc_rn")));

void bl_puts(char *s) {
	while (*s) {
		bl_putc_rn(*s);
		s++;
	}
	return;
}

/*************************
	History management
**************************/
static void bl_hst_append_line(line_t *line, history_t *hist) {
	int cur = hist->next;

	cur += bl_n_strcpy(&hist->buf[cur], line->buf);
	hist->buf[cur++] = (0x80 | line->len);

	if ((cur + line->sz_b + 1) > hist->sz_b) { //+2 for EOL & line len.
		bl_memset(&hist->buf[cur], 0, hist->sz_b - cur);
		cur = 0;
	}

	hist->next = cur;

	while (is_plain_char(hist->buf[cur])) {
		hist->buf[cur++] = 0;
	};
	hist->buf[cur] = 0;

	hist->cur = hist->next;

	return;
}

static void bl_hst_get_next(line_t *line, history_t *hist) {
	int next_line_head;

	if (hist->cur == hist->next) {
		return;
	}

	next_line_head = hist->cur;
	while (!is_hist_line_ind(hist->buf[next_line_head++])) {
		;
	}

	while (!is_plain_char(hist->buf[next_line_head])) {
		if (next_line_head == hist->next) {
			break;
		}
		if ((hist->sz_b - next_line_head) < (line->sz_b - 1)) {
			next_line_head = 0;
			break;
		}
		next_line_head++ ;
	}

	hist->cur = next_line_head;

	return;
}

static void bl_hst_get_prev(line_t *line, history_t *hist) {
	int prev_line_ind;
	int prev_line_head;

	prev_line_ind = hist->cur;

	//find prev. line indicator.
	do {
		prev_line_ind = (prev_line_ind + hist->sz_b - 1) % (hist->sz_b);
	} while (
		(prev_line_ind != hist->next) && //before wrap-around, and
		(!is_hist_line_ind(hist->buf[prev_line_ind]))); //not prev. line indicator

	if (prev_line_ind == hist->next) { //traversed the whole hist. buf. and find no older line
		return;
	}

	prev_line_head = prev_line_ind - (hist->buf[prev_line_ind] & 0x7f);

	hist->cur = prev_line_head;

	return;
}

static void bl_hst_init(history_t *hist, const int sz) {
	hist->sz_b = sz;
	hist->next = 0;
	hist->cur = 0;
	bl_memset(hist->buf, 0, sz);

	return;
}

/*************************
	Line buffer management
**************************/
static void bl_lb_reset(line_t *line) {
	line->cursor = 0;
	line->len = 0;
	bl_memset(&line->buf[0], 0, line->sz_b+1);
	return;
}

static void bl_lb_init(line_t *line, unsigned char sz_b) {
	line->sz_b = sz_b;
	bl_lb_reset(line);
	return;
}

static void bl_lb_add_char(line_t *line, history_t *hist, char input) {
	int lb_pos;

	if (line->len < line->sz_b) {
		if (!(is_plain_char(input))) {
			bl_dbg_printf("EE: Unknown key pressed: %c(%d)\n", input, input);
			return;
		}

		bl_puts(VT100_CURSOR_INSERT);
		bl_putc(input);

		lb_pos = line->len++;
		while (lb_pos != line->cursor) {
			line->buf[lb_pos] = line->buf[lb_pos-1];
			lb_pos--;
		}
		line->buf[lb_pos] = input;
		line->cursor++;
	}
	return;
}

static void bl_lb_del_char(line_t *line, history_t *hist) {
	int cur;

	if (line->len) {
		bl_puts(VT100_CURSOR_DEL);
		line->len--;
		cur = line->cursor;
		bl_n_strcpy(&line->buf[cur], &line->buf[cur+1]);
	}

	return;
}

static void bl_lb_cursor_head(line_t *line, history_t *hist) {
	if (line->cursor) {
		bl_puts("\r]");
		line->cursor = 0;
	}
	return;
}

static void bl_lb_cursor_end(line_t *line, history_t *hist) {
	if (line->cursor != line->len) {
		bl_puts(&line->buf[line->cursor]);
		line->cursor = line->len;
	}
	return;
}

static void bl_lb_cursor_left(line_t *line, history_t *hist) {
	if (line->cursor) {
		bl_puts(VT100_CURSOR_BW);
		line->cursor--;
	}
	return;
}

static void bl_lb_cursor_right(line_t *line, history_t *hist) {
	if (line->cursor < line->len) {
		bl_puts(VT100_CURSOR_FW);
		line->cursor++;
	}
	return;
}

static void bl_lb_erase_cursor_end(line_t *line, history_t *hist) {
	if (line->cursor < line->len) {
		bl_puts(VT100_ERASE_CURSOR_END);
		bl_memset(&line->buf[line->cursor], 0, line->len - line->cursor);
		line->len = line->cursor;
	}
	return;
}

static void bl_lb_exit(line_t *line, history_t *hist) {
	line->len = bl_n_strcpy(line->buf, "exit");
	return;
}

static int bl_lb_enter(line_t *line, history_t *hist) {
	int res = 0;

	bl_putc_rn('\n');

	if (line->len) {
		bl_hst_append_line(line, hist);
		res = bl_ctab_lookup(line->buf, &cmd_tab, bl_cmd_run);
		line->sz_b *= (!!line->buf[0]); //do_exit() makes buf[0] and further makes sz_b = 0
		bl_lb_reset(line);
	}

	return res;
}

static void bl_lb_get_history(line_t *line, history_t *hist, char dir) {
	if ((dir == K_UP) || (dir == K_CTRL_P)) {
		bl_hst_get_prev(line, hist);
	} else {
		bl_hst_get_next(line, hist);
	}

	line->cursor = bl_n_strcpy(line->buf, &hist->buf[hist->cur]);
	if (line->len > line->cursor) {
		bl_memset(&line->buf[line->cursor], 0, line->len - line->cursor);
	}
	line->len = line->cursor;

	bl_puts("\r]");
	bl_puts(line->buf);
	bl_puts(VT100_ERASE_CURSOR_END);

	return;
}

static int bl_lb_vt100_udlr(line_t *line, history_t *hist) {
	const char dir_map[] = {
		[0] = K_CTRL_P, //K_UP
		[1] = K_CTRL_N, //K_DOWN
		[2] = K_CTRL_F, //K_RIGHT
		[3] = K_CTRL_B //K_LEFT
	};
	unsigned char input2;

	bl_getc();
	input2 = (bl_getc() - 0x41);
	if (input2 < 4) {
		return dir_map[input2];
	} else {
		bl_dbg_printf("EE: ESC + unknown key pressed: %c(%d)\n", input2, input2);
		return 0xff;
	}
}

static void bl_lb_auto_complete(line_t *line, history_t *hist) {
	const int len = bl_ctab_lookup(line->buf, &cmd_tab, bl_ctab_cmplt);

	if (len > line->len) {
		line->cursor = len;
		line->len = len;
	}

	return;
}

/*************************
	Command
**************************/
static inline int not_space_eol(char c) {
	return (c & (~(' ')));
}

void bl_cmd_syntax(bl_cmd_t *cmd) {
	bl_printf("%s, %s\n", cmd->cmd, cmd->help);
	if (cmd->syntax) {
		bl_printf("syntax: %s %s\n", cmd->cmd, cmd->syntax);
	}
	return;
}

static int bl_cmd_run_cmdlen(const char *cmd) {
	int l = 0;
	while (not_space_eol(*cmd++) && (++l)) {
		;
	}
	return l;
}

static int bl_cmd_run(int ac, char** av, int cc, void **_cv) {
	bl_cmd_t **cv = (bl_cmd_t **)_cv;
	int av_len, cv_len, i;

	if (cc > 1) {
		av_len = bl_cmd_run_cmdlen(av[0]);

		i = 0;
		do {
			cv_len = bl_cmd_run_cmdlen(cv[i]->cmd);
			if (cv_len == av_len) {
				cc = 1;
				cv[0] = cv[i];
				break;
			}
		} while (++i < cc);
	}

	if (cc == 1) {
		if (ac < cv[0]->req_argc) {
			bl_puts("EE: syntax error: ");
			bl_cmd_syntax(cv[0]);
			return 0;
		} else {
			return cv[0]->func(ac, av);
		}
	} else if (cc > 1) {
		bl_puts("EE: ambiguous command: ");
		for (i=0; i<cc; i++) {
			bl_printf("%s, ", cv[i]->cmd);
		}
		bl_printf("%c%c \n", K_BSP, K_BSP);
	} else {
		bl_puts("EE: command not found\n");
	}
	return 0;
}

static int bl_ctab_match(const char *input, const char *cmd) {
	const char *_input, *_cmd;
	_input = input;
	_cmd = cmd;

	while (not_space_eol(*_input) && (*_input == *_cmd)) {
		_input++;
		_cmd++;
	}

	if ((!not_space_eol(*_input)) ||
			(*_input == '*') ||
			((*_input == '.') && (*_cmd == 0))) {
		return BL_MATCH_SOME;
	} else {
		return BL_MATCH_NONE;
	}
}

static int bl_cmd_retrieve(char *needle, const cmd_tab_t *ctab, void *strv[]) {
	int i = 0;
	bl_cmd_t *cmd;

	cmd = (bl_cmd_t *)ctab->start;
	while (cmd < (bl_cmd_t *)ctab->end) {
		if (BL_MATCH_SOME == bl_ctab_match(needle, cmd->cmd)) {
			strv[i++] = (void *)cmd;
		}
		cmd++;
	}

	return i;
}

int bl_ctab_lookup(char *cmdline, const cmd_tab_t *ctab, cmd_act_fp action) {
	int argc, cmdc;
	char **argv;
	void **cmdv;

	argc = bl_get_arguments(cmdline, NULL);
	if (argc) {
		argv = __builtin_alloca(sizeof(char *) * argc);
		bl_get_arguments(cmdline, argv);
	} else {
		argv = __builtin_alloca(sizeof(char *));
		argv[0] = "*";
	}

	cmdv = __builtin_alloca(
		sizeof(void *) *
		((bl_cmd_t *)ctab->end - (bl_cmd_t *)ctab->start));
	cmdc = bl_cmd_retrieve(argv[0], ctab, cmdv);

	return action(argc, argv, cmdc, cmdv);
}

/*************************
	Auto-complete
**************************/
static int bl_ctab_cmplt(int ac, char** av, int cc, void **_cv) {
	bl_cmd_t **cv = (bl_cmd_t **)_cv;
	char *input;

	if ((ac < 2) && cc) {
		if (cc == 1) {
			input = av[0];
			while (*input) {
				if ((*input++) == '.') {
					return 0;
				}
			}
			return bl_n_strcpy(av[0], (*cv)->cmd);
		} else {
			bl_puts("\n");
			while (cc--) {
				bl_printf("%s ", (*cv)->cmd);
				cv++;
			}
			bl_puts("\n");
		}
	}
	return 0;
}

/*************************
	Debug
**************************/
#if (BL_DEBUG == 1)
static void bl_dbg_dump_history(history_t *hist) {
	int i, cur, next;
	char *buf;

	next = hist->next;
	cur = hist->cur;
	buf = hist->buf;

	bl_printf(
		"\nDD: history: %p, buf: %p, sizeof(history_t): %d, sizeof(buf): %d",
		hist, (void *)buf, sizeof(history_t), hist->sz_b);
	bl_printf("\nDD: next: %d, cur: %d", next, cur);

	i = 0;
	while (i < hist->sz_b) {
		if ((i%16) == 0) {
			bl_dbg_printf("\nDD: %02x: ", i);
		}

		bl_putc(' ');
		if (i == cur) {
			bl_puts("\b>");
		}
		if (i == next) {
			bl_puts("\b@");
		}

		if (buf[i] == 0) {
			bl_puts("_");
		} else if (is_plain_char(buf[i])) {
			bl_putc(buf[i]);
		} else {
			bl_putc('#');
		}
		i++;
	}
	bl_dbg_printf("\n");

	return;
}

static void bl_dbg_dump_line(line_t *line) {
	int i;
	const int line_t_sz_b = sizeof(line_t) + line->sz_b + 1;

	bl_printf(
		"\nDD: line: %p, line->buf %p, sizeof(line_t): %d, sizeof(buf): %d\n",
		line, line->buf, sizeof(line_t), line->sz_b + 1);
	bl_printf("DD: cursor: %d, len: %d\n", line->cursor, line->len);

	for (i=0; i<(line_t_sz_b - sizeof(line_t)); i++) {
		bl_printf("  %d", i/10);
	}
	bl_puts("\n");

	for (i=0; i<(line_t_sz_b - sizeof(line_t)); i++) {
		bl_printf("  %d", i%10);
	}
	bl_puts("\n");

	for (i=0; i<=line->len; i++) {
		bl_puts("   ");
		if (i == line->cursor) bl_puts("\b\bC ");
		if (i == line->len-1) bl_puts("\bE");
	}
	bl_puts("\n");

	for (i=0; i<(line_t_sz_b - sizeof(line_t)); i++) {
		bl_printf(" %02x", line->buf[i]);
	}
	bl_puts("\n]");
	return;
}
#endif

/*************************
	Core
**************************/
__attribute__ ((weak))
void show_banner(void) {
	bl_puts("Bareline starts\n]");
	return;
}

void bl_main_loop(char *buf, int sz, unsigned char line_sz_b) {
	line_t *line;
	history_t *hist;

	char input;
	int cmd_res;

	if (sz < line_sz_b) {
		bl_puts("EE: line buffer too small\n");
		return;
	}

	line = (line_t *)buf;
	bl_lb_init(line, line_sz_b);

	hist = (history_t *)(line->buf + line_sz_b + 1);
	bl_hst_init(hist, sz - (hist->buf - buf));

#if (TERM_KEY_TEST_MODE == 1)
	bl_puts("Press 'CTRL + C' to exit key test mode\n");
#else
	show_banner();
#endif

	while (line->sz_b) {
		input = bl_getc();

#if (TERM_KEY_TEST_MODE == 1)
		bl_printf("Glyph: %c, Dec: %d, Hex: %02x\n", input, input, input);
		if (input == K_CTRL_C) {
			line->sz_b = 0;
		}
		continue;
#endif

	__reentrant_for_vt100:
		switch (input) {
		case K_CTRL_A:
			bl_lb_cursor_head(line, hist);
			break;
		case K_CTRL_B:
			bl_lb_cursor_left(line, hist);
			break;
		case K_CTRL_E:
			bl_lb_cursor_end(line, hist);
			break;
		case K_CTRL_F:
			bl_lb_cursor_right(line, hist);
			break;
		case K_CTRL_K:
			bl_lb_erase_cursor_end(line, hist);
			break;
		case K_CTRL_N:
		case K_CTRL_P:
			bl_lb_get_history(line, hist, input);
			break;
		case K_ESC:
			input = bl_lb_vt100_udlr(line, hist);
			goto __reentrant_for_vt100;
			break;
		case K_BSP:
			bl_lb_cursor_left(line, hist);
		case K_CTRL_D:
			bl_lb_del_char(line, hist);
			break;
		case K_CTRL_C:
			bl_lb_exit(line, hist);
			// let "K_LF/K_CR" (ENTER) applies "exit" command
		case K_LF:
		case K_CR:
			cmd_res = bl_lb_enter(line, hist);
			if (cmd_res) {
				bl_printf("EE: return value: %d\n", cmd_res);
			}
			bl_putc(']');
			break;
		case K_TAB:
			bl_lb_auto_complete(line, hist);
			break;
		default:
			bl_lb_add_char(line, hist, input);
		}

#if (BL_DEBUG == 1)
		bl_dbg_dump_line(line);
		bl_dbg_dump_history(hist);
#endif
	}
	return;
}
