//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   system.h - system functions interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////
// shutdown handling

typedef void shutdown_handler( void );
shutdown_handler *register_shutdown_handler( shutdown_handler *shutdown );

////////////////////////////////////////////////////////////////////////////
// idle handling

typedef void idle_handler( long system_time );
idle_handler *register_idle_handler( idle_handler *idle );
void system_call_idle_handlers( long system_time );

////////////////////////////////////////////////////////////////////////////
// message and error handling

void system_msg( const char *title, const char *msg, ... );
void system_msg( const char *title, const char *msg, va_list arguments );
bool system_err( const char *message, ... );
bool system_err( const char *message, va_list arguments );
#if !defined(NDEBUG) || defined(_DEBUG)
void system_dbg( const char *msg, ... );
void system_dbg( const char *msg, va_list arguments );
#else
#define system_dbg if(0)
#endif

////////////////////////////////////////////////////////////////////////////

// initializes the system
void system_init( void );
// terminates the program
void halt( int condition, char *message, ... );
void halt( int condition );

#endif
