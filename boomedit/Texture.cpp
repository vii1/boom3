//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   texture.cpp - texture editing implementation                       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TEXTURE_CPP
#include "texture.h"
#include "Ttexture.h"
#include "boom.h"

#include <string.h>
#include <malloc.h>

//adds a new texture 
bool newtexture(int w,int h,char *name,byte *data)
{
  if (ntextures>=MAX_TEXTURES) return false;
  Ttexture *t=textures+ntextures;
  t->wd=w;
  t->he=h;
  strcpy(t->name,name);
  t->datap=(byte *)malloc(w*h);
  if (!t->datap) return false;
  memcpy(t->datap,data,w*h);
  ntextures++;
  return true;
}

// changes the image in the texture
bool changetexture(int txt,int w,int h,char *name,byte *data)
{
  if (txt>=ntextures) return false;
  Ttexture *t=textures+txt;
  free(t->datap);
  t->wd=w;
  t->he=h;
  strcpy(t->name,name);
  t->datap=(byte *)malloc(w*h);
  if (!t->datap) return false;
  memcpy(t->datap,data,w*h);
  return true;
}

// checks if the texture is used by any walls
bool textureused(int txt)
{
  for (Tcluster *c=map->clusters;c;NEXTCLUSTER(c))
    for (Tsector *s=c->sectors;s;NEXTSECTOR(s)) {
      if (s->tfloor==txt || s->tceiling==txt) return true;
      for (Tline *l=s->lines;l;NEXTLINE(l)) {
        for (Twall **w=l->walls;*w;NEXTWALL(w))
          if ((*w)->texture==txt) return true;
      }
    }
  return false;
}

// creates a default texture
void defaulttexture( void )
{
  ntextures=1;
  Ttexture *t=textures;
  t->wd=64;
  t->he=64;
  strcpy(t->name,"default");
  t->datap=(byte *)malloc(64*64);
  memset(t->datap,0,64*64);
}

// deletes a texture
void deltexture(int txt)
{
  if (defwalltxt==txt) defwalltxt=0;
  else if (defwalltxt>txt) defwalltxt--;
  if (deffloortxt==txt) deffloortxt=0;
  else if (deffloortxt>txt) deffloortxt--;
  if (defceiltxt==txt) defceiltxt=0;
  else if (defceiltxt>txt) defceiltxt--;
  for (Tcluster *c=map->clusters;c;NEXTCLUSTER(c))
    for (Tsector *s=c->sectors;s;NEXTSECTOR(s)) {
      if (s->tfloor==txt) s->tfloor=deffloortxt;
      else if (s->tfloor>txt) s->tfloor--;
      if (s->tceiling==txt) s->tceiling=defceiltxt;
      else if (s->tceiling>txt) s->tceiling--;
      for (Tline *l=s->lines;l;NEXTLINE(l)) {
        for (Twall **w=l->walls;*w;NEXTWALL(w))
          if ((*w)->texture==txt) (*w)->texture=defwalltxt;
          else if ((*w)->texture>txt) (*w)->texture--;
      }
    }
  ntextures--;
  for (int i=txt;i<ntextures;i++)
    textures[i]=textures[i+1];
  if (!ntextures) defaulttexture();
}

