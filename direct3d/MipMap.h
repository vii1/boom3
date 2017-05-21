//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mipmap.h - Mipmap generation interface                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _MIPMAP_H
#define _MIPMAP_H

#include "texd3d.h"

// sets the current texture
// returns the maximum number of mipmaps for that texture
unsigned mipmap_give_map( void *src, unsigned w, unsigned h, tfENUM format, void *pal );
// calculates the next mipmap
bool mipmap_get_mip( void *dst, tfENUM format );
bool mipmap_get_mip( void *dst, void *ddpixelformat );

#endif
