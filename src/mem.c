// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Memory Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <bareline.h>

#define U_BOOT_CMD(_cmd, _argc, _dummy, _func, _help, _syntax) \
	BL_REG_CMD(_cmd, _func, _argc, _help, _syntax)

#define simple_strtoul(_string, _dummy1, _dummy2) bl_atoi(_string)

#define ctrlc() (bl_getc_nb() == K_CTRL_C)

static void *memcpy(void *d, void *s, int l) {
	return bl_memcpy(d, s, l);
}

static inline int isprint(const char c) {
	return ((c >= 32) && (c < 127));
}

#define CMD_RET_USAGE 0

#define CONFIG_CMD_MEMTEST (1)
#define CONFIG_CMD_MX_CYCLIC (1)
#define CONFIG_LOOPW (1)

typedef volatile unsigned long vu_long;
typedef unsigned long ulong;
typedef unsigned int u32;
typedef unsigned int uint;
typedef unsigned short u16;
typedef unsigned char u8;

#ifndef STDIO_H
static void puts(char *s) {
	return bl_puts(s);
}
#endif

static int cmd_get_data_size(char* arg, int default_size) {
  /* Check for a size specification .b, .w or .l. */

  if (arg[2] == '.') {
    switch (arg[3]) {
    case 'b':
      return 1;
    case 'w':
      return 2;
    case 'l':
      return 4;
#ifdef MEM_SUPPORT_64BIT_DATA
    case 'q':
      return 8;
#endif
    default:
			bl_printf("EE: unexpected size spec.: %c\n", arg[3]);
      return -1;
    }
  }
  return default_size;
}

#define MAX_LINE_LENGTH_BYTES (64)
#define DEFAULT_LINE_LENGTH_BYTES (16)
static int print_buffer(
	ulong addr, const void *data, uint width, uint count, uint linelen)
{
	/* linebuf as a union causes proper alignment */
	union linebuf {
#ifdef MEM_SUPPORT_64BIT_DATA
		uint64_t uq[MAX_LINE_LENGTH_BYTES/sizeof(uint64_t) + 1];
#endif
		uint32_t ui[MAX_LINE_LENGTH_BYTES/sizeof(uint32_t) + 1];
		uint16_t us[MAX_LINE_LENGTH_BYTES/sizeof(uint16_t) + 1];
		uint8_t  uc[MAX_LINE_LENGTH_BYTES/sizeof(uint8_t) + 1];
	} lb;
	int i;
#ifdef MEM_SUPPORT_64BIT_DATA
	uint64_t x;
#else
	uint32_t x;
#endif

	if (linelen*width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	while (count) {
		uint thislinelen = linelen;
		printf("%08lx:", addr);

		/* check for overflow condition */
		if (count < thislinelen)
			thislinelen = count;

		/* Copy from memory into linebuf and print hex values */
		for (i = 0; i < thislinelen; i++) {
			if (width == 4)
				x = lb.ui[i] = *(volatile uint32_t *)data;
#ifdef MEM_SUPPORT_64BIT_DATA
			else if (width == 8)
				x = lb.uq[i] = *(volatile uint64_t *)data;
#endif
			else if (width == 2)
				x = lb.us[i] = *(volatile uint16_t *)data;
			else
				x = lb.uc[i] = *(volatile uint8_t *)data;
#if defined(CONFIG_SPL_BUILD)
			printf(" %x", (uint)x);
#elif defined(MEM_SUPPORT_64BIT_DATA)
			printf(" %0*llx", width * 2, (long long)x);
#else
			printf(" %0*x", width * 2, x);
#endif
			data += width;
		}

		while (thislinelen < linelen) {
			/* fill line with whitespace for nice ASCII print */
			for (i=0; i<width*2+1; i++)
				puts(" ");
			linelen--;
		}

		/* Print data in ASCII characters */
		for (i = 0; i < thislinelen * width; i++) {
			if (!isprint(lb.uc[i]) || lb.uc[i] >= 0x80)
				lb.uc[i] = '.';
		}
		lb.uc[i] = '\0';
		printf("    %s\n", lb.uc);

		/* update references */
		addr += thislinelen * width;
		count -= thislinelen;

#ifndef CONFIG_SPL_BUILD
		if (ctrlc())
			return 0;
#endif
	}

	return 0;
}

#ifndef CONFIG_SYS_MEMTEST_SCRATCH
#	define CONFIG_SYS_MEMTEST_SCRATCH 0
#endif

/* Memory Display
 *
 * Syntax:
 *	md{.b, .w, .l, .q} {addr} {len}
 */
#define DISP_LINE_LEN	16
static int do_mem_md(int argc, char **argv)
{
	ulong	addr, length, bytes;
	const void *buf;
	int	size;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* New command specified.  Check for a size specification.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 0;

	/* Address is specified since argc > 1
	 */
	addr = simple_strtoul(argv[1], NULL, 16);

	/* If another parameter, it is the length to display.
	 * Length is the number of objects, not number of bytes.
	 */
	length = 1;
	if (argc > 2)
		length = simple_strtoul(argv[2], NULL, 16);

	bytes = size * length;
	buf = (void *)addr;

	/* Print the lines. */
	print_buffer(addr, buf, size, length, DISP_LINE_LEN / size);
	addr += bytes;

	return 0;
}

static int do_mem_mw(int argc, char **argv)
{
#ifdef MEM_SUPPORT_64BIT_DATA
	u64 writeval;
#else
	ulong writeval;
#endif
	ulong	addr, count;
	int	size;
	void *buf, *start;

	if ((argc < 3) || (argc > 4))
		return CMD_RET_USAGE;

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 1)
		return 0;

	/* Address is specified since argc > 1
	*/
	addr = simple_strtoul(argv[1], NULL, 16);

	/* Get the value to write.
	*/
#ifdef MEM_SUPPORT_64BIT_DATA
	writeval = simple_strtoull(argv[2], NULL, 16);
#else
	writeval = simple_strtoul(argv[2], NULL, 16);
#endif

	/* Count ? */
	if (argc == 4) {
		count = simple_strtoul(argv[3], NULL, 16);
	} else {
		count = 1;
	}

	start = (void *)addr;
	buf = start;
	while (count-- > 0) {
		if (size == 4)
			*((u32 *)buf) = (u32)writeval;
#ifdef MEM_SUPPORT_64BIT_DATA
		else if (size == 8)
			*((u64 *)buf) = (u64)writeval;
#endif
		else if (size == 2)
			*((u16 *)buf) = (u16)writeval;
		else
			*((u8 *)buf) = (u8)writeval;
		buf += size;
	}
	return 0;
}

