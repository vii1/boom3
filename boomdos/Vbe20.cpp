//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   vbe20.cpp - VESA BIOS EXTENSION 2.0 support implementation         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _VBE20_CPP
#include "vbe20.h"
#include "dos4gw.h"
#include <mem.h>
#include <malloc.h>
#include <stdio.h>
#include <conio.h>

// 4 byte color
struct Tcolor4 {
  byte b;
  byte g;
  byte r;
  byte reserved;
};

// pointers to DOS memory
static Tvbeinfo *pinfo=NULL;
static Tmodeinfo *pmode=NULL;
static Tcolor4 *ppal=NULL;
static word infosel,modesel,palsel;

static byte *vbebuffer; // linear frame buffer
static byte *membuffer; // additional buffer in system memory
static int curpagesize; // current page size in bytes
static int curpage;     // current page number (0 or 1)
static int curmode=0;   // current video mode number

// VBE 2.0 is initialized
static bool vbeinitfl;

static Tvbeinfo vbeinfo; // VBE controller information
static Tmodeinfo vbemode; // VBE video mode information

// initializes VESA BIOS EXTENSION
Tvbeinfo *vbe_init( void )
{
  REGPACKX r;
  short *modep;
  vbeinitfl=false;

  // allocates DOS memory for VBE structures
  pmode=NULL;
  ppal=NULL;
  if (!dosalloc(sizeof(Tvbeinfo),&pinfo,&infosel)) goto err;
  if (!dosalloc(sizeof(Tmodeinfo),&pmode,&modesel)) goto err;
  if (!dosalloc(256*sizeof(Tcolor4),&ppal,&palsel)) goto err;

  // gets the VBE controller information
  memset(pinfo,0,sizeof(Tvbeinfo));
  pinfo->VbeSignature='2EBV';
  r.eax=0x4F00;
  r.es=(dword)pinfo>>4;
  r.edi=0;
  if (!int16(0x10,&r)) goto err;
  memcpy(&vbeinfo,pinfo,sizeof(Tvbeinfo));
  if ((word)r.eax!=0x4F) goto err;
  if (vbeinfo.VbeVersion<0x200) goto err;
  dosfree(infosel);
  pinfo=NULL;

  // converts some pointers to flat mode
  vbeinfo.OemStringPtr=(char  *)realtoflat(vbeinfo.OemStringPtr);
  vbeinfo.VideoModePtr=(short *)realtoflat(vbeinfo.VideoModePtr);
  vbeinfo.OemVendorNamePtr=(char *)realtoflat(vbeinfo.OemVendorNamePtr);
  vbeinfo.OemProductRevPtr=(char *)realtoflat(vbeinfo.OemProductRevPtr);
  vbeinfo.OemProductNamePtr=(char *)realtoflat(vbeinfo.OemProductNamePtr);

  // gets the video modes
  modep=vbeinfo.VideoModePtr;
  vbemodenum=0;
  for (;*modep!=-1;modep++) {
    if (!vbe_getmodeinfo(*modep)) continue;
    if ((vbemode.ModeAttributes&0x90)!=0x90) continue;
    if (vbemode.BitsPerPixel!=8 && vbemode.BitsPerPixel!=16 &&
    vbemode.BitsPerPixel!=24 && vbemode.BitsPerPixel!=32) continue;
    vbemodes[vbemodenum].number=*modep;
    vbemodes[vbemodenum].width=vbemode.XResolution;
    vbemodes[vbemodenum].height=vbemode.YResolution;
    vbemodes[vbemodenum].bpp=vbemode.BitsPerPixel;
    vbemodenum++;
  }
  if (vbemodenum==0) goto err;
  vbeinitfl=true;
  return &vbeinfo;
 err:
  // frees the allocated resources and enables only mode 13 (320x200x8)
  vbe_done();
  vbemodes[0].width=320;
  vbemodes[0].height=200;
  vbemodes[0].bpp=8;
  vbemodenum=1;
  return NULL;
}

