TOOLS = /scratch/lea/bin/mips-sde-elf-
QEMUDIR = /scratch/lea/QEMU_sv/install/bin/

FILLTLB_SRC_FILE = filltlb.s
FILLTLB_OUT_FILE = filltlb.o
STARTUP_SRC_FILE = startup.s
STARTUP_OUT_FILE = startup.o
C_SRC_FILE = test.c
C_OUT_FILE = test.o
ELF_FILE = mips_program.elf
BIN_FILE = mips_program.bin

all: build

run: $(ELF_FILE)
	$(QEMUDIR)qemu-system-mipsel -d in_asm,cpu -D qemu.log -kernel mips_program.elf -nographic -singlestep -cpu Allegro -no-reboot > run.log
	python analyze.py

build: $(BIN_FILE)

$(C_OUT_FILE): $(C_SRC_FILE)
	$(TOOLS)gcc -mips32 -EL -G 0 -mno-abicalls -fno-pic -fno-builtin -c $^ -o $@

$(FILLTLB_OUT_FILE): $(FILLTLB_SRC_FILE)
	$(TOOLS)gcc -mips32 -EL -c $^ -o $@

$(STARTUP_OUT_FILE): $(STARTUP_SRC_FILE)
	$(TOOLS)gcc -mips32 -EL -c $^ -o $@

$(ELF_FILE): test.ld $(STARTUP_OUT_FILE) $(C_OUT_FILE) $(FILLTLB_OUT_FILE)
	$(TOOLS)ld -mips32 -EL -nostdlib -T $^ -o $@
	@echo "MIPS32 elf file: $(PWD)/$(ELF_FILE)"

$(BIN_FILE): $(ELF_FILE)
	$(TOOLS)objcopy -O binary $^ $@
	@echo "MIPS32 binary file: $(PWD)/$(BIN_FILE)"

clean:
	rm $(STARTUP_OUT_FILE) $(ELF_FILE) $(BIN_FILE) $(C_OUT_FILE) $(FILLTLB_OUT_FILE) qemu.log qemu.svtrace run.log
