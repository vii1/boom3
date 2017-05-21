//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   draw2d.h - 2D map drawing interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _DRAW2D_H
#define _DRAW2D_H

#include "types.h"

#include <windows.h>

#define MoveTo(hdc,x,y) MoveToEx(hdc,x,y,NULL)

// draws the XY view
void drawxy(HDC hdc);
// draws the XZ view
void drawxz(HDC hdc);
// draws the YZ view
void drawyz(HDC hdc);

// draws the frame
void drawframe(HDC hdc);
// initialization
void draw2d_init( void );


#endif