// sets text mode
void vbe_settext( void )
{
  union REGS r;

  r.x.eax=3;
  int386(0x10,&r,&r);
}

// frees all allocated resources
void vbe_done( void )
{
  if (pmode) dosfree(modesel);
  if (ppal) dosfree(palsel);
  if (vbebuffer) freelinear(vbebuffer);
  if (membuffer) free(membuffer);
  vbeinitfl=false;
}

// gets the information for a video mode
Tmodeinfo *vbe_getmodeinfo(int mode)
{
  REGPACKX r;

  r.eax=0x4F01;
  r.ecx=mode;
  r.es=(dword)pmode>>4;
  r.edi=0;
  if (!int16(0x10,&r)) return 0;
  memcpy(&vbemode,pmode,sizeof(Tmodeinfo));
  vbemode.WinFuncPtr=(char *)realtoflat(vbemode.WinFuncPtr);
  if ((word)r.eax==0x4F) return &vbemode;
  else return NULL;
}

// sets the current video mode
bool vbe_setmode(int mode,bool backbuffl)
{
  union REGS r;

  if (membuffer) free(membuffer),membuffer=NULL;
  if (vbebuffer!=NULL) freelinear(vbebuffer),vbebuffer=NULL;
  curmode=0;

  if (mode==-1) {
    // sets mode 13
    r.x.eax=0x13;
    int386(0x10,&r,&r);
    vbe_width=320;
    vbe_height=200;
    vbe_bpp=8;
    curpage=0;
    curpagesize=320*200;
    membuffer=(byte *)malloc(curpagesize);
    if (!membuffer) return false;
    vbe_backbuffer=membuffer;
    curmode=-1;
    return true;
  }
  if (!vbeinitfl) return false;

  // sets a VBE 2.0 video mode with linear frame buffer
  r.w.ax=0x4F02;
  r.w.bx=mode|0x4000;
  if (!int386(0x10,&r,&r)) return false;
  if (r.w.ax!=0x4F) return false;
  vbedac=6;
  if (!vbe_getmodeinfo(mode)) return false;
  vbebuffer=NULL;
  if (!getlinear(vbemode.PhysBasePtr,vbeinfo.TotalMemory*65536,&vbebuffer)) return false;
  vbe_width=vbemode.XResolution;
  vbe_height=vbemode.YResolution;
  vbe_bpp=vbemode.BitsPerPixel;
  curpage=0;
  curpagesize=vbe_width*vbe_height*(vbe_bpp/8);

  // creates a back buffer in system memory
  if (vbemode.NumberOfImagePages==0 || !backbuffl) {
    membuffer=(byte *)malloc(curpagesize);
    if (!membuffer) return false;
    vbe_backbuffer=membuffer;
  }
  else {
    vbe_setpage(1,false);
    vbe_backbuffer=vbebuffer;
  }
  curmode=mode;
  return true;
}

// finds a video mode number
int vbe_findmode(int width,int height,int bpp)
{
  if (!vbeinitfl)
    if (width==320 && height==200 && bpp==8) return -1;
    else return 0;
  for (int i=0;i<vbemodenum;i++)
    if (vbemodes[i].width==width && vbemodes[i].height==height &&
        vbemodes[i].bpp==bpp)
      return vbemodes[i].number;
  return 0;
}

// gets the current video mode number
int vbe_getcurrentmode( void )
{
  if (!vbeinitfl) return 0;
  union REGS r;

  r.w.ax=0x4F03;
  if (!int386(0x10,&r,&r)) return 0;
  if (r.w.ax!=0x4F) return 0;
  return r.w.bx;
}

// sets the position of the begining of the screen in the video memory
bool vbe_setstart(int x,int y,bool waitfl)
{
  if (!vbeinitfl) return false;
  union REGS r;

  r.w.ax=0x4F07;
  r.w.bx=waitfl?0x80:0x00;
  r.w.cx=x;
  r.w.dx=y;
  if (!int386(0x10,&r,&r)) return false;
  return r.w.ax==0x4F;
}

