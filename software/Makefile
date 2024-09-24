# Phonies
.PHONY: all linux clean

# Global
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
ELF_DIR=$(BUILD_DIR)/elf
DUMP_DIR=$(BUILD_DIR)/dump
LINUX_CFLAGS= -Wall -Wextra -Wshadow -std=c99 -O2 -g 
RISCV_CFLAGS= -Wall -Wextra -Wshadow -std=c99 -O2
TOOLS_DIR=tools
SRC_DIR=src
INCLUDE_DIR=-I$(SRC_DIR) -I.

LINUX_TARGET=$(BUILD_DIR)/game.linux
RISCV_TARGET=$(ELF_DIR)/game.fpga.elf 

# RISC_V Compilation
LINKER_FILE=$(TOOLS_DIR)/rv32-board.lds
RISCV_CC=riscv64-unknown-elf-gcc
MARCH=rv32imf
MABI=ilp32f
MACH=-march=$(MARCH) -mabi=$(MABI) -mstrict-align -misa-spec=2.2 -mno-relax
LDFLAGS=-nostdlib -fno-exceptions -nostartfiles -ffreestanding -nodefaultlibs -T $(LINKER_FILE) -Wl,-Map=$(BUILD_DIR)/final.map -Wl,--no-relax

# Elf executable name

# Add your sources here
RISCV_STD_SOURCES=$(SRC_DIR)/std/math.c \
		$(SRC_DIR)/std/assert.c \
		$(SRC_DIR)/std/memory.c \
		$(SRC_DIR)/std/arith64.c \
		$(SRC_DIR)/cr0.c	

RISCV_STD_OBJ_NAMES=$(RISCV_STD_SOURCES:.c=.o)
RISCV_STD_OBJECTS=$(patsubst %,$(OBJ_DIR)/%,$(RISCV_STD_OBJ_NAMES))

.PHONY: all clean dump
all: linux dump

linux: $(LINUX_TARGET)
dump: $(DUMP_DIR)/dump_text.mif

clean:
	rm -rf build

$(RISCV_TARGET): $(RISCV_STD_OBJECTS) $(OBJ_DIR)/platform_fpga.o $(LINKER_FILE)
	@mkdir -p $(dir $@)
	$(RISCV_CC) $(MACH) $(LDFLAGS) -o $@ $(RISCV_STD_OBJECTS) $(OBJ_DIR)/platform_fpga.o

$(OBJ_DIR)/platform_fpga.o: $(SRC_DIR)/platform_fpga.c $(SRC_DIR)/game.c
	$(RISCV_CC) $(MACH) -I$(INCLUDE_DIR) $(RISCV_CFLAGS) -c -o $@ $(SRC_DIR)/platform_fpga.c

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(RISCV_CC) $(MACH) -I$(INCLUDE_DIR) $(RISCV_CFLAGS) -c -o $@ $^

# Dump rules
OBJCOPY_FLAGS=-R .comment -R .riscv.attributes -S --strip-unneeded -g --reverse-bytes=4 -O verilog --verilog-data-width 1

$(DUMP_DIR)/dump.bin: $(RISCV_TARGET)
	@mkdir -p $(dir $@)
	riscv64-unknown-elf-objcopy $(OBJCOPY_FLAGS) $^ $@

$(DUMP_DIR)/dump_text.mif $(DUMP_DIR)/dump_data.mif: $(DUMP_DIR)/dump.bin
	@mkdir -p $(dir $@)
	python3 $(TOOLS_DIR)/verilog_to_mif.py $^ $(DUMP_DIR)/dump


# Linux build
CC=gcc
LIBS=-lm $(shell pkg-config --libs glfw3 gl)

$(LINUX_TARGET): $(SRC_DIR)/platform_linux.c $(SRC_DIR)/game.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDE_DIR) $(LINUX_CFLAGS) -o $@ $(SRC_DIR)/platform_linux.c $(LIBS)

