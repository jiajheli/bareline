common_src += printf.c uart_16550.c xmodem.c mem.c
plat_src := bareline_example.c plat_dep.c
plat_asm := start.S

BIN := demo.out

PLAT_CFLAGS := -fno-pic -fno-builtin -mno-abicalls -mno-memcpy -EB -std=c99

ifdef UART_BASE_ADDR
PLAT_CFLAGS += -DUART_BASE_ADDR=$(UART_BASE_ADDR)
else
$(error EE: UART_BASE_ADDR is undefined)
endif

ifndef CROSS_COMPILE
$(warning WW: --------------------------)
$(warning WW: CROSS_COMPILE is undefined)
$(warning WW: --------------------------)
endif

PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_FLOAT
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_LONG_LONG

PLAT_LDFLAGS := -T $(PLAT)/$(PLAT).lds -emips_entry -EB

GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OD := $(CROSS_COMPILE)objdump
