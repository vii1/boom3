//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mouse.cpp - Direct Input mouse implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _MOUSE_CPP
#include "mouse.h"
#include "win32.h"
#include "system.h"
#include <dinput.h>

static IDirectInput *dinput=NULL;
static IDirectInputDevice *dimouse=NULL;
static bool mousefl=false;

// frees all allocated resources
static void mouse_done( void )
{
  if (dimouse) dimouse->Unacquire(),dimouse->Release(),dimouse=NULL;
  if (dinput) dinput->Release(),dinput=NULL;
  mousefl=false;
}

// creates all mouse objects
bool mouse_init( void )
{
  ShowCursor(FALSE);
  atexit(mouse_done);
  if (DirectInputCreate(hinst,DIRECTINPUT_VERSION,&dinput,NULL)!=DI_OK) return false;
  if (dinput->CreateDevice(GUID_SysMouse,&dimouse,NULL)!=DI_OK) return false;
  if (dimouse->SetDataFormat(&c_dfDIMouse)!=DI_OK) return false;
  if (dimouse->SetCooperativeLevel(hwndMain,DISCL_NONEXCLUSIVE|DISCL_FOREGROUND)!=DI_OK) return false;
  mousefl=true;
  return true;
}

// reads the mouse position and fills mousedx and mousedy
void mouse_read( void )
{
  if (mousefl) {
    DIMOUSESTATE state;
    HRESULT err=dimouse->GetDeviceState(sizeof(state),&state);
    // if the mouse is lost, try to acquire it
    if (err==DIERR_NOTACQUIRED || err==DIERR_INPUTLOST) {
      dimouse->Acquire();
      err=dimouse->GetDeviceState(sizeof(state),&state);
    }
    if (err==DI_OK) {
      mousedx=state.lX;
      mousedy=state.lY;
    }
    else {
      mousedx=mousedy=0;
    }
  }
}
