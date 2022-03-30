!ifndef ROOT
!include ../os.mif
!else
!include $(ROOT)/os.mif
!endif

PROJECT 	= engine

!ifdef ROOT
PROJ_DIR 	= $(ROOT)$(SEP)$(PROJECT)
!else
PROJ_DIR	= .
ROOT		= ..
!endif

CONFIG		= release
CPU			= 386
OUTDIR_BASE	= $(ROOT)$(SEP)build.$(TARGET_SYS)
OUTDIR		= $(OUTDIR_BASE)$(SEP)$(PROJECT)$(SEP)$(CONFIG).$(CPU)

OPTIONS		= -bt=$(WATCOM_BT) -w1 -we -q -zp4 -mf -I$(ROOT) #-za99

!ifeqi CONFIG debug
OPTIONS		+= -d2 -D_DEBUG -DDEBUG_PREVIEW -xs
!else
OPTIONS		+= -onatx
!endif

!ifeq CPU 586
OPTIONS		+= -5r -fp5
!else
OPTIONS		+= -3r -fpi
!endif

LIB			= $(PROJECT)$(LIB_SUFFIX)
SOURCEPATH  = $(PROJ_DIR)

OBJS = Collide.obj Draw.obj Geom.obj Player.obj Read.obj Tclip.obj Tcluster.obj Thole.obj &
			Tline.obj Tmap.obj Tmapitem.obj Tportal.obj Tsector.obj Ttexture.obj Ttrap.obj Twall.obj Write.obj

!ifeqi CONFIG debug
OBJS		+= mono.obj debug.obj
SOURCEPATH +=;$(ROOT)$(SEP)mono
!endif


.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)$(PROJECT) mkdir $(OUTDIR_BASE)$(SEP)$(PROJECT)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: $(LIB) .SYMBOLIC
	@%null

# !include ../fixmath/fixmath.mif

OPTIONS += $(INCLUDE) $(DEFINE)

$(LIB): $(OUTDIR)$(SEP)$(LIB) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(LIB): $(OBJS)
#	wlib -n -q $^@
#	@for %i in ($(OBJS)) do wlib -q $^@ +$(OUTDIR)$(SEP)%i
	*wlib -n -q $^@ +$(OUTDIR)$(SEP)$(OBJS: = +$(OUTDIR)$(SEP))

.c: $(SOURCEPATH)
.cpp: $(SOURCEPATH)
.obj: $(OUTDIR)

.ERASE
.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. $[@

.ERASE
.cpp.obj: .AUTODEPEND
	*wpp386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. $[@

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(LIB) $(DELETE) $(OUTDIR)$(SEP)$(LIB)
