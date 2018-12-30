//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   texd3d.cpp - Direct 3D texture implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TEXD3D_CPP
#include "texd3d.h"
#include "system.h"
#include "mipmap.h"

////////////////////////////////////////////////////////////////////////////

static unsigned nbits( unsigned long x )
{
  unsigned i,n;
  unsigned long m;
  for (n=0,i=0,m=1;i<32;i++,m<<=1)
    if (x&m) n++;
  return n;
}

static bool texture_format_supported_flag;
static DDPIXELFORMAT texture_format_lpDDPixFmt;
static HRESULT WINAPI texture_formats_callback( LPDDPIXELFORMAT lpDDPixFmt, LPVOID lparam )
{
  unsigned rbits = nbits( lpDDPixFmt->dwRBitMask );
  unsigned gbits = nbits( lpDDPixFmt->dwGBitMask );
  unsigned bbits = nbits( lpDDPixFmt->dwBBitMask );
  unsigned abits = nbits( lpDDPixFmt->dwRGBAlphaBitMask );
  switch ((tfENUM)((unsigned long)lparam)) {
    case tf1PAL:
      if (!(lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED1)) return D3DENUMRET_OK;
      break;
    case tf2PAL:
      if (!(lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED2)) return D3DENUMRET_OK;
      break;
    case tf4PAL:
      if (!(lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED4)) return D3DENUMRET_OK;
      break;
    case tf8PAL:
      if (!(lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED8)) return D3DENUMRET_OK;
      break;
    case tf555:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || (lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=5 || gbits!=5 || bbits!=5) return D3DENUMRET_OK;
      break;
    case tf565:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || (lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=5 || gbits!=6 || bbits!=5) return D3DENUMRET_OK;
      break;
    case tf888:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || (lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=8 || gbits!=8 || bbits!=8) return D3DENUMRET_OK;
      break;
    case tf5551:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || !(lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=5 || gbits!=5 || bbits!=5 || abits!=1) return D3DENUMRET_OK;
      break;
    case tf4444:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || !(lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=4 || gbits!=4 || bbits!=4 || abits!=4) return D3DENUMRET_OK;
      break;
    case tf6666:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || !(lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=6 || gbits!=6 || bbits!=6 || abits!=6) return D3DENUMRET_OK;
      break;
    case tf8888:
      if (!(lpDDPixFmt->dwFlags&DDPF_RGB) || !(lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS) ||
        rbits!=8 || gbits!=8 || bbits!=8 || abits!=8) return D3DENUMRET_OK;
      break;
    default:
      system_err( "Unknown texture format" );
  }

  //requested format is supported
  texture_format_supported_flag = true;
  texture_format_lpDDPixFmt = *lpDDPixFmt;
  return D3DENUMRET_CANCEL;
}

static bool texture_format_supported( tfENUM format, LPDDPIXELFORMAT lpDDPixFmt )
{
  texture_format_supported_flag = false;
  d3d_device->EnumTextureFormats( texture_formats_callback, (LPVOID)format );
  *lpDDPixFmt = texture_format_lpDDPixFmt;
  return texture_format_supported_flag;
}

////////////////////////////////////////////////////////////////////////////

static tfENUM *get_priorities( tfENUM format )
{
  //Convertion priorities
  static tfENUM priorities_1pal[tfTOTAL] = { tf1PAL, tf2PAL, tf4PAL, tf8PAL, tf555,  tf565,  tf888,  tfNA };
  static tfENUM priorities_2pal[tfTOTAL] = { tf2PAL, tf4PAL, tf8PAL, tf555,  tf565,  tf888,  tfNA         };
  static tfENUM priorities_4pal[tfTOTAL] = { tf4PAL, tf8PAL, tf555,  tf565,  tf888,  tfNA                 };
  static tfENUM priorities_8pal[tfTOTAL] = { tf565,  tf8PAL, tf555,  tf888,  tfNA                         };
  static tfENUM priorities_555 [tfTOTAL] = { tf555,  tf5551, tf565,  tf888,  tf6666, tf8888, tf4444, tfNA };
  static tfENUM priorities_565 [tfTOTAL] = { tf565,  tf555,  tf5551, tf888,  tf6666, tf8888, tf4444, tfNA };
  static tfENUM priorities_888 [tfTOTAL] = { tf888,  tf565,  tf555,  tf5551, tf8888, tf6666, tf4444, tfNA };
  static tfENUM priorities_5551[tfTOTAL] = { tf5551, tf6666, tf8888, tf4444, tfNA                         };
  static tfENUM priorities_4444[tfTOTAL] = { tf4444, tf6666, tf8888, tfNA                                 };
  static tfENUM priorities_6666[tfTOTAL] = { tf6666, tf8888, tfNA                                         };
  static tfENUM priorities_8888[tfTOTAL] = { tf8888, tfNA                                                 };

  switch (format) {
    case tf1PAL: return priorities_1pal;
    case tf2PAL: return priorities_2pal;
    case tf4PAL: return priorities_4pal;
    case tf8PAL: return priorities_8pal;
    case tf555:  return priorities_555;
    case tf565:  return priorities_565;
    case tf888:  return priorities_888;
    case tf5551: return priorities_5551;
    case tf4444: return priorities_4444;
    case tf6666: return priorities_6666;
    case tf8888: return priorities_8888;
    default:     system_err( "Unknown texture format" ); return NULL;
  }
}