#ifdef CONFIG_CMD_MX_CYCLIC
static int do_mem_mdc(int argc, char **argv)
{
	int i;
	ulong count;

	count = bl_atoi(argv[3]);

	for (;;) {
		do_mem_md(3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			puts("Abort\n");
			return 0;
		}
	}

	return 0;
}

static int do_mem_mwc(int argc, char **argv)
{
	int i;
	ulong count;

	count = bl_atoi(argv[3]);

	for (;;) {
		do_mem_mw(3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			puts("Abort\n");
			return 0;
		}
	}

	return 0;
}
#endif /* CONFIG_CMD_MX_CYCLIC */

static int do_mem_cmp(int argc, char **argv)
{
	ulong	addr1, addr2, count, ngood;
	int	size;
	const char *type;
	const void *buf1, *buf2, *base;
#ifdef MEM_SUPPORT_64BIT_DATA
	u64 word1, word2;
#else
	ulong word1, word2;
#endif

	if (argc != 4)
		return CMD_RET_USAGE;

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 0;
	type = size == 8 ? "double word" :
	       size == 4 ? "word" :
	       size == 2 ? "halfword" : "byte";

	addr1 = simple_strtoul(argv[1], NULL, 16);

	addr2 = simple_strtoul(argv[2], NULL, 16);

	count = simple_strtoul(argv[3], NULL, 16);

	base = buf1 = (void *)addr1;
	buf2 = (void *)addr2;
	for (ngood = 0; ngood < count; ++ngood) {
		if (size == 4) {
			word1 = *(u32 *)buf1;
			word2 = *(u32 *)buf2;
#ifdef MEM_SUPPORT_64BIT_DATA
		} else if (size == 8) {
			word1 = *(u64 *)buf1;
			word2 = *(u64 *)buf2;
#endif
		} else if (size == 2) {
			word1 = *(u16 *)buf1;
			word2 = *(u16 *)buf2;
		} else {
			word1 = *(u8 *)buf1;
			word2 = *(u8 *)buf2;
		}
		if (word1 != word2) {
			ulong offset = buf1 - base;
#ifdef MEM_SUPPORT_64BIT_DATA
			printf("%s at 0x%p (%#0*llx) != %s at 0x%p (%#0*llx)\n",
			       type, (void *)(addr1 + offset), size, word1,
			       type, (void *)(addr2 + offset), size, word2);
#else
			printf("%s at 0x%08lx (%#0*lx) != %s at 0x%08lx (%#0*lx)\n",
				type, (ulong)(addr1 + offset), size, word1,
				type, (ulong)(addr2 + offset), size, word2);
#endif
			break;
		}

		buf1 += size;
		buf2 += size;
	}

	printf("Total of %ld %s(s) were the same\n", ngood, type);
	return 0;
}

