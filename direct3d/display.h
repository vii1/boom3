//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   display.h - Direct 3D display functions interface                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <ddraw.h>
#include <d3d.h>

#define _DISPLAY_MAX_DRIVERS     8
#define _DISPLAY_MAX_DEVICES     8
#define _DISPLAY_MAX_MODES       64
#define _DISPLAY_MAX_DEVICE_NAME 32
#define _DISPLAY_MAX_DEVICE_DESC 64
#define _DISPLAY_MAX_DRIVER_NAME 32
#define _DISPLAY_MAX_DRIVER_DESC 64

struct DisplayDriverDesc
{
  GUID *guid;
  char name[_DISPLAY_MAX_DRIVER_NAME];
  char description[_DISPLAY_MAX_DRIVER_DESC];
  GUID guid_storage;
};

struct DisplayDeviceDesc
{
  GUID *guid;
  char name[_DISPLAY_MAX_DEVICE_NAME];
  char description[_DISPLAY_MAX_DEVICE_DESC];
  D3DDEVICEDESC d3d_device_desc_hw;
  D3DDEVICEDESC d3d_device_desc_hel;
  D3DDEVICEDESC *d3d_device_desc;
  bool hardware;
  GUID guid_storage;
};

struct DisplayModeDesc
{
  unsigned width, height;
  unsigned bit_depth;
  unsigned r, g, b;
};

bool display_init();

bool display_drivers_available( DisplayDriverDesc **drivers, int *ndrivers );
bool display_driver_init( const DisplayDriverDesc *driver );
bool display_driver_done();

bool display_devices_available( DisplayDeviceDesc **devices, int *ndevices );
bool display_device_init( const DisplayDeviceDesc *device );
bool display_device_done();

bool display_modes_available( DisplayModeDesc **modes, int *nmodes );
bool display_mode_init( unsigned nback_buffers, bool z_buffer, const DisplayModeDesc *mode );
bool display_mode_init( unsigned nback_buffers, bool z_buffer, unsigned width, unsigned height, unsigned bit_depth=16, unsigned r=0, unsigned g=0, unsigned b=0 );
bool display_mode_done();


#ifdef _DISPLAY_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL const DisplayDriverDesc  *display_current_driver;
GLOBAL const DisplayDeviceDesc  *display_current_device;
GLOBAL const DisplayModeDesc    *display_current_mode;
GLOBAL IDirectDraw4             *dd_object;
GLOBAL IDirectDrawSurface4      *dd_buffer_front;
GLOBAL IDirectDrawSurface4      *dd_buffer_back;
GLOBAL IDirectDrawSurface4      *dd_zbuffer;
GLOBAL IDirect3D3               *d3d_object;
GLOBAL IDirect3DDevice3         *d3d_device;
GLOBAL IDirect3DViewport3       *d3d_viewport;

#undef GLOBAL

#endif
