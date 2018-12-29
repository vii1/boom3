//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   display.cpp - Direct 3D display functions implementation           //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _DISPLAY_CPP
#include "display.h"
#include "system.h"
#include "../win32/win32.h"

#define FAIL(msg) { on_error=msg; goto error; };
static DisplayDriverDesc __drivers[_DISPLAY_MAX_DRIVERS];
static DisplayDeviceDesc __devices[_DISPLAY_MAX_DEVICES];
static DisplayModeDesc   __modes[_DISPLAY_MAX_MODES];
static unsigned __ndrivers, __ndevices, __nmodes;
static DDPIXELFORMAT zbuf_pixelformat;

static bool display_devices_enumerate();
static bool display_modes_enumerate();

////////////////////////////////////////////////////////////////////////////
static BOOL WINAPI enumerate_drivers_callback( GUID *lpGUID, LPSTR description, LPSTR name, LPVOID param )
{
  memset( &__drivers[__ndrivers], 0, sizeof(__drivers[__ndrivers]) );
  if (!lpGUID)
    __drivers[__ndrivers].guid = NULL;
  else {
    __drivers[__ndrivers].guid_storage = *lpGUID;
    __drivers[__ndrivers].guid = &__drivers[__ndrivers].guid_storage;
  }
  strncpy( __drivers[__ndrivers].name, name, _DISPLAY_MAX_DRIVER_NAME-1 )[_DISPLAY_MAX_DRIVER_NAME-1] = 0;
  strncpy( __drivers[__ndrivers].description, description, _DISPLAY_MAX_DRIVER_DESC-1 )[_DISPLAY_MAX_DRIVER_DESC-1] = 0;
  if (++__ndrivers<_DISPLAY_MAX_DRIVERS)
    return DDENUMRET_OK;
  else
    return DDENUMRET_CANCEL;
}

static BOOL WINAPI find_driver_callback( GUID *lpGUID, LPSTR description, LPSTR name, LPVOID param )
{
  DisplayDriverDesc *d = (DisplayDriverDesc*)param;
  if ((d->guid || lpGUID) && (!d->guid || !lpGUID || memcmp(d->guid,lpGUID,sizeof(GUID))))
    return DDENUMRET_OK;
  else {
    IDirectDraw *dd;
    if (DD_OK!=DirectDrawCreate(lpGUID,&dd,NULL)) {
      system_err( "DirectDrawCreate failed for %s", description );
      return DDENUMRET_CANCEL;
    }
    HRESULT r = dd->QueryInterface( IID_IDirectDraw4, (void**)&dd_object );
    dd->Release();
    if (DD_OK!=r) {
      system_err( "DirectX 6 required" );
      return DDENUMRET_CANCEL;
    }
    display_current_driver = d;
    return DDENUMRET_CANCEL;
  }
}

static bool display_drivers_enumerate()
{
  __ndrivers = 0;
  if (DD_OK!=DirectDrawEnumerate(enumerate_drivers_callback,NULL)) return system_err( "DirectDrawEnumerate failed" );
  if (!__ndrivers) return system_err( "No display drivers detected" );
  return true;
}

bool display_drivers_available( DisplayDriverDesc **drivers, int *ndrivers )
{
  *ndrivers = __ndrivers;
  *drivers = __drivers;
  return true;
}

bool display_driver_init( const DisplayDriverDesc *driver )
{
  char *on_error="";
  display_driver_done();
  if (!driver) {
    IDirectDraw *dd;
    if (DD_OK!=DirectDrawCreate(NULL,&dd,NULL)) FAIL( "DirectDrawCreate failed for Primary Display Driver" );
    HRESULT r = dd->QueryInterface( IID_IDirectDraw4, (void**)&dd_object );
    dd->Release();
    if (DD_OK!=r) FAIL( "DirectX 6 required" );
    display_current_driver = __drivers;
  }
  else {
    if (DD_OK!=DirectDrawEnumerate(find_driver_callback,(void*)driver)) FAIL( "DirectDrawEnumerate() failed" );
    if (dd_object==NULL) FAIL( "Requested driver not available" );
  }
  if (DD_OK!=dd_object->SetCooperativeLevel(hwndMain,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN)) FAIL( "SetCooperativeLevel failed" );
  if (DD_OK!=dd_object->QueryInterface(IID_IDirect3D3,(void**)&d3d_object)) FAIL( "DirectX 6 required" );
  if (!display_devices_enumerate()) goto error;
  return true;

error:
  display_driver_done();
  system_err( on_error );
  return false;
}

bool display_driver_done()
{
  bool r = display_device_done();
  if (d3d_object) d3d_object->Release();
  if (dd_object) dd_object->Release();
  d3d_object = NULL;
  dd_object = NULL;
  display_current_driver = NULL;
  return r;
}

////////////////////////////////////////////////////////////////////////////