static int do_mem_cp(int argc, char **argv)
{
	ulong	addr, dest, count;
	void	*src, *dst;
	int	size;

	if (argc != 4)
		return CMD_RET_USAGE;

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 0;

	addr = simple_strtoul(argv[1], NULL, 16);

	dest = simple_strtoul(argv[2], NULL, 16);

	count = simple_strtoul(argv[3], NULL, 16);

	if (count == 0) {
		puts ("Zero length ???\n");
		return 0;
	}

	src = (void *)addr;
	dst = (void *)dest;

#ifdef CONFIG_MTD_NOR_FLASH
	/* check if we are copying to Flash */
	if (addr2info((ulong)dst)) {
		int rc;

		puts ("Copy to Flash... ");

		rc = flash_write((char *)src, (ulong)dst, count * size);
		if (rc != 0) {
			flash_perror(rc);
			return 0;
		}
		puts ("done\n");
		return 0;
	}
#endif

	memcpy(dst, src, count * size);

	return 0;
}

static int do_mem_loop(int argc, char **argv)
{
	ulong	addr, length, i;
	int	size;
#ifdef MEM_SUPPORT_64BIT_DATA
	volatile u64 *llp;
#endif
	volatile u32 *longp;
	volatile u16 *shortp;
	volatile u8 *cp;
	const void *buf;

	/*
	 * Check for a size specification.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 0;

	/* Address is always specified.
	*/
	addr = simple_strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = simple_strtoul(argv[2], NULL, 16);

	buf = (void *)addr;

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
#ifdef MEM_SUPPORT_64BIT_DATA
		if (size == 8) {
			llp = (u64 *)buf;
			for (;;)
				i = *llp;
		}
#endif
		if (size == 4) {
			longp = (u32 *)buf;
			for (;;)
				i = *longp;
		}
		if (size == 2) {
			shortp = (u16 *)buf;
			for (;;)
				i = *shortp;
		}
		cp = (u8 *)buf;
		for (;;)
			i = *cp;
	}

#ifdef MEM_SUPPORT_64BIT_DATA
	if (size == 8) {
		for (;;) {
			llp = (u64 *)buf;
			i = length;
			while (i-- > 0)
				*llp++;
		}
	}
#endif
	if (size == 4) {
		for (;;) {
			longp = (u32 *)buf;
			i = length;
			while (i-- > 0)
				*longp++;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (u16 *)buf;
			i = length;
			while (i-- > 0)
				*shortp++;
		}
	}
	for (;;) {
		cp = (u8 *)buf;
		i = length;
		while (i-- > 0)
			*cp++;
	}

	return 0;
}

#ifdef CONFIG_LOOPW
static int do_mem_loopw(int argc, char **argv)
{
	ulong	addr, length, i;
	int	size;
#ifdef MEM_SUPPORT_64BIT_DATA
	volatile u64 *llp;
	u64 data;
#else
	ulong	data;
#endif
	volatile u32 *longp;
	volatile u16 *shortp;
	volatile u8 *cp;
	void *buf;

	/*
	 * Check for a size specification.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 0;

	/* Address is always specified.
	*/
	addr = simple_strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = simple_strtoul(argv[2], NULL, 16);

	/* data to write */
#ifdef MEM_SUPPORT_64BIT_DATA
	data = simple_strtoull(argv[3], NULL, 16);
#else
	data = simple_strtoul(argv[3], NULL, 16);
#endif

	buf = (void *)addr;

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
#ifdef MEM_SUPPORT_64BIT_DATA
		if (size == 8) {
			llp = (u64 *)buf;
			for (;;)
				*llp = data;
		}
#endif
		if (size == 4) {
			longp = (u32 *)buf;
			for (;;)
				*longp = data;
		}
		if (size == 2) {
			shortp = (u16 *)buf;
			for (;;)
				*shortp = data;
		}
		cp = (u8 *)buf;
		for (;;)
			*cp = data;
	}

