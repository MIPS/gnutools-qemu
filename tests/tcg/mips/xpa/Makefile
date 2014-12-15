XPATEST_TOOLS ?= mips-sde-elf-
XPATEST_QEMUBIN ?=

CPU = mips64r2-generic
CPU32 = mips32r5-generic
ELF = mips_program.elf
ELF32 = mips_program64.elf
#DBG_OPTS = -d in_asm,cpu -D qemu.log -singlestep
CMD = qemu-system-mips64el $(DBG_OPTS) -kernel $(ELF) -cpu $(CPU) -nographic -no-reboot
CMD32 = qemu-system-mipsel $(DBG_OPTS) -kernel $(ELF32) -cpu $(CPU32) -nographic -no-reboot

OBJ = test.o startup.o filltlb.o
OBJ32 = test.o32 startup.o32 filltlb.o32

all: build

run: $(ELF) $(ELF32)
	$(XPATEST_QEMUBIN)$(CMD) > run.log
	$(XPATEST_QEMUBIN)$(CMD32) > run32.log
	python analyze.py run.log run32.log

build: $(ELF) $(ELF32)

%.o: %.S
	$(XPATEST_TOOLS)gcc -DTARGET64 -mips64r2 -mabi=o64 -EL -c $^ -o $@

%.o: %.c
	$(XPATEST_TOOLS)gcc -DTARGET64 -mips64r2 -mabi=o64 -EL -c $^ -o $@

%.o32: %.S
	$(XPATEST_TOOLS)gcc -mips32r2 -EL -c $^ -o $@

test.o32: test40.c
	$(XPATEST_TOOLS)gcc -mips32r2 -EL -c $^ -o $@

$(ELF): test.ld $(OBJ)
	$(XPATEST_TOOLS)ld -mips64r2 -EL -T $^ -o $@
	@echo "MIPS64R2 ELF: $(PWD)/$(ELF)"

$(ELF32): test.ld32 $(OBJ32)
	$(XPATEST_TOOLS)ld -mips32r2 -EL -T $^ -o $@
	@echo "MIPS32R2 ELF: $(PWD)/$(ELF)"

clean:
	rm $(OBJ) $(OBJ32) $(ELF) $(ELF32) qemu.log qemu.svtrace run.log run32.log
