XPATEST_TOOLS ?= mips-sde-elf-
XPATEST_QEMUBIN ?=

CPU = Allegro
ELF = mips_program.elf
#DBG_OPTS = -d in_asm,cpu -D qemu.log -singlestep
CMD = qemu-system-mipsel $(DBG_OPTS) -kernel $(ELF) -cpu $(CPU) -nographic -no-reboot

OBJ = test.o startup.o filltlb.o

all: build

run: $(ELF)
	$(XPATEST_QEMUBIN)$(CMD) > run.log
	python analyze.py

build: $(ELF)

%.o: %.s
	$(XPATEST_TOOLS)gcc -mips32r2 -EL -c $^ -o $@

%.o: %.c
	$(XPATEST_TOOLS)gcc -mips32r2 -EL -c $^ -o $@

$(ELF): test.ld $(OBJ)
	$(XPATEST_TOOLS)ld -mips32r2 -EL -T $^ -o $@
	@echo "MIPS32R2 ELF: $(PWD)/$(ELF)"

clean:
	rm $(ELF) $(OBJ) qemu.log qemu.svtrace run.log
