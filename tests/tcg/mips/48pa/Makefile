XPATEST_TOOLS ?= mips-sde-elf-
XPATEST_QEMUBIN ?=

CPU = mips64r6-generic
ELF = mips_program.elf
#DBG_OPTS = -d in_asm,cpu -D qemu.log -singlestep
CMD = qemu-system-mips64el $(DBG_OPTS) -kernel $(ELF) -cpu $(CPU) -nographic -no-reboot

OBJ = test.o startup.o filltlb.o

all: build

run: $(ELF)
	$(XPATEST_QEMUBIN)$(CMD) > run.log
	python analyze.py run.log

build: $(ELF) $(ELF32)

%.o: %.S
	$(XPATEST_TOOLS)gcc -DTARGET64 -mips64r6 -mabi=o64 -EL -c $^ -o $@

%.o: %.c
	$(XPATEST_TOOLS)gcc -DTARGET64 -mips64r6 -mabi=o64 -EL -c $^ -o $@

$(ELF): test.ld $(OBJ)
	$(XPATEST_TOOLS)ld -mips64r6 -EL -T $^ -o $@
	@echo "MIPS64R6 ELF: $(PWD)/$(ELF)"

clean:
	rm $(OBJ) $(OBJ32) $(ELF) $(ELF32) qemu.log qemu.svtrace run.log
