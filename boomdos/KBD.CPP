//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   kbd.cpp - MS-DOS keyboard handling implementation                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _KBD_CPP
#include "kbd.h"
#include "engine/types.h"
#include "engine/player.h"
#include "dos4gw.h"

#include <conio.h>
#include <dos.h>
#include <stdlib.h>

void (interrupt far *old_kbd) ( void );

static byte scan_codes[] = { 0x11, 0x1F, 75, 77, 0x1E, 0x20, 0x1D, 57, 0x47, 0x4F, 24, 25, 76, 42, 1, 0x3B };

// dummy function to indicate org of the code to lock
static void kbd_lock_region_begin( void )
{
}

// keyboard interrupt handler
static void _loadds interrupt keyboard_handler( void )
{
  byte k=(byte)inp(0x60);
  byte c=k&127;
  if (c==54) c=42; // use both shifts to run
  int i;
  dword m;

  for (i=0,m=1;i<sizeof(scan_codes)/sizeof(scan_codes[0]);i++,m<<=1)
    if (c==scan_codes[i]) {
      if (k&128) player_keys&=~m;
            else player_keys|=m;
      break;
    }
  old_kbd();
}

// dummy function to indicate end of the code to lock
void kbd_lock_region_end( void )
{
}

// unhooks the keyboard interrupt handler
static void kbd_done( void )
{
  _dos_setvect(9,old_kbd);
}

// hooks the keyboard interrupt handler
void kbd_init( void )
{
  lockregion((void *) kbd_lock_region_begin,
                 (dword) kbd_lock_region_end-
                 (dword) kbd_lock_region_begin);
  old_kbd=_dos_getvect(9);
  _dos_setvect(9,keyboard_handler);
  atexit(kbd_done);
}
