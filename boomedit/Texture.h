//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   texture.h - texture editing interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "engine/types.h"

#ifdef _TEXTURE_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL int defwalltxt,deffloortxt,defceiltxt;

#undef GLOBAL

//adds a new texture 
bool newtexture(int w,int h,char *name,byte *data);
// changes the image in the texture
bool changetexture(int txt,int w,int h,char *name,byte *data);
// checks if the texture is used by any walls
bool textureused(int txt);
// deletes a texture
void deltexture(int txt);
// creates a default texture
void defaulttexture( void );
#endif