# Environment variables needed:
# TOOLS = (...)/bin/mips-sde-elf-
# QEMUDIR = (...)/bin/

CPU = MIPS64R6-generic
ELF = mips_program.elf
CMD = qemu-system-mips64el -d in_asm,cpu -D qemu.log -kernel $(ELF) -cpu $(CPU) -nographic -singlestep -no-reboot

OBJ = test.o startup.o exc_handler.o

all: build

run: $(ELF)
	$(QEMUDIR)$(CMD)

build: $(ELF)

%.o: %.s
	$(TOOLS)gcc -mips64r2 -mabi=o64 -EL -c $^ -o $@

%.o: %.c
	$(TOOLS)gcc -mips64r2 -mabi=o64 -EL -G 0 -c $^ -o $@

$(ELF): test.ld $(OBJ)
	$(TOOLS)ld -mips64r2 -EL -T $^ -o $@
	@echo "MIPS32 elf file: $(PWD)/$(ELF)"

clean:
	rm $(OBJ) $(ELF) qemu.log qemu.svtrace
