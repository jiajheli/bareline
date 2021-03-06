ifneq ($(MAKECMDGOALS),clean)
  ifndef PLAT
    PLAT_LIST := $(shell ls -d plat_*)
    $(error EE: unknown "PLAT"; allowed: "$(PLAT_LIST)")
  endif

  sinclude $(PLAT)/Makefile

  ifndef plat_src
    $(error EE: missing "plat_src")
  endif
endif

BIN ?= bareline.out
GCC ?= gcc
LD ?= ld
OD ?= objdump
OC ?= objcopy

BIN_DIR := ./bin
SRC_DIR := ./src

common_src += bareline.c bareline_builtin.c
common_src := $(addprefix $(SRC_DIR)/,$(common_src))
common_obj := $(common_src:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

CFLAGS := -Wall -g -Os -I $(SRC_DIR) -I $(PLAT)
LDFLAGS := -static

plat_src := $(addprefix $(PLAT)/,$(plat_src))
plat_obj := $(plat_src:$(PLAT)/%.c=$(BIN_DIR)/%.o)
plat_asm := $(addprefix $(PLAT)/,$(plat_asm))
plat_aobj := $(plat_asm:$(PLAT)/%.S=$(BIN_DIR)/%.o)

all: $(BIN_DIR) $(BIN_DIR)/$(BIN) $(BIN_DIR)/$(BIN)

$(BIN_DIR)/$(BIN): $(common_obj) $(plat_obj) $(plat_aobj)
	$(LD) $(LDFLAGS) $(PLAT_LDFLAGS) $^ -o $@
ifeq ($(DUMP),1)
	$(OD) -Dlxt $@ > $(@:.out=.dump)
endif
ifeq ($(BINARY),1)
	$(OC) --gap-fill=0x00 -O binary $@ $(@:.out=.bin)
endif

$(common_obj): $(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(CFLAGS) $(PLAT_CFLAGS) -c $^ -o $@

$(plat_obj): $(BIN_DIR)/%.o: $(PLAT)/%.c
$(plat_aobj): $(BIN_DIR)/%.o: $(PLAT)/%.S

$(plat_obj) $(plat_aobj):
	$(GCC) $(CFLAGS) $(PLAT_CFLAGS) -c $^ -o $@

$(BIN_DIR):
	mkdir -p $@

$(PLAT)/Makefile:
	$(error EE: missing "$@")

clean:
	rm -rf $(BIN_DIR)