////////////////////////////////////////////////////////////////////////////

static bool create_surface( IDirect3DTexture2 **d3d_texture, IDirectDrawPalette **dd_pal, tfENUM data_format, unsigned width, unsigned height, void *palette, unsigned nmipmaps )
{
  char *on_error = "";
  IDirectDrawSurface4 *surface;
  tfENUM *p;
  if (!(p=get_priorities(data_format))) return false;
  for (;*p!=tfNA;p++) {
    DDPIXELFORMAT pixelformat;
    if (texture_format_supported(*p,&pixelformat)) {
      DDSURFACEDESC2 surfacedesc;
      memset( &surfacedesc, 0, sizeof(surfacedesc) );
      surfacedesc.dwSize = sizeof(surfacedesc);
      surfacedesc.dwFlags=DDSD_PIXELFORMAT|DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
      surfacedesc.ddpfPixelFormat=pixelformat;
      surfacedesc.dwWidth = width;
      surfacedesc.dwHeight = height;
      surfacedesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE,
      surfacedesc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
      if (nmipmaps>0) {
        surfacedesc.dwFlags |= DDSD_MIPMAPCOUNT, surfacedesc.dwMipMapCount = nmipmaps+1;
        surfacedesc.ddsCaps.dwCaps |= DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
      }

      D3DDEVICEDESC hal, hel;
      memset(&hal,0,sizeof(hal)); hal.dwSize = sizeof(hal);
      memset(&hel,0,sizeof(hel)); hel.dwSize = sizeof(hel);
      d3d_device->GetCaps( &hal, &hel );
      on_error = "Can't create Direct3D texture surface";

      //try to create the surface, if fail, try with no mipmaps
      HRESULT err = dd_object->CreateSurface( &surfacedesc, &surface, NULL );
      if (err!=DD_OK)
        if (err!=DDERR_NOMIPMAPHW)
          goto error;
        else {
          surfacedesc.dwFlags &= ~DDSD_MIPMAPCOUNT; 
          surfacedesc.ddsCaps.dwCaps &= ~(DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);
          if (DD_OK!=dd_object->CreateSurface(&surfacedesc,&surface,NULL)) goto error;
        }
        
      if (surfacedesc.ddpfPixelFormat.dwFlags&DDPF_PALETTEINDEXED8) {
        //initialize palette entries
        PALETTEENTRY pal[256];
        char *p=(char*)palette;
        for (unsigned i=0;i<256;i++) {
          pal[i].peRed = *p++;
          pal[i].peGreen = *p++;
          pal[i].peBlue = *p++;
          pal[i].peFlags = 0;
        } 

        //create a direct draw palette
        on_error = "Can't create Direct3D palette for texture surface";
        if (DD_OK!=dd_object->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256,pal,dd_pal,NULL))
          goto error_release_surface;

        //now, set the palette for each mipmap surface
        IDirectDrawSurface4 *lpDDLevel, *lpDDNextLevel;
        DDSCAPS2 ddsCaps;
        lpDDLevel = surface;
        lpDDLevel->AddRef();
        ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP; 
        HRESULT r=DD_OK; 
        while (r==DD_OK) { 
          //Set palette for current mipmap level
          on_error = "Can't set palette for texture surface";
          r = lpDDLevel->SetPalette( *dd_pal );
          if (DD_OK!=r) goto error_release_pal;

          //get next mipmap level
          r = lpDDLevel->GetAttachedSurface( &ddsCaps, &lpDDNextLevel );
          lpDDLevel->Release();
          lpDDLevel = lpDDNextLevel;
        } 
      }

      on_error ="DirectX 6 Required";
      if (DD_OK!=surface->QueryInterface(IID_IDirect3DTexture2,(void**)d3d_texture)) goto error_release_pal;
      surface->Release();

      return true;
    }
  }
  system_err( "Requested texture format not supported" );
  return false;

error_release_pal:
  (*dd_pal)->Release();
error_release_surface:
  surface->Release();
