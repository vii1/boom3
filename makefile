!include os.mif

# ROOT		= $+$(%cwd)$-
ROOT		= .
CONFIG		= release
CPU			= 386
TARGET_SYS	= dos
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)

BOOMDOS_EXE = $(ROOT)$(SEP)boomdos$(EXE_SUFFIX)

all: engine boomdos .SYMBOLIC
	@%null

boomdos: engine .SYMBOLIC
	$(MAKE) -f $(ROOT)$(SEP)boomdos$(SEP)makefile TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(COPY) $(OUTDIR_BASE)$(SEP)boomdos$(SEP)$(CONFIG).$(CPU)$(SEP)boomdos$(EXE_SUFFIX) $(BOOMDOS_EXE)

# !include fixmath/fixmath.mif
!include engine/engine.mif

clean: .SYMBOLIC
	$(MAKE) -f $(ROOT)$(SEP)boomdos$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(MAKE) -f $(ROOT)$(SEP)engine$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
#	cd $(ROOT)$(SEP)fixmath
#	$(MAKE) clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
#	cd $+$(%cwd)$-
