!include os.mif

# ROOT		= $+$(%cwd)$-
ROOT		= .
CONFIG		= release
CPU			= 386
TARGET_SYS	= dos
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)

BOOMDOS_EXE = $(ROOT)$(SEP)boomdos$(EXE_SUFFIX)
BOOMEDIT_EXE = $(ROOT)$(SEP)boomedit$(EXE_SUFFIX)

all: engine boomdos .SYMBOLIC
!ifeq TARGET_SYS win
all: boomedit
!endif
	@%null

boomdos: engine .SYMBOLIC
	$(MAKE) -f $(ROOT)$(SEP)boomdos$(SEP)makefile TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(COPY) $(OUTDIR_BASE)$(SEP)boomdos$(SEP)$(CONFIG).$(CPU)$(SEP)boomdos$(EXE_SUFFIX) $(BOOMDOS_EXE)

boomedit: .SYMBOLIC
	$(MAKE) -f $(ROOT)$(SEP)boomedit$(SEP)makefile TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(COPY) $(OUTDIR_BASE)$(SEP)boomedit$(SEP)$(CONFIG).$(CPU)$(SEP)boomedit$(EXE_SUFFIX) $(BOOMEDIT_EXE)

# !include fixmath/fixmath.mif
!include engine/engine.mif

clean: .SYMBOLIC
	$(MAKE) -f $(ROOT)$(SEP)boomdos$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(MAKE) -f $(ROOT)$(SEP)boomedit$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(MAKE) -f $(ROOT)$(SEP)engine$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT)
	$(MAKE) -f $(ROOT)$(SEP)engine$(SEP)makefile clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU) ROOT=$(ROOT) EDITOR=1
#	cd $(ROOT)$(SEP)fixmath
#	$(MAKE) clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
#	cd $+$(%cwd)$-
