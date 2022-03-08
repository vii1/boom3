//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   types.h - basic type and macro definitions                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TYPES_H
#define _TYPES_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

////////////////////////////////////////////////////////////////////////////

//general typedefs
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef float angle;
typedef double coord3d;
typedef float coord2d;

////////////////////////////////////////////////////////////////////////////

//general struct definitions
typedef struct Tvertex {
  coord3d x,y;
} Tvertex;

typedef struct Tvector {
  coord3d x,y,z;
} Tvector;

#endif
