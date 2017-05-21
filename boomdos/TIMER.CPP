//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   timer.cpp - MS-DOS timer handling implementation                   //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TIMER_CPP
#include "timer.h"
#include "dos4gw.h"

#include <dos.h>
#include <stdlib.h>

void (interrupt far *old_timer) ( void );
static long volatile timer_clocks55;

// dummy function to indicate org of the code to lock
static void timer_lock_region_begin( void )
{
}

// timer interrupt handler
static void _loadds interrupt timer_handler( void )
{
  timer_clocks++;
  timer_clocks55--;
  if (timer_clocks55==0) {
    timer_clocks55=55;
    old_timer();
  }
  else _asm {
    mov  al,20h
    out  20h,al
  }
}

// dummy function to indicate end of the code to lock
static void timer_lock_region_end( void )
{
}

// set timer speed
static void set_timer(word speed)
{
  _asm {
    cli
    mov   al,36h
    out   43h,al
    mov   cx,speed
    mov   al,cl
    out   40h,al
    mov   al,ch
    out   40h,al
    sti
  }
}

// unhooks the timer interrupt handler
static void timer_done( void )
{
  _dos_setvect(8,old_timer);
  set_timer(0);
}

// hooks the timer interrupt handler
void timer_init( void )
{
  set_timer(1192755/1000);
  lockregion((void *) timer_lock_region_begin,
                 (dword) timer_lock_region_end-
                 (dword) timer_lock_region_begin);
  old_timer=_dos_getvect(8);
  timer_clocks=0;
  timer_clocks55=55;
  _dos_setvect(8,timer_handler);
  atexit(timer_done);
}
