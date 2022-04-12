!ifndef ROOT
!include ../os.mif
!else
!include $(ROOT)/os.mif
!endif

PROJECT 	= boomedit

!ifdef ROOT
PROJ_DIR 	= $(ROOT)$(SEP)$(PROJECT)
!else
PROJ_DIR	= .
ROOT		= ..
!endif

CONFIG		= release
CPU			= 386
TARGET_SYS	= win
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)
OUTDIR		= $(OUTDIR_BASE)$(SEP)$(PROJECT)$(SEP)$(CONFIG).$(CPU)

OPTIONS		= -bt=$(WATCOM_BT) -w1 -q -zp4 -mf -I$(ROOT) #-xs

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
SOURCEPATH  = $(PROJ_DIR);$(ROOT)$(SEP)Win32

OBJS		= BoomEdit.obj Cluster.obj Contour.obj Draw2d.obj Line.obj Readbmp.obj Sector.obj &
			  Select.obj Texture.obj Vertex.obj &
			  Dialog.obj Win32.obj

RESOURCE	= $(OUTDIR)$(SEP)boomedit.res

STDLIBS		= winmm

!ifneq TARGET_SYS win
!error BoomEdit is available only for Windows. Use TARGET_SYS=win
!endif

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)$(PROJECT) mkdir $(OUTDIR_BASE)$(SEP)$(PROJECT)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: $(EXE) .SYMBOLIC
	@%null

EDITOR = 1
# !include ../fixmath/fixmath.mif
!include $(ROOT)/engine/engine.mif

OPTIONS += $(INCLUDE) $(DEFINE)

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS) $(RESOURCE)
	*wlink &
		option quiet &
!ifeqi CONFIG debug
		debug all &
!endif
		system nt_win &
		option eliminate &
		option stack=32768 &
		path $(OUTDIR) &
		name $^@ &
		file { $(OBJS) } &
		libfile { $(LIBS) } &
		resource { $(RESOURCE) } &
		library { $(STDLIBS) }

.SUFFIXES: .res .rc

.c: $(SOURCEPATH)
.cpp: $(SOURCEPATH)
.asm: $(SOURCEPATH)
.rc: $(SOURCEPATH)
.obj: $(OUTDIR)
.res: $(OUTDIR)

.ERASE
.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -za99 -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $[@

.ERASE
.cpp.obj: .AUTODEPEND
	*wpp386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $[@

.ERASE
.asm.obj: .AUTODEPEND
	*wasm -i=$(ROOT) -q -fo=$(OUTDIR)\$^. -fr=$(OUTDIR)\$^& $[@

.ERASE
.rc.res: .AUTODEPEND
	*wrc -q -r -ad -bt=nt -i=$(SOURCEPATH) -fo=$(OUTDIR)\$^. -fr=$(OUTDIR)\$^& $[@

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.res $(DELETE) $(OUTDIR)$(SEP)*.res
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
