define help
	@echo "Make file removing INTERNAL ONLY features"
	@echo "USAGE:"
	@echo "	[QEMUROOT={PATH/TO/QEMU/TO/BE/CLEARED}] make [option]"
	@echo "options:"
	@echo "	help  Display this help and exit"
	@echo "	all  Remove all internal only features (default)"
	@echo "	dryrun  Do not actually change any files; just print what would happen"
	@echo "	clean  Remove any intermediate files if exists"
endef

QEMUROOT ?= ..

# Modify below target lists according to QEMU version
# NOTE: The lists of target files are only valid for 1.0.0
UNIFDEF_TARGETS = vl.c hw/mips_mipssim.c target-mips/helper.h target-mips/mips-defs.h target-mips/op_helper.c target-mips/translate.c target-mips/translate_init.c
RM_TARGETS = target-mips/mips-avp.h
# NOTE: It is assumed to have {FILENAME}.patch in the directory where Makefile is
PATCH_TARGETS = disas.c hw/mips_timer.c 
# List of defintions to be removed
UNIFDEF_OPTION = -UMIPSSIM_COMPAT -UMIPS_IGNORE_MTC0_TO_UNDEFINED 

UNIFDEF := $(shell which unifdef 2>/dev/null)
PYTHON := $(shell which python 2>/dev/null)
DOS2UNIX := $(shell which dos2unix 2>/dev/null)
DOS2UNIX_QUIET := -q

ifeq ($(UNIFDEF),)
	ifneq ($(PYTHON),)
		UNIFDEF=./ifdef.py
	endif
endif

ifeq ($(DOS2UNIX),)
	ifneq ($(PYTHON),)
		DOS2UNIX=./dos2unix.py
		DOS2UNIX_QUIET=
	endif
endif

DRYUNDEFRUN = $(addprefix $(QEMUROOT)/, $(UNIFDEF_TARGETS:=.dryundef))
UNDEFRUN = $(addprefix $(QEMUROOT)/, $(UNIFDEF_TARGETS:=.undef))
RMRUN = $(addprefix $(QEMUROOT)/, $(RM_TARGETS))
PATCHRUN = $(addprefix $(QEMUROOT)/, $(PATCH_TARGETS:=.patched))
DRYPATCHRUN = $(addprefix $(QEMUROOT)/, $(PATCH_TARGETS:=.drypatched))
EXISTRUN = $(addprefix $(QEMUROOT)/, $(UNIFDEF_TARGETS:=.exist))
EXISTRUN += $(addprefix $(QEMUROOT)/, $(RM_TARGETS:=.exist))
EXISTRUN += $(addprefix $(QEMUROOT)/, $(PATCH_TARGETS:=.exist))

all: check $(UNDEFRUN) rm_target $(PATCHRUN)
	@echo =====================================
	@echo completed removing INTERNAL ONLY 
	@echo now delete this directory manually
	@echo =====================================

dryrun: check $(DRYUNDEFRUN) dry_rm $(DRYPATCHRUN)
	@echo =====================================
	@echo completed dry removing INTERNAL ONLY 
	@echo =====================================

%.undef : %
	@echo undefining: $<
ifeq ($(DOS2UNIX),)
	@-$(DOS2UNIX) $(DOS2UNIX_QUIET) $<
endif
	@-$(UNIFDEF) $(UNIFDEF_OPTION) $< > $@ ; true
	@if test -e $@; then \
		cp -f $@ $<;\
		rm -rf $@; \
	fi

%.dryundef : %
	@echo dry undefining: $<
	@-$(UNIFDEF) $(UNIFDEF_OPTION) $< > /dev/null ; true

rm_target: 
	@echo removing: $(RMRUN)
	@-rm -f $(RMRUN)

dry_rm:
	@echo dry removing: $(RMRUN)

%.patched : %
	@echo patching: $< from $(notdir $(<:=.patch))
	@-patch -sN $< $(notdir $(<:=.patch)) -r $(notdir $(<:=.patch.rej)) ; true

%.drypatched: %
	@echo dry patching: $< from $(notdir $(<:=.patch))
	@-patch -N --dry-run $< $(notdir $(<:=.patch)) -r $(notdir $(<:=.patch.rej)) ; true

%.exist: %
	@if test -s $<; then echo $<; else echo error: cannot find $<; false; fi

clean: 
	-rm -f $(UNDEFRUN)
	-rm -f *.rej

envcheck:
	@if test -s $(UNIFDEF); then echo "unifdef ok"; true; else echo "unifdef or python required"; false; fi
	@if test -s $(DOS2UNIX); then echo "dos2unix ok"; true; else echo "dos2unix or python is missing. it may not work"; true; fi

filecheck: $(EXISTRUN)

check: envcheck filecheck

help:
	$(call help)