#ifdef MEM_SUPPORT_64BIT_DATA
	if (size == 8) {
		for (;;) {
			llp = (u64 *)buf;
			i = length;
			while (i-- > 0)
				*llp++ = data;
		}
	}
#endif
	if (size == 4) {
		for (;;) {
			longp = (u32 *)buf;
			i = length;
			while (i-- > 0)
				*longp++ = data;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (u16 *)buf;
			i = length;
			while (i-- > 0)
				*shortp++ = data;
		}
	}
	for (;;) {
		cp = (u8 *)buf;
		i = length;
		while (i-- > 0)
			*cp++ = data;
	}
}
#endif /* CONFIG_LOOPW */

#ifdef CONFIG_CMD_MEMTEST
static ulong mem_test_alt(vu_long *buf, ulong start_addr, ulong end_addr,
			  vu_long *dummy)
{
	vu_long *addr;
	ulong errs = 0;
	ulong val, readback;
	int j;
	vu_long offset;
	vu_long test_offset;
	vu_long pattern;
	vu_long temp;
	vu_long anti_pattern;
	vu_long num_words;
	static const ulong bitpattern[] = {
		0x00000001,	/* single bit */
		0x00000003,	/* two adjacent bits */
		0x00000007,	/* three adjacent bits */
		0x0000000F,	/* four adjacent bits */
		0x00000005,	/* two non-adjacent bits */
		0x00000015,	/* three non-adjacent bits */
		0x00000055,	/* four non-adjacent bits */
		0xaaaaaaaa,	/* alternating 1/0 */
	};

	num_words = (end_addr - start_addr) / sizeof(vu_long);

	/*
	 * Data line test: write a pattern to the first
	 * location, write the 1's complement to a 'parking'
	 * address (changes the state of the data bus so a
	 * floating bus doesn't give a false OK), and then
	 * read the value back. Note that we read it back
	 * into a variable because the next time we read it,
	 * it might be right (been there, tough to explain to
	 * the quality guys why it prints a failure when the
	 * "is" and "should be" are obviously the same in the
	 * error message).
	 *
	 * Rather than exhaustively testing, we test some
	 * patterns by shifting '1' bits through a field of
	 * '0's and '0' bits through a field of '1's (i.e.
	 * pattern and ~pattern).
	 */
	addr = buf;
	for (j = 0; j < sizeof(bitpattern) / sizeof(bitpattern[0]); j++) {
		val = bitpattern[j];
		for (; val != 0; val <<= 1) {
			*addr = val;
			*dummy  = ~val; /* clear the test data off the bus */
			readback = *addr;
			if (readback != val) {
				printf("FAILURE (data line): "
					"expected %08lx, actual %08lx\n",
						val, readback);
				errs++;
				if (ctrlc())
					return 0;
			}
			*addr  = ~val;
			*dummy  = val;
			readback = *addr;
			if (readback != ~val) {
				printf("FAILURE (data line): "
					"Is %08lx, should be %08lx\n",
						readback, ~val);
				errs++;
				if (ctrlc())
					return 0;
			}
		}
	}

	/*
	 * Based on code whose Original Author and Copyright
	 * information follows: Copyright (c) 1998 by Michael
	 * Barr. This software is placed into the public
	 * domain and may be used for any purpose. However,
	 * this notice must not be changed or removed and no
	 * warranty is either expressed or implied by its
	 * publication or distribution.
	 */

	/*
	* Address line test

	 * Description: Test the address bus wiring in a
	 *              memory region by performing a walking
	 *              1's test on the relevant bits of the
	 *              address and checking for aliasing.
	 *              This test will find single-bit
	 *              address failures such as stuck-high,
	 *              stuck-low, and shorted pins. The base
	 *              address and size of the region are
	 *              selected by the caller.

	 * Notes:	For best results, the selected base
	 *              address should have enough LSB 0's to
	 *              guarantee single address bit changes.
	 *              For example, to test a 64-Kbyte
	 *              region, select a base address on a
	 *              64-Kbyte boundary. Also, select the
	 *              region size as a power-of-two if at
	 *              all possible.
	 *
	 * Returns:     0 if the test succeeds, 1 if the test fails.
	 */
	pattern = (vu_long) 0xaaaaaaaa;
	anti_pattern = (vu_long) 0x55555555;

	bl_dbg_printf("%s:%d: length = 0x%.8lx\n", __func__, __LINE__, num_words);
	/*
	 * Write the default pattern at each of the
	 * power-of-two offsets.
	 */
	for (offset = 1; offset < num_words; offset <<= 1)
		addr[offset] = pattern;

	/*
	 * Check for address bits stuck high.
	 */
	test_offset = 0;
	addr[test_offset] = anti_pattern;

	for (offset = 1; offset < num_words; offset <<= 1) {
		temp = addr[offset];
		if (temp != pattern) {
			printf("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx\n",
				start_addr + offset*sizeof(vu_long),
				pattern, temp);
			errs++;
			if (ctrlc())
				return 0;
		}
	}
	addr[test_offset] = pattern;

	/*
	 * Check for addr bits stuck low or shorted.
	 */
	for (test_offset = 1; test_offset < num_words; test_offset <<= 1) {
		addr[test_offset] = anti_pattern;

		for (offset = 1; offset < num_words; offset <<= 1) {
			temp = addr[offset];
			if ((temp != pattern) && (offset != test_offset)) {
				printf("\nFAILURE: Address bit stuck low or"
					" shorted @ 0x%.8lx: expected 0x%.8lx,"
					" actual 0x%.8lx\n",
					start_addr + offset*sizeof(vu_long),
					pattern, temp);
				errs++;
				if (ctrlc())
					return 0;
			}
		}
		addr[test_offset] = pattern;
	}

	/*
	 * Description: Test the integrity of a physical
	 *		memory device by performing an
	 *		increment/decrement test over the
	 *		entire region. In the process every
	 *		storage bit in the device is tested
	 *		as a zero and a one. The base address
	 *		and the size of the region are
	 *		selected by the caller.
	 *
	 * Returns:     0 if the test succeeds, 1 if the test fails.
	 */
	num_words++;

	/*
	 * Fill memory with a known pattern.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		addr[offset] = pattern;
	}

	/*
	 * Check each location and invert it for the second pass.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		temp = addr[offset];
		if (temp != pattern) {
			printf("\nFAILURE (read/write) @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\n",
				start_addr + offset*sizeof(vu_long),
				pattern, temp);
			errs++;
			if (ctrlc())
				return 0;
		}

		anti_pattern = ~pattern;
		addr[offset] = anti_pattern;
	}

	/*
	 * Check each location for the inverted pattern and zero it.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		anti_pattern = ~pattern;
		temp = addr[offset];
		if (temp != anti_pattern) {
			printf("\nFAILURE (read/write): @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\n",
				start_addr + offset*sizeof(vu_long),
				anti_pattern, temp);
			errs++;
			if (ctrlc())
				return 0;
		}
		addr[offset] = 0;
	}

	return 0;
}

static ulong mem_test_quick(vu_long *buf, ulong start_addr, ulong end_addr,
			    vu_long pattern, int iteration)
{
	vu_long *end;
	vu_long *addr;
	ulong errs = 0;
	ulong incr, length;
	ulong val, readback;

	/* Alternate the pattern */
	incr = 1;
	if (iteration & 1) {
		incr = -incr;
		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if (pattern & 0x80000000)
			pattern = -pattern;	/* complement & increment */
		else
			pattern = ~pattern;
	}
	length = (end_addr - start_addr) / sizeof(ulong);
	end = buf + length;
	printf("\rPattern %08lX  Writing..."
		"%12s"
		"\b\b\b\b\b\b\b\b\b\b",
		pattern, "");

	for (addr = buf, val = pattern; addr < end; addr++) {
		*addr = val;
		val += incr;
	}

	puts("Reading...");

	for (addr = buf, val = pattern; addr < end; addr++) {
		readback = *addr;
		if (readback != val) {
			ulong offset = addr - buf;

			printf("\nMem error @ 0x%08X: "
				"found %08lX, expected %08lX\n",
				(uint)(uintptr_t)(start_addr + offset*sizeof(vu_long)),
				readback, val);
			errs++;
			if (ctrlc())
				return 0;
		}
		val += incr;
	}

	return 0;
}

