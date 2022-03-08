//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   grtext.h - string writting interface                               //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _GRTEXT_H
#define _GRTEXT_H
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
extern dword scr_cols; // screen width
extern dword scr_bpp;  // pixel depth
void textout_asm(void *addr,char *str,int color); // writes a string in a buffer
#ifdef __cplusplus
}
#endif

#pragma aux textout_asm parm [edi] [esi] [edx] modify [eax ebx ecx edx esi edi]

// writes a string in a buffer at given position
inline void textout(void *buffer,int x,int y,char *str,int color)
{
  textout_asm(((char *)buffer)+y*scr_cols+x*scr_bpp/8,str,color);
}

#endif
