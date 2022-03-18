//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mouse.cpp - MS-DOS mouse implementation                            //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _MOUSE_CPP
#include "mouse.h"
#include "engine/types.h"
#include "dos4gw.h"
#include <stdlib.h>

static short volatile mousex0,mousey0; // previous mouse position
static bool firstevent; // to skip the first mouse event

// dummy function to indicate org of the code to lock
static void mouse_lock_region_begin( void )
{
}

// disables stack check for the mouse handler
#pragma off (check_stack)

// mouse event handler
void _loadds far mouse_handler(short si,short di)
{
  if (!firstevent) {
    mousedx+=si-mousex0;
    mousedy+=di-mousey0;
  }
  mousex0=si;
  mousey0=di;
  firstevent=false;
}

// defines the mouse handler calling convention
#pragma aux mouse_handler parm [ESI] [EDI]

// dummy function to indicate end of the code to lock
static void mouse_lock_region_end( void )
{
}

// unhooks the mouse event handler
static void mouse_done( void )
{
  _asm {
    mov   eax,0Ch
    xor   ecx,ecx
    xor   edx,edx
    mov   es,dx
    int   33h
  }
}

// hooks the mouse event handler
void mouse_init( void )
{
  word result;

  _asm {
    xor  eax,eax
    int  33h
    mov  result,ax
  }
  if (result) {
    lockregion((void *) mouse_lock_region_begin,
                   (dword) mouse_lock_region_end-
                  (dword) mouse_lock_region_begin);
    mousedx=0;
    mousedy=0;
    firstevent=true;
    _asm {
      // sets the new mouse event handler
      mov  eax,0Ch
      mov  ecx,1
      mov  edx,offset mouse_handler
      push es
      mov  bx,seg mouse_handler
      mov  es,bx
      int  33h
      pop  es
    }
    atexit(mouse_done);
  }
}