/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CONFIG_SYS_ALT_MEMTEST. The complete test loops until
 * interrupted by ctrl-c or by a failure of one of the sub-tests.
 */
static int do_mem_mtest(int argc, char **argv)
{
	ulong start, end;
	vu_long *buf, *dummy;
	ulong iteration_limit = 0;
	ulong errs = 0;	/* number of errors, or -1 if interrupted */
	ulong pattern = 0;
	int iteration;
#if defined(CONFIG_SYS_ALT_MEMTEST)
	const int alt_test = 1;
#else
	const int alt_test = 0;
#endif

	start = bl_atoi(argv[1]);
	end = bl_atoi(argv[2]);

	switch (argc) {
	case 5:
		iteration_limit = bl_atoi(argv[4]);
	case 4:
		pattern = bl_atoi(argv[3]);
		break;
	default:
		break;
	}

	if (end < start) {
		bl_printf("EE: end < start: refusing to do empty test\n");
		return 0;
	}

	bl_printf(
		"Testing %08lx ... %08lx w/ [%lx] for %lx iter.:\n",
		start, end, pattern, iteration_limit);
	bl_dbg_printf("%s:%d: start %#08lx end %#08lx\n", __func__, __LINE__,
	      start, end);

	buf = (void *)start;
	dummy = (void *)(CONFIG_SYS_MEMTEST_SCRATCH);
	for (iteration = 0;
			!iteration_limit || iteration < iteration_limit;
			iteration++) {
		if (ctrlc()) {
			errs = -1UL;
			break;
		}

		printf("Iteration: %6d\r", iteration + 1);
		bl_dbg_printf("\n");
		if (alt_test) {
			errs = mem_test_alt(buf, start, end, dummy);
		} else {
			errs = mem_test_quick(buf, start, end, pattern,
					      iteration);
		}
		if (errs == -1UL)
			break;
	}

	if (errs == -1UL) {
		/* Memory test was aborted - write a newline to finish off */
		bl_puts("\n");
	} else {
		printf("Tested %d iteration(s) with %lu errors.\n",
			iteration, errs);
	}

	return 0;
}
#endif	/* CONFIG_CMD_MEMTEST */

