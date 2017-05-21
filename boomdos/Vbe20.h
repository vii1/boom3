//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   vbe20.h - VESA BIOS EXTENSION 2.0 support interface                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _VBE20_H
#define _VBE20_H

#include "types.h"

// VBE controller information
struct Tvbeinfo {
  long  VbeSignature;
  word  VbeVersion;
  char *OemStringPtr;
  dword Capabilities;
  short *VideoModePtr;
  word  TotalMemory;
  word  OemSoftwareRev;
  char *OemVendorNamePtr;
  char *OemProductNamePtr;
  char *OemProductRevPtr;
  char  Reserved[222];
  char  OemData[256];
};

// VBE video mode information
struct Tmodeinfo {
  word  ModeAttributes;
  byte  WinAAttributes;
  byte  WinBAttributes;
  word  WinGranularity;
  word  WinSize;
  word  WinASegment;
  word  WinBSegment;
  char *WinFuncPtr;
  word  BytesPerScanLine;
  word  XResolution;
  word  YResolution;
  byte  XCharSize;
  byte  YCharSize;
  byte  NumberOfPlanes;
  byte  BitsPerPixel;
  byte  NumberOfBanks;
  byte  MemoryModel;
  byte  BankSize;
  byte  NumberOfImagePages;
  byte  reserved1;
  byte  RedMaskSize;
  byte  RedFieldPosition;
  byte  GreenMaskSize;
  byte  GreenFieldPosition;
  byte  BlueMaskSize;
  byte  BlueFieldPosition;
  byte  RsvdMaskSize;
  byte  RsvdFieldPosition;
  byte  DirectColorModeInfo;
  dword PhysBasePtr;
  dword OffScreenMemOffset;
  word  OffScreenMemSize;
  char  reserved2[206];
};

// palette entry
struct Tcolor {
  byte r;
  byte g;
  byte b;
};

// video mode information
struct Tvbemode {
  short number;
  word  width;
  word  height;
  word  bpp;
};

#ifdef _VBE20_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL int vbedac; // DAC bit depth (6 or 8 bits)
GLOBAL int vbemodenum;  // number of video modes
GLOBAL Tvbemode vbemodes[256]; // video modes description
GLOBAL int vbe_width,vbe_height,vbe_bpp; // curent video mode info
GLOBAL byte *vbe_backbuffer; // current back buffer address

#undef GLOBAL

// initializes VESA BIOS EXTENSION
Tvbeinfo *vbe_init( void );
// frees all allocated resources
void vbe_done( void );
// sets the current video mode
bool vbe_setmode(int mode,bool backbuffl);
// sets text mode
void vbe_settext( void );
// gets the information for a video mode
Tmodeinfo *vbe_getmodeinfo(int mode);
// finds a video mode number
int vbe_findmode(int width,int height,int bpp);
// gets the current video mode number
int vbe_getcurrentmode( void );
// sets the position of the begining of the screen in the video memory
bool vbe_setstart(int x,int y,bool waitfl);
// gets the position of the begining of the screen in the video memory
bool vbe_getstart(word *x,word *y);
// sets the current video page
bool vbe_setpage(int page,bool waitfl);
// flips the video pages (if waitfl, waits for the vertical retrace)
void vbe_flip(bool waitfl);
// sets DAC to 6 bit
bool vbe_setdac6( void );
// sets DAC to 8 bit
bool vbe_setdac8( void );
// sets the current palette
bool vbe_setpal(Tcolor *pal,int first,int number);
// gets the current palette
bool vbe_getpal(Tcolor *pal,int first,int number);

#endif
