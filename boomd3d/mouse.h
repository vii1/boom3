//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mouse.cpp - Direct Input mouse interface                           //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _MOUSE_H
#define _MOUSE_H

#ifdef _MOUSE_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL int mousedx,mousedy;

#undef GLOBAL

// creates all mouse objects
bool mouse_init( void );
// reads the mouse position and fills mousedx and mousedy
void mouse_read( void );

#endif
