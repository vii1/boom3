//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   system.cpp - system functions implementation                       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _SYSTEM_CPP
#include "system.h"
#include "../win32/win32.h"
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////

static char halt_message[256] = "";
static shutdown_handler *shutdown_handlers = 0;
static idle_handler *idle_handlers = NULL;

////////////////////////////////////////////////////////////////////////////
// shutdown handling

shutdown_handler *register_shutdown_handler( shutdown_handler *shutdown )
{
  shutdown_handler *sh = shutdown_handlers;
  shutdown_handlers = shutdown;
  return sh;
}

static void main_shutdown_handler( void )
{
  if (*halt_message)
    system_msg( "Fatal Error", halt_message );
}

static void system_call_shutdown_handlers( void )
{
  shutdown_handlers();
}

////////////////////////////////////////////////////////////////////////////
// idle handling

idle_handler *register_idle_handler( idle_handler *idle )
{
  idle_handler *ih = idle_handlers;
  idle_handlers = idle;
  return ih;
}

static void dummy_idle_handler( long system_time )
{
}

void system_call_idle_handlers( long system_time )
{
  idle_handlers( system_time );
}

////////////////////////////////////////////////////////////////////////////
// message and error handling

void system_msg( const char *title, const char *msg, ... )
{
  va_list arguments;
  va_start( arguments, msg );
  system_msg( title, msg, arguments );
  va_end( arguments );
}

bool system_err( const char *message, ... )
{
  va_list arguments;
  va_start( arguments, message );
  system_err( message, arguments );
  va_end( arguments );
  return false;
}

void system_msg( const char *title, const char *msg, va_list arguments )
{
  char buf[256];
  vsprintf( buf, msg, arguments );
  MessageBox( hwndMain, buf, title, MB_OK );
}

bool system_err( const char *message, va_list arguments )
{
  char buf[256];
  vsprintf( halt_message, message, arguments );
  sprintf( buf, "%s\n", halt_message );
  system_dbg( buf );
  return false;
}

void system_err( const char *title, const char *error, va_list arguments )
{
  system_msg( title, error, arguments );
}

#if !defined(NDEBUG) || defined(_DEBUG)
void system_dbg( const char *msg, ... )
{
  va_list arguments;
  va_start( arguments, msg );
  system_dbg( msg, arguments );
  va_end( arguments );
}

void system_dbg( const char *msg, va_list arguments )
{
  char buf[256];
  vsprintf( buf, msg, arguments );
  OutputDebugString( buf );
}
#endif

////////////////////////////////////////////////////////////////////////////

// initializes the system
void system_init( void )
{
  register_shutdown_handler( main_shutdown_handler );
  register_idle_handler( dummy_idle_handler );
  atexit( system_call_shutdown_handlers );
}

// terminates the program
void halt( int condition, char *message, ... )
{
  if (condition) {
    va_list arguments;
    va_start( arguments, message );
    vsprintf( halt_message, message, arguments );
    va_end( arguments );
    exit( -1 );
  }
}

void halt( int condition )
{
  if (condition) exit( -1 );
}
