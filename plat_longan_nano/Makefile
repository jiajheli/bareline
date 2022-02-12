common_src += printf.c xmodem.c mem.c
plat_src := main.c plat_dep.c uart_gd32vf103.c cmd_timer.c
plat_asm := start.S

BIN := demo.out

PLAT_CFLAGS := -fno-pic -fno-builtin -march=rv32imac_zicsr

# ifdef UART_BASE_ADDR
# PLAT_CFLAGS += -DUART_BASE_ADDR=$(UART_BASE_ADDR)
# else
# $(error EE: UART_BASE_ADDR is undefined)
# endif

ifndef CROSS_COMPILE
$(warning WW: --------------------------)
$(warning WW: CROSS_COMPILE is undefined)
$(warning WW: --------------------------)
endif

PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_FLOAT
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_LONG_LONG

PLAT_LDFLAGS := -T $(PLAT)/$(PLAT).lds -elongan_nano_entry -nostdlib

GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OD := $(CROSS_COMPILE)objdump
