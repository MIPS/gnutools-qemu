# Environment variables needed:
# TOOLS = (...)/bin/mips-img-elf-
# QEMUDIR = (...)/bin/

# QEMU specific command line
CPU = mips32r6-generic
CPU64 = mips64r6-generic

BIN_EL = qemu-system-mipsel
BIN_EB = qemu-system-mips
BIN_EL64 = qemu-system-mips64el
BIN_EB64 = qemu-system-mips64

ARGS = -cpu $(CPU) -nographic -semihosting-config arg=program_e,arg="a",arg="b b",arg=ccc,arg="dd\" d \"d"
ARGS64 = -cpu $(CPU64) -nographic -semihosting-config arg=program_e,arg="a",arg="b b",arg=ccc,arg="dd\" d \"d"

CMD_EL = $(BIN_EL) -kernel program_el $(ARGS)
CMD_EB = $(BIN_EB) -kernel program_eb $(ARGS)

CMD_EL64 = $(BIN_EL64) -kernel program_el64 $(ARGS64)
CMD_EB64 = $(BIN_EB64) -kernel program_eb64 $(ARGS64)

CMD_EL64_n32 = $(BIN_EL64) -kernel program_eln32 $(ARGS64)
CMD_EB64_n32 = $(BIN_EB64) -kernel program_ebn32 $(ARGS64)

all: build build64
run: build run_el run_eb run_el64 run_eb64
build: program_eb program_el
build64: program_eb64 program_el64 program_ebn32 program_eln32

run_el: program_el
	$(QEMUDIR)/$(CMD_EL) || [ $$? -eq 88 ]
	rm tmptestprog*

run_eb: program_eb
	$(QEMUDIR)/$(CMD_EB) || [ $$? -eq 88 ]
	rm tmptestprog*

run_el64: program_el64 program_eln32
	$(QEMUDIR)/$(CMD_EL64) || [ $$? -eq 88 ]
	rm tmptestprog*
	$(QEMUDIR)/$(CMD_EL64_n32) || [ $$? -eq 88 ]
	rm tmptestprog*

run_eb64: program_eb64 program_ebn32
	$(QEMUDIR)/$(CMD_EB64) || [ $$? -eq 88 ]
	rm tmptestprog*
	$(QEMUDIR)/$(CMD_EB64_n32) || [ $$? -eq 88 ]
	rm tmptestprog*

program_eb: main.c
	$(TOOLS)gcc main.c -Tuhi32.ld -mips32r6 -EB -o program_eb

program_el: main.c
	$(TOOLS)gcc main.c -Tuhi32.ld -mips32r6 -EL -o program_el

program_eb64: main.c
	$(TOOLS)gcc main.c -Tuhi64_64.ld -mips64r6 -mabi=64 -EB -o program_eb64

program_el64: main.c
	$(TOOLS)gcc main.c -Tuhi64_64.ld -mips64r6 -mabi=64 -EL -o program_el64

program_ebn32: main.c
	$(TOOLS)gcc main.c -Tuhi64_n32.ld -mips64r6 -mabi=n32 -EB -o program_ebn32

program_eln32: main.c
	$(TOOLS)gcc main.c -Tuhi64_n32.ld -mips64r6 -mabi=n32 -EL -o program_eln32

clean:
	rm program_e*
	rm tmptestprog*
	rm -r ./chroot*
