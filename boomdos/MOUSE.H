//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mouse.h - MS-DOS mouse interface                                   //
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

GLOBAL int volatile mousedx,mousedy;

#undef GLOBAL

// hooks the mouse event handler
void mouse_init( void );

#endif
