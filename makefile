!include os.mif

ROOT		= $+$(%cwd)$-
CONFIG		= release
CPU			= 386
TARGET_SYS	= dos
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)

MAKE = $(MAKE) -h

BOOMDOS_EXE = $(ROOT)$(SEP)boomdos$(EXE_SUFFIX)

all: engine boomdos .SYMBOLIC
	@%null

boomdos: engine .SYMBOLIC
	cd $(ROOT)$(SEP)boomdos
	$(MAKE) TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
	cd $+$(%cwd)$-
	$(COPY) $(OUTDIR_BASE)$(SEP)boomdos$(SEP)$(CONFIG).$(CPU)$(SEP)boomdos$(EXE_SUFFIX) $(BOOMDOS_EXE)

# !include fixmath/fixmath.mif
!include engine/engine.mif

clean: .SYMBOLIC
	cd $(ROOT)$(SEP)boomdos
	$(MAKE) clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
	cd $(ROOT)$(SEP)engine
	$(MAKE) clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
#	cd $(ROOT)$(SEP)fixmath
#	$(MAKE) clean TARGET_SYS=$(TARGET_SYS) CONFIG=$(CONFIG) CPU=$(CPU)
	cd $+$(%cwd)$-