/**************************************************/
U_BOOT_CMD(
	md,	2,	1,	do_mem_md,
	"memory display",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address [# of objects]"
#else
	"[.b, .w, .l] address [# of objects]"
#endif
);


U_BOOT_CMD(
	mw,	3,	1,	do_mem_mw,
	"memory write (fill)",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address value [count]"
#else
	"[.b, .w, .l] address value [count]"
#endif
);

U_BOOT_CMD(
	cp,	4,	1,	do_mem_cp,
	"memory copy",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] source target count"
#else
	"[.b, .w, .l] source target count"
#endif
);

U_BOOT_CMD(
	cmp,	4,	1,	do_mem_cmp,
	"memory compare",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] addr1 addr2 count"
#else
	"[.b, .w, .l] addr1 addr2 count"
#endif
);

U_BOOT_CMD(
	loopr, 3, 1, do_mem_loop,
	"infinite read loop on address range",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address number_of_objects"
#else
	"[.b, .w, .l] address number_of_objects"
#endif
);

#ifdef CONFIG_LOOPW
U_BOOT_CMD(
	loopw,	4,	1,	do_mem_loopw,
	"infinite write loop on address range",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address number_of_objects data_to_write"
#else
	"[.b, .w, .l] address number_of_objects data_to_write"
#endif
);
#endif /* CONFIG_LOOPW */

#ifdef CONFIG_CMD_MEMTEST
U_BOOT_CMD(
	mtest,	3,	1,	do_mem_mtest,
	"simple RAM read/write test",
	"start end [pattern [iterations]]"
);
#endif	/* CONFIG_CMD_MEMTEST */

#ifdef CONFIG_CMD_MX_CYCLIC
U_BOOT_CMD(
	mdc,	4,	1,	do_mem_mdc,
	"memory display cyclic",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address count delay(ms)"
#else
	"[.b, .w, .l] address count delay(ms)"
#endif
);

U_BOOT_CMD(
	mwc,	4,	1,	do_mem_mwc,
	"memory write cyclic",
#ifdef MEM_SUPPORT_64BIT_DATA
	"[.b, .w, .l, .q] address value delay(ms)"
#else
	"[.b, .w, .l] address value delay(ms)"
#endif
);
#endif /* CONFIG_CMD_MX_CYCLIC */