error:
  *dd_pal = NULL;
  *d3d_texture = NULL;
  system_err( on_error );
  return false;
}

static bool copy_data_to_surface( IDirect3DTexture2 *d3d_texture, void *data, tfENUM format, unsigned width, unsigned height, void *pal )
{
  HRESULT r;
  IDirectDrawSurface4 *surface;
  char *on_error = "DirectX 6 required";
  if (D3D_OK!=d3d_texture->QueryInterface(IID_IDirectDrawSurface4,(void**)&surface)) goto error1;
  IDirectDrawSurface4 *lpDDLevel, *lpDDNextLevel;
  DDSCAPS2 ddsCaps;
  lpDDLevel = surface;
  lpDDLevel->AddRef();
  ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP; 
  for (r=DD_OK;r==DD_OK;) { 
    //copy data to current mipmap level
    DDSURFACEDESC2 sd;
    memset( &sd, 0, sizeof(sd) );
    sd.dwSize = sizeof(sd);

    on_error = "Can't lock texture surface";
    if (DD_OK!=lpDDLevel->Lock(NULL,&sd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL)) goto error;

    mipmap_get_mip( sd.lpSurface, &sd.ddpfPixelFormat );

    on_error = "Can't unlock texture surface";
    if (DD_OK!=lpDDLevel->Unlock(NULL)) goto error;

    //get next mipmap level
    r = lpDDLevel->GetAttachedSurface( &ddsCaps, &lpDDNextLevel );
    lpDDLevel->Release();
    lpDDLevel = lpDDNextLevel;
  }
  surface->Release();
  return true;

error:
  lpDDLevel->Release();
  surface->Release();
error1:
  system_err( on_error );
  return false;
}

////////////////////////////////////////////////////////////////////////////

class Texture
{
public:
  Texture *next_vram_texture;

  Texture() {}

  bool create( tfENUM _data_format, void *_data, void *_palette, unsigned _width, unsigned _height, unsigned _nmipmaps );
  void destroy();
  bool request();
  bool get_d3d_texture( IDirect3DTexture2 **_d3d_texture );

private:
  unsigned width, height;
  IDirect3DTexture2 *d3d_texture;
	IDirectDrawPalette *palette;
	D3DTEXTUREHANDLE handle;
};

bool Texture::create( tfENUM _data_format, void *_data, void *_palette, unsigned _width, unsigned _height, unsigned _nmipmaps )
{
  width = _width;
  height = _height;
  next_vram_texture = NULL;

  //create surface and copy data to surface
  unsigned n=mipmap_give_map(_data,_width,_height,_data_format,_palette);
  if (_nmipmaps>n) _nmipmaps=n;  
  if (!create_surface(&d3d_texture,&palette,_data_format,_width,_height,_palette,_nmipmaps)) return false;
  if (!copy_data_to_surface(d3d_texture,_data,_data_format,width,height,_palette)) return false;

  return true;
}

void Texture::destroy()
{
  if (d3d_texture) d3d_texture->Release();
  if (palette) palette->Release();
  delete this;
}

bool Texture::request()
{
  static IDirect3DTexture2 *cur_texture=NULL;
  if (cur_texture!=d3d_texture && D3D_OK!=d3d_device->SetTexture(0,d3d_texture)) return system_err( "d3d_device->SetTexture() failed" );
  cur_texture = d3d_texture;
  return true;
}

bool Texture::get_d3d_texture( IDirect3DTexture2 **_d3d_texture )
{
  *_d3d_texture = d3d_texture;
  return true;
}


////////////////////////////////////////////////////////////////////////////

// sets priorities for texture format conversion
bool texture_set_conversion_priorities( tfENUM format, tfENUM *priorities )
{
  tfENUM *tf;
  if (!(tf=get_priorities(format))) return false;
  memcpy( tf, priorities, tfTOTAL*sizeof(tfENUM) );
  return true;
}

// loads a texture in Direct 3D
hTexture texture_load(tfENUM data_format, void *data, void *palette, unsigned width, unsigned height, unsigned nmipmaps )
{
  Texture *t = new Texture();
  if (t->create(data_format,data,palette,width,height,nmipmaps)) return t;
  return NULL;
}

// removes the texture from Direct 3D
bool texture_unload( hTexture texture )
{
  ((Texture *)texture)->destroy();
  return true;
}

// sets the current texture
bool texture_request( hTexture texture )
{
  return ((Texture *)texture)->request();
}

// gets Direct 3D texture from a texture handle
bool texture_get_d3d_texture( hTexture texture, IDirect3DTexture2 **d3d_texture )
{
  return ((Texture *)texture)->get_d3d_texture( d3d_texture );
}
