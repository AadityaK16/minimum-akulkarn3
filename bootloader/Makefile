ARM_PREFIX ?= arm-none-eabi-
CC := $(ARM_PREFIX)gcc
OBJCOPY := $(ARM_PREFIX)objcopy
READELF := $(ARM_PREFIX)readelf
NM := $(ARM_PREFIX)nm

BUILD := build
ELF := $(BUILD)/bootloader.elf
GENERATED := $(BUILD)/bootloader.bin
CANONICAL := bootloader.bin
OBJECTS := \
	$(BUILD)/start.o \
	$(BUILD)/boot.o

DEPENDENCIES := $(OBJECTS:.o=.d)

ARCH_FLAGS := -mcpu=cortex-a9 -marm -mfloat-abi=soft

CFLAGS := $(ARCH_FLAGS) \
	-Os \
	-std=c11 \
	-ffreestanding \
	-fno-builtin \
	-fno-stack-protector \
	-fno-pic \
	-fno-pie \
	-fno-unwind-tables \
	-fno-asynchronous-unwind-tables \
	-fno-tree-loop-distribute-patterns \
	-fdata-sections \
	-ffunction-sections \
	-mno-unaligned-access \
	-Wall -Wextra -Werror -Wstack-usage=512 \
	-MMD -MP

ASFLAGS := $(ARCH_FLAGS) \
	-ffreestanding \
	-MMD -MP

LDFLAGS := $(ARCH_FLAGS) \
	-nostdlib -nostartfiles -nodefaultlibs \
	-static -no-pie \
	-Wl,-T,linker/bootloader.ld \
	-Wl,--gc-sections \
	-Wl,--build-id=none \
	-Wl,--orphan-handling=error \
	-Wl,--fatal-warnings \
	-Wl,--no-undefined \
	-Wl,--require-defined=_start \
	-Wl,-Map,$(BUILD)/bootloader.map

.PHONY: all check update clean

all: $(GENERATED)

$(BUILD)/start.o: src/start.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/boot.o: src/boot.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJECTS) linker/bootloader.ld
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(GENERATED): $(ELF)
	$(OBJCOPY) -O binary --gap-fill 0 --pad-to 0x10000 $< $@

check: $(GENERATED) $(CANONICAL)
	@test "$$($(READELF) -h $(ELF) | awk '/Entry point address:/ { print $$4 }')" = "0x0"
	@test -z "$$($(NM) -u $(ELF))"
	@test "$$(wc -c < $(GENERATED))" -eq 65536
	@! $(READELF) -s $(ELF) | grep -q '\$$t'
	cmp $(CANONICAL) $(GENERATED)

update: $(GENERATED)
	cp $(GENERATED) $(CANONICAL)

clean:
	rm -rf $(BUILD)

-include $(DEPENDENCIES)
