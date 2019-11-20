common_src += printf.c
plat_src := bareline_example.c

BIN := demo.out

PLAT_CFLAGS := -fno-builtin -fno-pic
# fix-me: ignore warning about int & pointer casting in do_md()
PLAT_CFLAGS += -Wno-int-to-pointer-cast

ifndef CROSS_COMPILE
$(warning WW: --------------------------)
$(warning WW: CROSS_COMPILE is undefined)
$(warning WW: --------------------------)
endif

PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_FLOAT
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T
PLAT_CFLAGS += -DPRINTF_DISABLE_SUPPORT_LONG_LONG

PLAT_LDFLAGS := -T $(PLAT)/$(PLAT).lds -emain

GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