static HRESULT WINAPI enumerate_devices_callback( GUID *lpGUID, LPSTR description, LPSTR name, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID param )
{
  memset( &__devices[__ndevices], 0, sizeof(__devices[__ndevices]) );
  if (!lpGUID)
    __devices[__ndevices].guid = NULL;
  else {
    __devices[__ndevices].guid_storage = *lpGUID;
    __devices[__ndevices].guid = &__devices[__ndevices].guid_storage;
  }
  strncpy( __devices[__ndevices].name, name, _DISPLAY_MAX_DEVICE_NAME-1 )[_DISPLAY_MAX_DEVICE_NAME-1] = 0;
  strncpy( __devices[__ndevices].description, description, _DISPLAY_MAX_DEVICE_DESC-1 )[_DISPLAY_MAX_DEVICE_DESC-1] = 0;
  __devices[__ndevices].d3d_device_desc_hw = *lpD3DHWDeviceDesc;
  __devices[__ndevices].d3d_device_desc_hel = *lpD3DHELDeviceDesc;
  __devices[__ndevices].hardware = (lpD3DHWDeviceDesc->dcmColorModel!=0);
  __devices[__ndevices].d3d_device_desc = __devices[__ndevices].hardware?
      &(__devices[__ndevices].d3d_device_desc_hw ):
      &(__devices[__ndevices].d3d_device_desc_hel);
  if (++__ndevices<_DISPLAY_MAX_DEVICES)
    return DDENUMRET_OK;
  else
    return DDENUMRET_CANCEL;
}

static HRESULT WINAPI find_device_callback( GUID *lpGUID, LPSTR description, LPSTR name, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID param )
{
  DisplayDeviceDesc *d = (DisplayDeviceDesc*)param;
  if ((d->guid || lpGUID) && (!d->guid || !lpGUID || memcmp(d->guid,lpGUID,sizeof(GUID))))
    return DDENUMRET_OK;
  else {
    display_current_device = d;
    return DDENUMRET_CANCEL;
  }
}

static HRESULT WINAPI enum_zbuffer_formats_callback( DDPIXELFORMAT* pddpf, VOID* pddpfDesired )
{
  if (NULL==pddpf || NULL==pddpfDesired) return D3DENUMRET_CANCEL;
  if (pddpf->dwFlags==((DDPIXELFORMAT*)pddpfDesired)->dwFlags) {
    memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );
    return D3DENUMRET_CANCEL;
  }
  return D3DENUMRET_OK;
}

static bool display_devices_enumerate()
{
  __ndevices=0;
  if (D3D_OK!=d3d_object->EnumDevices(enumerate_devices_callback,NULL)) return system_err( "d3d_object->EnumDevices() failed" );
  if (!__ndevices) return system_err( "No devices available" );
  return true;
}

bool display_devices_available( DisplayDeviceDesc **devices, int *ndevices )
{
  *ndevices = __ndevices;
  *devices = __devices;
  return true;
}

bool display_device_init( const DisplayDeviceDesc *device )
{
  if (!display_modes_enumerate()) return false;
  display_current_device = device;
  memset( &zbuf_pixelformat, 0, sizeof(zbuf_pixelformat) );
  zbuf_pixelformat.dwFlags = DDPF_ZBUFFER;
  HRESULT err = d3d_object->EnumZBufferFormats( *device->guid, enum_zbuffer_formats_callback, (void*)&zbuf_pixelformat );
  if (DDERR_NOZBUFFERHW==err) return true;
  if (D3D_OK!=err) return system_err( "d3d_object->EnumZBufferFormats() failed" );
  return true;
}

bool display_device_done()
{
  display_current_device = NULL;
  return display_mode_done();
}

////////////////////////////////////////////////////////////////////////////

static unsigned count_bits( unsigned x )
{
	unsigned n;
  for (n=0;x;x&=(x-1),n++) {}
  return n;
}

static HRESULT WINAPI enumerate_modes_callback( LPDDSURFACEDESC2 pdds, LPVOID lParam )
{
  if (pdds->ddpfPixelFormat.dwRGBBitCount<16) return DDENUMRET_OK;
  __modes[__nmodes].width = pdds->dwWidth;
  __modes[__nmodes].height = pdds->dwHeight;
  __modes[__nmodes].bit_depth = pdds->ddpfPixelFormat.dwRGBBitCount;
  __modes[__nmodes].r = count_bits( pdds->ddpfPixelFormat.dwRBitMask );
  __modes[__nmodes].g = count_bits( pdds->ddpfPixelFormat.dwGBitMask );
  __modes[__nmodes].b = count_bits( pdds->ddpfPixelFormat.dwBBitMask );
  if (++__nmodes<_DISPLAY_MAX_MODES)
    return DDENUMRET_OK;
  else
    return DDENUMRET_CANCEL;
}

