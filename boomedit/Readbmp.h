//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   readbmp.h - Bitmap reading interface                               //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _READBMP_H
#define _READBMP_H

#include "engine/types.h"

// reads bitmap from a file
// returns the palette, bits and size
bool read_bmp(char *name, byte **data, byte **palette, int *width, int *height );

// reads palette from a file
bool read_pal(char *name, byte **palette);

#endif