// gets the position of the begining of the screen in the video memory
bool vbe_getstart(word *x,word *y)
{
  if (!vbeinitfl) return false;
  union REGS r;

  r.w.ax=0x4F07;
  r.w.bx=1;
  if (!int386(0x10,&r,&r)) return false;
  if (r.w.ax!=0x4F) return false;
  *x=r.w.cx;
  *y=r.w.dx;
  return true;
}

// sets the current video page
bool vbe_setpage(int page,bool waitfl)
{
  if (!vbeinitfl) return false;
  if (page>vbemode.NumberOfImagePages) return false;
  return vbe_setstart(0,page*vbemode.YResolution,waitfl);
}

// flips the video pages
void vbe_flip(bool waitfl)
{
  if (curmode==-1) {
    if (membuffer) memcpy((void *)0xA0000,membuffer,curpagesize);
  }
  else if (vbeinitfl) {
    if (membuffer) memcpy(vbebuffer,membuffer,curpagesize);
    else {
      vbe_setpage(curpage,waitfl);
      curpage=1-curpage;
      vbe_backbuffer=vbebuffer+curpage*curpagesize;
    }
  }
}

// sets DAC to 6 bit
bool vbe_setdac6( void )
{
  if (!vbeinitfl) return false;
  union REGS r;

  r.w.ax=0x4F08;
  r.w.bx=0x0600;
  if (!int386(0x10,&r,&r)) return false;
  if (r.w.ax==0x4F) {
    vbedac=6;
    return true;
  }
  return false;
}

// sets DAC to 8 bit
bool vbe_setdac8( void )
{
  if (!vbeinitfl) return false;
  union REGS r;

  r.w.ax=0x4F08;
  r.w.bx=0x0800;
  if (!int386(0x10,&r,&r)) return false;
  if (r.w.ax==0x4F) {
    vbedac=8;
    return true;
  }
  return false;
}

// sets the current palette
bool vbe_setpal(Tcolor *pal,int first,int number)
{
  if (vbeinitfl) {
    // sets VBE 2.0 palette
    REGPACKX r;

    if (vbedac==6) {
      for (int i=0;i<number;i++) {
        ppal[i].r=pal[i].r>>2;
        ppal[i].g=pal[i].g>>2;
        ppal[i].b=pal[i].b>>2;
      }
    }
    else {
      for (int i=0;i<number;i++) {
        ppal[i].r=pal[i].r;
        ppal[i].g=pal[i].g;
        ppal[i].b=pal[i].b;
      }
    }

    r.eax=0x4F09;
    r.ebx=0;
    r.ecx=number;
    r.edx=first;
    r.es=(dword)ppal>>4;
    r.edi=0;
    if (!int16(0x10,&r)) return false;
    return (word)r.eax==0x4F;
  }
  else {
    // sets mode 13 palette
    byte pal3[256][3];
    for (int i=0;i<number;i++) {
      pal3[i][0]=pal[i].r>>2;
      pal3[i][1]=pal[i].g>>2;
      pal3[i][2]=pal[i].b>>2;
    }
    union REGS r;
    struct SREGS s;

    r.w.ax=0x1012;
    r.w.bx=first;
    r.w.cx=number;
    r.x.edx=FP_OFF(pal3);
    segread(&s);
    s.es=FP_SEG(pal3);
    int386x(0x10,&r,&r,&s);
    return true;
  }
}

// gets the current palette
bool vbe_getpal(Tcolor *pal,int first,int number)
{
  if (!vbeinitfl) return false;
  REGPACKX r;

  r.eax=0x4F09;
  r.ebx=1;
  r.ecx=number;
  r.edx=first;
  r.es=(dword)ppal>>4;
  r.edi=0;
  if (!int16(0x10,&r) || (word)r.eax!=0x4F) return false;
  memcpy(ppal,pal,number*sizeof(Tcolor));
  return true;
}