static HRESULT WINAPI find_mode_callback( LPDDSURFACEDESC2 pdds, LPVOID lParam )
{
  DisplayModeDesc *dmd = (DisplayModeDesc *)lParam;
  unsigned rb = count_bits( pdds->ddpfPixelFormat.dwRBitMask );
  unsigned gb = count_bits( pdds->ddpfPixelFormat.dwGBitMask );
  unsigned bb = count_bits( pdds->ddpfPixelFormat.dwBBitMask );
  if (dmd->width!=pdds->dwWidth || dmd->height!=pdds->dwHeight || dmd->bit_depth!=pdds->ddpfPixelFormat.dwRGBBitCount ||
      (dmd->r && dmd->r!=rb ) || (dmd->g && dmd->g!=gb) || (dmd->b && dmd->b!=bb))
    return DDENUMRET_OK;
  else {
    dmd->r=rb, dmd->g=gb, dmd->b=bb;
    if (DD_OK!=dd_object->SetDisplayMode(dmd->width,dmd->height,dmd->bit_depth,0,0))
      system_err( "Failed to SetDisplayMode %dx%dx%d", dmd->width, dmd->height, dmd->bit_depth );
    else
      display_current_mode = dmd;
    return DDENUMRET_CANCEL;
  }
}

static bool display_modes_enumerate()
{
  __nmodes = 0;
  if (DD_OK!=dd_object->EnumDisplayModes(0,NULL,NULL,enumerate_modes_callback)) return system_err( "EnumDisplayModes failed" );
  if (!__nmodes) return system_err( "No display modes available" );
  return true;
}

bool display_modes_available( DisplayModeDesc **modes, int *nmodes )
{
  *nmodes = __nmodes;
  *modes = __modes;
  return true;
}

bool display_mode_init( unsigned nback_buffers, bool z_buffer, unsigned width, unsigned height, unsigned bit_depth, unsigned r, unsigned g, unsigned b )
{
  DisplayModeDesc mode = { width, height, bit_depth, r, g, b };
  return display_mode_init( nback_buffers, z_buffer, &mode );
}

