!include ../os.mif

PROJECT 	= boomdos

ROOT		= ..
CONFIG		= release
CPU			= 386
TARGET_SYS	= dos
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)
OUTDIR		= $(OUTDIR_BASE)$(SEP)$(PROJECT)$(SEP)$(CONFIG).$(CPU)

OPTIONS		= -bt=$(WATCOM_BT) -w1 -q -zp4 -mf -I$(ROOT)

!ifeqi CONFIG debug
OPTIONS		+= -d2 -D_DEBUG
!else
OPTIONS		+= -onatx
!endif

!ifeq CPU 586
OPTIONS		+= -5r -fp5
!else
OPTIONS		+= -3r -fpi
!endif

EXE			= $(PROJECT)$(EXE_SUFFIX)

OBJS		= boomdos.obj dos4gw.obj grtext.obj kbd.obj mouse.obj timer.obj vbe20.obj

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)$(PROJECT) mkdir $(OUTDIR_BASE)$(SEP)$(PROJECT)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: $(EXE) .SYMBOLIC
	@%null

# !include ../fixmath/fixmath.mif
!include ../engine/engine.mif

OPTIONS += $(INCLUDE) $(DEFINE)

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS)
	*wlink &
		option quiet &
!ifeqi CONFIG debug
		debug all &
!endif
		$(WLINK_TARGET_SYS) &
		option eliminate &
		option stack=32768 &
		path $(OUTDIR) &
		name $^@ &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.c: .
.obj: $(OUTDIR)

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -za99 -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

.cpp.obj: .AUTODEPEND
	*wpp386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

.asm.obj: .AUTODEPEND
	*wasm -i=$(ROOT) -q -fo=$(OUTDIR)\$^. -fr=$(OUTDIR)\$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
