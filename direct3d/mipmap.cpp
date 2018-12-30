//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   mipmap.cpp - Mipmap generation implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _MIPMAP_CPP
#include "mipmap.h"

////////////////////////////////////////////////////////////////////////////

#include "system.h"
#include <d3d.h>

////////////////////////////////////////////////////////////////////////////

struct Map
{
  unsigned width, height, size;
  RGBA buffer[256*256];
};

////////////////////////////////////////////////////////////////////////////

static unsigned char pal[256*3];
static Map maps[2];
static Map *map=&maps[0];
static Map *mip=&maps[1];

////////////////////////////////////////////////////////////////////////////

// conversion functions for different pixel format
static bool _pal_8888( void *src, unsigned w, unsigned h, void *palette )
{
  memcpy( pal, palette, 3*256 );
  RGBA *d = map->buffer;
  unsigned char *s = (unsigned char *)src;
  for (unsigned size=w*h+1;--size;d++) {
    unsigned char *p = &pal[(*s++)*3];
    d->r = *p++;
    d->g = *p++;
    d->b = *p;
    d->a = 255;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _555_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned short *s = (unsigned short *)src;
  for (unsigned size=w*h+1;--size;d++) {
    unsigned short p = *s++;
    d->r = (p&0x1F)<<3;
    d->g = ((p>>5)&0x1F)<<3;
    d->b = ((p>>10)&0x1F)<<3;
    d->a = 255;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _565_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned short *s = (unsigned short *)src;
  for (unsigned size=w*h+1;--size;d++) {
    unsigned short p = *s++;
    d->r = (p&0x1F)<<3;
    d->g = ((p>>5)&0x3F)<<2;
    d->b = (p>>11)<<3;
    d->a = 255;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _888_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned char *s = (unsigned char *)src;
  for (unsigned size=w*h+1;--size;d++) {
    d->r = *s++;
    d->g = *s++;
    d->b = *s++;
    d->a = 255;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _5551_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned short *s = (unsigned short *)src;
  for (unsigned size=w*h+1;--size;d++) {
    unsigned short p = *s++;
    d->r = (p&0x1F)<<3;
    d->g = ((p>>5)&0x1F)<<3;
    d->b = ((p>>10)&0x1F)<<3;
    d->a = (p&0x8000)? 255 : 0;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _4444_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned short *s = (unsigned short *)src;
  for (unsigned size=w*h+1;--size;d++) {
    unsigned short p = *s++;
    d->r = (p&0xF)<<4;
    d->g = ((p>>4)&0xF)<<4;
    d->b = ((p>>8)&0xF)<<4;
    d->a = (p>>12)<<4;
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _6666_8888( void *src, unsigned w, unsigned h )
{
  RGBA *d = map->buffer;
  unsigned char *s = (unsigned char *)src;
  for (unsigned size=w*h+1;--size;s+=3,d++) {
    unsigned long p = *((unsigned long *)s) & 0xFFFFFF;
    d->r = (unsigned char)( (p&0x3F)<<2 );
    d->g = (unsigned char)( ((p>>6)&0x3F)<<2 );
    d->b = (unsigned char)( ((p>>12)&0x3F)<<2 );
    d->a = (unsigned char)( (p>>18)<<2 );
  }
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  return true;
}

static bool _8888_8888( void *src, unsigned w, unsigned h )
{
  map->width = w;
  map->height = h;
  map->size = w*h*sizeof(RGBA);
  memcpy( map->buffer, src, map->size );
  return true;
}

////////////////////////////////////////////////////////////////////////////

static bool swap_maps()
{
  if (mip==map)
    mip=&maps[1];
  else if (map==&maps[0]) {
    map=&maps[1];
    mip=&maps[0];
  }
  else {
    map=&maps[0];
    mip=&maps[1];
  }
  return true;
}

static unsigned nmipmaps( unsigned w, unsigned h )
{
  unsigned n;
  for (n=0;(w/=2) && (h/=2);n++) {}
  return n;
}

static bool map2mip_rgb()
{
  mip->width=map->width/2; mip->height=map->height/2;
  RGBA *d, *s1, *s2;
  unsigned row;
  for (row=mip->height+1,d=mip->buffer,s1=map->buffer,s2=map->buffer+map->width; --row; s1+=map->width,s2+=map->width)
    for (unsigned column=mip->width+1;--column;d++) {
      unsigned r=0, g=0, b=0;
      r+=s1->r, g+=s1->g, b+=s1->b; s1++;
      r+=s1->r, g+=s1->g, b+=s1->b; s1++;
      r+=s2->r, g+=s2->g, b+=s2->b; s2++;
      r+=s2->r, g+=s2->g, b+=s2->b; s2++;
      r/=4, g/=4, b/=4;
      d->r=r, d->g=g, d->b=b;
    }
  return true;
}

static bool map2mip_rgba()
{
  mip->width=map->width/2; mip->height=map->height/2;
  RGBA *d, *s1, *s2;
  unsigned row;
  for (row=mip->height+1,d=mip->buffer,s1=map->buffer,s2=map->buffer+map->width; --row; s1+=map->width,s2+=map->width)
    for (unsigned column=mip->width+1;--column;d++) {
      unsigned r=0, g=0, b=0, a=0;
      r+=s1->r, g+=s1->g, b+=s1->b, a+=s1->a; s1++;
      r+=s1->r, g+=s1->g, b+=s1->b, a+=s1->a; s1++;
      r+=s2->r, g+=s2->g, b+=s2->b, a+=s2->a; s2++;
      r+=s2->r, g+=s2->g, b+=s2->b, a+=s2->a; s2++;
      r/=4, g/=4, b/=4, a/=4;
      d->r=r, d->g=g, d->b=b, d->a=a;
    }
  return true;
}

////////////////////////////////////////////////////////////////////////////

static bool get_shifts( unsigned long ddmask, unsigned *shift1, unsigned *shift2 )
{
  unsigned i=1, n=0, n1=0, n2=0;
  for (;n<32 && !(ddmask&i);n++,i<<=1,n1++) {};
  for (;n<32 &&  (ddmask&i);n++,i<<=1,n2++) {};
  for (;n<32;n++,i<<=1)
    if (ddmask&i) return system_err( "Requested Direct3D texture surface format not supported" );
  *shift1 = 8-n2;
  *shift2 = n1;
  return true;
}

static bool get_shifts_rgb( DDPIXELFORMAT *ddpixelformat, unsigned *rshift1, unsigned *gshift1, unsigned *bshift1, unsigned *rshift2, unsigned *gshift2, unsigned *bshift2 )
{
  return get_shifts( ddpixelformat->dwRBitMask, rshift1, rshift2 ) &&
         get_shifts( ddpixelformat->dwGBitMask, gshift1, gshift2 ) &&
         get_shifts( ddpixelformat->dwBBitMask, bshift1, bshift2 );
}

static bool get_shifts_rgba( DDPIXELFORMAT *ddpixelformat, unsigned *rshift1, unsigned *gshift1, unsigned *bshift1, unsigned *ashift1, unsigned *rshift2, unsigned *gshift2, unsigned *bshift2, unsigned *ashift2 )
{
  return get_shifts_rgb( ddpixelformat, rshift1, gshift1, bshift1, rshift2, gshift2, bshift2 ) &&
         get_shifts( ddpixelformat->dwRGBAlphaBitMask, ashift1, ashift2 );
}

static inline unsigned char closest_color( unsigned char r, unsigned char g, unsigned char b, unsigned char *pal )
{
  unsigned char best;
  unsigned mind = 0xFFFFFFFF;
  unsigned i=257;
  while (--i) {
    unsigned rd = *pal++ - r;
    unsigned gd = *pal++ - g;
    unsigned bd = *pal++ - b;
    unsigned d = rd*rd + gd*gd + bd*bd;
    if (d<mind) {
      mind = d;
      best = 256-i;
    }
  }
  return best;
}

static bool _8888_pal( void *dst )
{
  unsigned char *d = (unsigned char *)dst;
  RGBA *s = mip->buffer;
  for (unsigned size=mip->width*mip->height+1;--size;s++)
    *d++ = closest_color( s->r, s->g, s->b, pal );
  return true;
}

static inline bool _8888_rgb( void *dst, unsigned bits_per_texel, unsigned rshift1, unsigned gshift1, unsigned bshift1, unsigned rshift2, unsigned gshift2, unsigned bshift2 )
{
  RGBA *s = mip->buffer;
  unsigned char *d = (unsigned char *)dst;
  unsigned bytes_per_texel = bits_per_texel/8;
  unsigned and_mask = ((1<<bits_per_texel)-1) ^ 0xFFFFFFFF;
  for (unsigned size=mip->width*mip->height+1;--size;s++,d+=bytes_per_texel) {
    unsigned *x = (unsigned *)d;
    unsigned or_mask = ((s->r>>rshift1)<<rshift2) | ((s->g>>gshift1)<<gshift2) | ((s->b>>bshift1)<<bshift2);
    *x &= and_mask;
    *x |= or_mask;
  }
  return true;
}

static inline bool _8888_rgba( void *dst, unsigned bits_per_texel, unsigned rshift1, unsigned gshift1, unsigned bshift1, unsigned ashift1, unsigned rshift2, unsigned gshift2, unsigned bshift2, unsigned ashift2 )
{
  RGBA *s = mip->buffer;
  unsigned char *d = (unsigned char *)dst;
  unsigned bytes_per_texel = bits_per_texel/8;
  unsigned and_mask = ((1<<bits_per_texel)-1) ^ 0xFFFFFFFF;
  for (unsigned size=mip->width*mip->height+1;--size;s++,d+=bytes_per_texel) {
    unsigned *x = (unsigned *)d;
    unsigned or_mask = ((s->r>>rshift1)<<rshift2) | ((s->g>>gshift1)<<gshift2) | ((s->b>>bshift1)<<bshift2) | ((s->a>>ashift1)<<ashift2);
    *x &= and_mask;
    *x |= or_mask;
  }
  return true;
}

static bool _8888_rgb( void *dst, DDPIXELFORMAT *ddpixelformat )
{
  unsigned rshift1, gshift1, bshift1, rshift2, gshift2, bshift2;
  if (!get_shifts_rgb(ddpixelformat,&rshift1,&gshift1,&bshift1,&rshift2,&gshift2,&bshift2)) return false;
  return _8888_rgb( dst, ddpixelformat->dwRGBBitCount,  rshift1, gshift1, bshift1, rshift2, gshift2, bshift2 );
}

static bool _8888_rgba( void *dst, DDPIXELFORMAT *ddpixelformat )
{
  unsigned rshift1, gshift1, bshift1, ashift1, rshift2, gshift2, bshift2, ashift2;
  if (!get_shifts_rgba(ddpixelformat,&rshift1,&gshift1,&bshift1,&ashift1,&rshift2,&gshift2,&bshift2,&ashift2)) return false;
  return _8888_rgba( dst, ddpixelformat->dwRGBBitCount, rshift1, gshift1, bshift1, ashift1, rshift2, gshift2, bshift2, ashift2 );
}

static bool _8888_tfENUM( void *dst, tfENUM format )
{
  switch (format) {
  case tf555:  return _8888_rgb( dst, 16, 8-5, 8-5, 8-5, 0, 5, 10 );
  case tf565:  return _8888_rgb( dst, 16, 8-5, 8-6, 8-5, 0, 5, 11 );
  case tf888:  return _8888_rgb( dst, 16, 8-8, 8-8, 8-8, 0, 8, 16 );
  case tf5551: return _8888_rgba( dst, 16, 8-5, 8-5, 8-5, 8-1, 0, 5, 10, 15 );
  case tf4444: return _8888_rgba( dst, 16, 8-4, 8-4, 8-4, 8-4, 0, 4, 8, 12 );
  case tf6666: return _8888_rgba( dst, 24, 8-6, 8-6, 8-6, 8-6, 0, 6, 12, 18 );
  case tf8888: return _8888_rgba( dst, 32, 8-8, 8-8, 8-8, 8-8, 0, 8, 16, 24 );
  default:     return system_err( "Requested tfENUM format not supported yet" );
  }
}

////////////////////////////////////////////////////////////////////////////

// sets the current texture
// returns the maximum number of mipmaps for that texture
unsigned mipmap_give_map( void *src, unsigned w, unsigned h, tfENUM format, void *pal )
{
  mip=map = &maps[0];
  switch (format) {
  case tf1PAL:
  case tf2PAL:
  case tf4PAL:
    system_err( "Requested tfENUM format not supported yet" );
    return 0;
  case tf8PAL:
    if (!_pal_8888(src,w,h,pal)) return 0;
    break;
  case tf555:
    if (!_555_8888(src,w,h)) return 0;
    break;
  case tf565:
    if (!_565_8888(src,w,h)) return 0;
    break;
  case tf888:
    if (!_888_8888(src,w,h)) return 0;
    break;
  case tf5551:
    if (!_5551_8888(src,w,h)) return 0;
    break;
  case tf4444:
    if (!_4444_8888(src,w,h)) return 0;
    break;
  case tf6666:
    if (!_6666_8888(src,w,h)) return 0;
    break;
  case tf8888:
    if (!_8888_8888(src,w,h)) return 0;
    break;
  }
  return nmipmaps( w, h );
}

// calculates the next mipmap
bool mipmap_get_mip( void *dst, void *ddpixelformat )
{
  DDPIXELFORMAT *ddpf = (DDPIXELFORMAT*)ddpixelformat;
  if (ddpf->dwFlags&DDPF_PALETTEINDEXED8) {
    if (mip!=map && !map2mip_rgb()) return false;
    if (!_8888_pal(dst)) return false;
  }
  else if((ddpf->dwFlags&DDPF_RGB) && !(ddpf->dwFlags&DDPF_ALPHAPIXELS)) {
    if (mip!=map && !map2mip_rgb()) return false;
    if (!_8888_rgb(dst,ddpf)) return false;
  }
  else if ((ddpf->dwFlags&DDPF_RGB) && (ddpf->dwFlags&DDPF_ALPHAPIXELS)) {
    if (mip!=map && !map2mip_rgba()) return false;
    if (!_8888_rgba(dst,ddpf)) return false;
  }
  else
    return system_err( "Requested Direct3D texture surface format not supported" );
  return swap_maps();
}

// calculates the next mipmap
bool mipmap_get_mip( void *dst, tfENUM format )
{
  switch (format) {
    case tf8PAL:
      if (mip!=map && !map2mip_rgb()) return false;
      if (!_8888_pal(dst)) return false;
      break;
    case tf555:
    case tf565:
    case tf888:
      if (mip!=map && !map2mip_rgb()) return false;
      if (!_8888_tfENUM(dst,format)) return false;
      break;
    case tf5551:
    case tf4444:
    case tf6666:
    case tf8888:
      if (mip!=map && !map2mip_rgba()) return false;
      if (!_8888_tfENUM(dst,format)) return false;
      break;
    default:
      return system_err( "Requested tfENUM format not supported yet" );
  }
  return swap_maps();
}
