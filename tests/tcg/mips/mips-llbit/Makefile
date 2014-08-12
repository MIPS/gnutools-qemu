# Environment variables needed:
# TOOLS = (...)/bin/mips-sde-elf-
# QEMUDIR = (...)/bin/

CPU = MIPS64R6-generic
CPU32 = P5600

ELF = mips_program.elf
ELF32 = mips_program.elf32

CMD = qemu-system-mips64el -d in_asm,cpu -D qemu.log -kernel $(ELF) -cpu $(CPU) -nographic -no-reboot
CMD32 = qemu-system-mipsel -d in_asm,cpu -D qemu.log -kernel $(ELF32) -cpu $(CPU32) -nographic -no-reboot

OBJ = test.o startup.o exc_handler.o
OBJ32 = test.o32 startup.o32 exc_handler.o32

all: build

run: $(ELF) $(ELF32)
	$(QEMUDIR)$(CMD)
	$(QEMUDIR)$(CMD32)

build: $(ELF) $(ELF32)

%.o: %.S
	$(TOOLS)gcc -DREL6 -mips64r6 -mabi=o64 -EL -c $^ -o $@

%.o: %.c
	$(TOOLS)gcc -mips64r6 -mabi=o64 -EL -G 0 -c $^ -o $@

$(ELF): test.ld $(OBJ)
	$(TOOLS)ld -mips64r6 -EL -T $^ -o $@
	@echo "MIPS64 elf file: $(PWD)/$(ELF)"

%.o32: %.S
	$(TOOLS)gcc -mips32r2 -EL -c $^ -o $@

%.o32: %.c
	$(TOOLS)gcc -mips32r2 -EL -G 0 -c $^ -o $@

$(ELF32): test.ld32 $(OBJ32)
	$(TOOLS)ld -mips32r2 -EL -T $^ -o $@
	@echo "MIPS32 elf file: $(PWD)/$(ELF)"

clean:
	rm $(OBJ) $(OBJ32) $(ELF) $(ELF32) qemu.log qemu.svtrace
