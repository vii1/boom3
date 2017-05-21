//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   texd3d.h - Direct 3D texture interface                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "display.h"

typedef void *hTexture;

enum tfENUM
{
  tfNA    =  0, //unsupported texture format
  tf1PAL  =  1, //8 bits paletized
  tf2PAL  =  2, //8 bits paletized
  tf4PAL  =  3, //8 bits paletized
  tf8PAL  =  4, //8 bits paletized
  tf555   =  5, //5r 5g 5b
  tf565   =  6, //5r 6g 5b
  tf888   =  7, //8r 8g 8b
  tf5551  =  8, //5r 5g 5b 1a
  tf4444  =  9, //4r 4g 4b 4a
  tf6666  = 10, //6r 6g 6b 6a
  tf8888  = 11, //8r 8g 8b 8a
  tfTOTAL = 12,
};

struct RGBA
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

// sets priorities for texture format conversion
bool texture_set_conversion_priorities( tfENUM format, tfENUM *priorities );
// loads a texture in Direct 3D
hTexture texture_load(tfENUM data_format, void *data, void *palette, unsigned width, unsigned height, unsigned nmipmaps );
// removes the texture from Direct 3D
bool texture_unload( hTexture texture );
// sets the current texture
bool texture_request( hTexture texture );
// gets Direct 3D texture from a texture handle
bool texture_get_d3d_texture( hTexture texture, IDirect3DTexture2 **d3d_texture );

#endif
