//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   readbmp.cpp - Bitmap reading implementaton                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _READBMP_CPP
#include "readbmp.h"
#include "types.h"

#include <windows.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////

static bool is_power_of_2(int x)
{
  return (x&(x-1))==0;
}

#define MAX_ASPECT_RATIO 8

static bool check_aspect_ratio( int width, int height )
{
  return width<=MAX_ASPECT_RATIO*height && height<=MAX_ASPECT_RATIO*width;
}

static void flip( void *data, int width, int height)
{
  width/= 4;
  dword *p1 = (dword *)data;
  dword *p2 = p1 + (height-1)*width;
  height /= 2;
  for (int r=0;r<height;r++,p2-=2*width)
    for (int c=0;c<width;c++,p1++,p2++) {
      dword x=*p1; *p1=*p2; *p2=x;
    }
}

// reads bitmap from a file
// returns the palette, bits and size
bool read_bmp(char *name, byte **data, byte **palette, int *width, int *height )
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  int size;
  *data=NULL;
  *palette=NULL;

  FILE *f=fopen(name,"rb");
  if (!f) return false;
  if (!fread(&bmfh,sizeof(bmfh),1,f)) goto err;
  if (bmfh.bfType!='MB') goto err;

  if (!fread(&bmih,sizeof(bmih),1,f)) goto err;
  if (!is_power_of_2(bmih.biWidth)) goto err;
  if (!is_power_of_2(bmih.biHeight)) goto err;
  if (!check_aspect_ratio(bmih.biWidth,bmih.biHeight)) goto err;
  if (bmih.biBitCount!=8) goto err;

  *palette=(byte *)malloc(256*4);
  if (!*palette) goto err;
  if (!fread(*palette,256*4,1,f)) goto err;

  size=bmih.biWidth*bmih.biHeight;
  *data=(byte *)malloc(size);
  if (!*data) goto err;
  if (!fread(*data,size,1,f)) goto err;

  fclose(f);
  flip(*data,bmih.biWidth,bmih.biHeight);
  *width=bmih.biWidth;
  *height=bmih.biHeight;
  return true;

 err:
  fclose(f);
  if (*data) free(*data);
  if (*palette) free(*palette);
  return false;
}

// reads palette from a file
bool read_pal(char *name, byte **palette)
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  *palette=NULL;

  FILE *f=fopen(name,"rb");
  if (!f) return false;
  if (!fread(&bmfh,sizeof(bmfh),1,f)) goto err;
  if (bmfh.bfType!='MB') goto err;

  if (!fread(&bmih,sizeof(bmih),1,f)) goto err;
  if (bmih.biBitCount!=8) goto err;

  *palette=(byte *)malloc(256*4);
  if (!*palette) goto err;
  if (!fread(*palette,256*4,1,f)) goto err;
  fclose(f);
  return true;

 err:
  fclose(f);
  if (*palette) free(*palette);
  return false;
}
