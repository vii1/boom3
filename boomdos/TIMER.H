//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   kbd.cpp - MS-DOS version timer handling implementation             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TIMER_H
#define _TIMER_H

#ifdef _TIMER_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

// miliseconds from the begining
GLOBAL long volatile timer_clocks;

#undef GLOBAL

void timer_init( void );

#endif
