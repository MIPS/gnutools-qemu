# Required:
# CROSS = .../bin/mips-sde-elf-
# QEMU  = .../bin/
# TARGET = mips32r5 or mips64r6

CC=$(CROSS)gcc
LD=$(CROSS)ld

ifeq ($(QEMU_TARGET),mips32r5)
	CFLAGS  = -mips32r2 -mmsa -EL -g -DMIPS32
	LDFLAGS = -mips32r2 -EL -T linker.ld -g
	CPU = MIPS32R5-generic
	BINARY = qemu-system-mipsel
else
	CFLAGS  = -mips64r6 -mmsa -mabi=o64 -EL -g -DMIPS64
	LDFLAGS = -mips64r6 -EL -T linker.ld -g
	CPU = MIPS64R6-generic
	BINARY = qemu-system-mips64el
endif

HELPERS = head.o helper.o exceptions.o

TESTS += test-simple.tst
TESTS += test-msaldsw.tst
TESTS += test-lpa_regs.tst

RUN = $(QEMU)/$(BINARY) -cpu $(CPU) -nographic -no-reboot -d in_asm -D qemu.log

all: build

build: $(TESTS)

%.o: %.S
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

%.tst: %.o linker.ld $(HELPERS) macros.inc Makefile
	@echo LD $@
	@$(LD) $(LDFLAGS) $< $(HELPERS) -o $@

run: $(addprefix run-, $(TESTS))

run-%.tst: %.tst
	$(RUN) -kernel ./$<

clean:
	rm $(TESTS) qemu.log
