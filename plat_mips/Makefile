common_src += printf.c
plat_src := bareline_example.c

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

PLAT_LDFLAGS := -T $(PLAT)/$(PLAT).lds -emain -EB

GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