bool display_mode_init( unsigned nback_buffers, bool z_buffer, const DisplayModeDesc *mode )
{
  if (mode->bit_depth<16) return system_err( "Can't set mode with bit depth < 16" );
  if (nback_buffers<1 || nback_buffers>2) return system_err( "Back buffers count could be either 1 or 2" );
  unsigned i;
  for (i=0;i<__nmodes;i++)
    if (__modes[i].width==mode->width && __modes[i].height==mode->height && __modes[i].bit_depth==mode->bit_depth &&
        (!mode->r || __modes[i].r==mode->r) && (!mode->g || __modes[i].g==mode->g) && (!mode->b || __modes[i].b==mode->b)) break;
  if (i==__nmodes) return system_err( "Requested display mode not available" );

  //Enumerate display modes to find requested one
  HRESULT err;
  char *on_error="";
  display_mode_done();
  display_current_mode = NULL;
  if (DD_OK!=dd_object->EnumDisplayModes(0,NULL,(LPVOID)&__modes[i],find_mode_callback)) FAIL( "EnumDisplayModes failed" );
  if (display_current_mode==NULL) FAIL( "Requested display mode not available" );

  //create screen surfaces
  DDSURFACEDESC2 ddsd;
  memset( &ddsd, 0, sizeof(ddsd) );
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_3DDEVICE|DDSCAPS_VIDEOMEMORY;
  ddsd.dwBackBufferCount = nback_buffers;
  err = dd_object->CreateSurface( &ddsd, &dd_buffer_front, NULL );
  while (DD_OK!=err && ddsd.dwBackBufferCount>1) {
     ddsd.dwBackBufferCount--;
     err = dd_object->CreateSurface( &ddsd, &dd_buffer_front, NULL );
  }
  if (DD_OK!=err) FAIL( "Can't create screen surfaces" );
  DDSCAPS2 caps;
  caps.dwCaps = DDSCAPS_BACKBUFFER;
  if (DD_OK!=dd_buffer_front->GetAttachedSurface(&caps,&dd_buffer_back)) FAIL( "Can't obtain the back buffer surface" );

  //create z-buffer if requested and available
  if (z_buffer && zbuf_pixelformat.dwSize==sizeof(zbuf_pixelformat)) {
    //Check if the device supports z-bufferless hidden surface removal. If so,
    //we don't really need a z-buffer
    DWORD dwRasterCaps = display_current_device->d3d_device_desc->dpcTriCaps.dwRasterCaps;
    if (!(dwRasterCaps&D3DPRASTERCAPS_ZBUFFERLESSHSR)) {
      DDSURFACEDESC2 ddsd;
      memset( &ddsd, 0, sizeof(DDSURFACEDESC2) );
      ddsd.dwSize         = sizeof(DDSURFACEDESC2);
      ddsd.dwFlags        = DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS|DDSD_PIXELFORMAT;
      ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER|DDSCAPS_VIDEOMEMORY;
      ddsd.dwWidth        = mode->width;
      ddsd.dwHeight       = mode->height;
      memcpy( &ddsd.ddpfPixelFormat, &zbuf_pixelformat, sizeof(DDPIXELFORMAT) );
      err = dd_object->CreateSurface( &ddsd, &dd_zbuffer, NULL );
      if (err== DDERR_OUTOFVIDEOMEMORY || err==DDERR_NOZBUFFERHW) {
        system_dbg( "Warning: z-buffer is in the system memory\n" );
        ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        err = dd_object->CreateSurface( &ddsd, &dd_zbuffer, NULL );
      }
      if (DD_OK!=err) {
        display_mode_done();
        return system_err( "Failed to create the z-buffer 0x%X %d", err, err&0xFFFF );
      }
      if (DD_OK!=dd_buffer_back->AddAttachedSurface(dd_zbuffer)) FAIL( "dd_buffer_back->AddAttachedSurface(dd_zbuffer) failed" );
    }
  }

  //create Direct3D device
  if (display_current_device->hardware) {
    if (D3D_OK!=d3d_object->CreateDevice(IID_IDirect3DHALDevice,dd_buffer_back,&d3d_device,NULL)) FAIL( "Unable to create Direct3D HAL device" );
  }
  else {
    if (D3D_OK!=d3d_object->CreateDevice(IID_IDirect3DRGBDevice,dd_buffer_back,&d3d_device,NULL)) FAIL( "Unable to create Direct3D RGB device" );
  }

  //Create and setup the viewport
  if (D3D_OK!=d3d_object->CreateViewport(&d3d_viewport,NULL)) FAIL( "d3d_object->CreateViewport() failed" );
  if (D3D_OK!=d3d_device->AddViewport(d3d_viewport)) FAIL( "d3d_device->AddViewport() failed" );

  D3DVIEWPORT2 view;
  memset( &view, 0, sizeof(D3DVIEWPORT2) );
  view.dwSize = sizeof(D3DVIEWPORT2);
  view.dwX = 0;
  view.dwY = 0;
  view.dwWidth  = mode->width;
  view.dwHeight = mode->height;
  view.dvClipX = -1.0f;
  view.dvClipWidth = 2.0f;
  view.dvClipHeight = (D3DVALUE)(mode->height * 2.0 / mode->width);
  view.dvClipY = view.dvClipHeight / 2.0f;
  view.dvMinZ = 0.0f;
  view.dvMaxZ = 1.0f;
  if (D3D_OK!=d3d_viewport->SetViewport2(&view)) FAIL( "d3d_viewport->SetViewport2() failed" );
  if (D3D_OK!=d3d_device->SetCurrentViewport(d3d_viewport)) FAIL( "SetCurrentViewport failed" );
  return true;

error:
  display_mode_done();
  return system_err( on_error );
}

bool display_mode_done()
{
  if (d3d_viewport) d3d_viewport->Release();
  if (d3d_device) d3d_device->Release();
  if (dd_zbuffer) dd_zbuffer->Release();
  if (dd_buffer_back) dd_buffer_back->Release();
  if (dd_buffer_front) dd_buffer_front->Release();
  d3d_viewport = NULL;
  d3d_device = NULL;
  dd_zbuffer = NULL;
  dd_buffer_back = NULL;
  dd_buffer_front = NULL;
  display_current_mode = NULL;
  return true;
}

////////////////////////////////////////////////////////////////////////////

static shutdown_handler *prev_shutdown_handler;
static void display_done()
{
  display_driver_done();
  prev_shutdown_handler();
}

static idle_handler *prev_idle_handler;
static void display_idle( long system_time )
{
  prev_idle_handler( system_time );
  if (dd_buffer_front && dd_buffer_front->IsLost()==DDERR_SURFACELOST) dd_buffer_front->Restore();
  if (dd_buffer_back && dd_buffer_back->IsLost()==DDERR_SURFACELOST) dd_buffer_back->Restore();
  if (dd_zbuffer && dd_zbuffer->IsLost()==DDERR_SURFACELOST) dd_zbuffer->Restore();
}

bool display_init()
{
  if (!display_drivers_enumerate()) return false;
  prev_shutdown_handler = register_shutdown_handler( display_done );
  prev_idle_handler = register_idle_handler( display_idle );
  d3d_viewport = NULL;
  d3d_device = NULL;
  dd_zbuffer = NULL;
  dd_buffer_back = NULL;
  dd_buffer_front = NULL;
  d3d_object = NULL;
  dd_object = NULL;
  display_current_device = NULL;
  return true;
}
