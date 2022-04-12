//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   draw2d.cpp - 2D map drawing implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _DRAW2D_CPP
#include "draw2d.h"
#include "boomedit.h"
#include "select.h"
#include "cluster.h"
#include "vertex.h"
#include "engine/geom.h"

#include <stdlib.h>

static HPEN linepen,gridpen,selvpen,boxpen,selspen,contpen,vertpen,sppen,cppen;
static HBRUSH brush1;
static HBITMAP hbmpxy,hbmpxz,hbmpyz;
static int bmpxyw,bmpxyh;
static int bmpxzw,bmpxzh;
static int bmpyzw,bmpyzh;

// draws a line in XY view
static void drawlinexy(HDC hdc,Tline *l)
{
  MoveTo(hdc,xymap2scrx(l->v1),xymap2scry(l->v1));
  LineTo(hdc,xymap2scrx(l->v2),xymap2scry(l->v2));
}

// draws a sector in XY view
static void drawsectorxy(HDC hdc,Tsector *s)
{
  Tline *l;
  for (l=s->lines;l;l=l->next)
    drawlinexy(hdc,l);
}

// draws the XY view
void drawxy(HDC hdc)
{
  HDC hsrc=CreateCompatibleDC(hdc);
  int w=viewxy.right-viewxy.left;
  int h=viewxy.bottom-viewxy.top;
  RECT r1={0,0,w,h};
  if (w!=bmpxyw || h!=bmpxyh) DeleteObject(hbmpxy),hbmpxy=NULL;
  if (!hbmpxy) {
    hbmpxy=CreateCompatibleBitmap(hdc,w,h);
    bmpxyw=w;
    bmpxyh=h;
  }
  SelectObject(hsrc,hbmpxy);
  FillRect(hsrc,&r1,brush1);
  // draws the grid
  if (gridfl) {
    SelectObject(hsrc,gridpen);
    float dd=scale*gridsize;
    if (dd>=3) {
      float x1=xymap2scrx((coord3d)((int)(viewxyx0/gridsize)*gridsize));
      float y1=xymap2scry((coord3d)((int)(viewxyy0/gridsize)*gridsize));
      for (;y1>=0;y1-=dd) {
        MoveTo(hsrc,0,y1);
        LineTo(hsrc,w,y1);
      }
      for (;x1<w;x1+=dd) {
        MoveTo(hsrc,x1,0);
        LineTo(hsrc,x1,h);
      }
    }
  }
  if (map) {
    Tcluster *c;
    // draws the sectors
    SelectObject(hsrc,linepen);
    for (c=map->clusters;c;c=c->next)
      if (c->visible) {
        Tsector *s;
        for (s=c->sectors;s;s=s->next)
          if (s!=cursector) drawsectorxy(hsrc,s);
      }
    // draws the vertices
    SelectObject(hsrc,vertpen);
    for (int i=0;i<vertsnum;i++)
      if (vertvisible(i)) {
        int x=xymap2scrx(verts[i].x);
        int y=xymap2scry(verts[i].y);
        MoveTo(hsrc,x,y);
        LineTo(hsrc,x+1,y);
      }
  }
  // draws the selected vertices
  SelectObject(hsrc,selvpen);
  drawselectedverts(hsrc);
  // draws the selected sector
  if (cursector) {
    SelectObject(hsrc,selspen);
    drawsectorxy(hsrc,cursector);
  }
  // draws the selected line
  if (curline) {
    SelectObject(hsrc,selspen);
    drawlinexy(hsrc,curline);
  }
  // draws the selection box
  SelectObject(hsrc,boxpen);
  SelectObject(hsrc,GetStockObject(NULL_BRUSH));
  drawselbox(hsrc);
  // draws the currently edited contour
  SelectObject(hsrc,contpen);
  drawcontour(hsrc);
  // draws the start position
  if (startpfl) {
    SelectObject(hsrc,sppen);
    int x=xymap2scrx(map->start_x);
    int y=xymap2scry(map->start_y);
    MoveTo(hsrc,x-3,y-3);
    LineTo(hsrc,x+3,y+3);
    MoveTo(hsrc,x-3,y+3);
    LineTo(hsrc,x+3,y-3);
  }
  // draws the current position
  if (currentpfl) {
    SelectObject(hsrc,cppen);
    int x=xymap2scrx(view.x);
    int y=xymap2scry(view.y);
    MoveTo(hsrc,x-3,y-3);
    LineTo(hsrc,x+3,y+3);
    MoveTo(hsrc,x-3,y+3);
    LineTo(hsrc,x+3,y-3);
  }
  BitBlt(hdc,viewxy.left,viewxy.top,w,h,hsrc,0,0,SRCCOPY);
  DeleteDC(hsrc);
}

// draws a line in XZ view
static void drawlinexz(HDC hdc,Tline *l,Tsector *s)
{
  int x1=xzmap2scrx(l->v1);
  int x2=xzmap2scrx(l->v2);
  if ((x1<0 && x2<0) || (x1>vieww2 && x2>vieww2)) return;
  coord3d f1=s->getzf(verts[l->v1].x,verts[l->v1].y);
  coord3d f2=s->getzf(verts[l->v2].x,verts[l->v2].y);
  coord3d c1=s->getzc(verts[l->v1].x,verts[l->v1].y);
  coord3d c2=s->getzc(verts[l->v2].x,verts[l->v2].y);
  int z1=xzmap2scrz(f1);
  int z2=xzmap2scrz(f2);
  int zc1=xzmap2scrz(c1);
  int zc2=xzmap2scrz(c2);
  // draws the floor and ceiling
  MoveTo(hdc,x1,z1);
  LineTo(hdc,x2,z2);
  MoveTo(hdc,x1,zc1);
  LineTo(hdc,x2,zc2);

  Twall *w;
  int z3=z1,z4=z2;
  // draws the walls
  for (w=*l->walls;w;w=w->next) {
    if (is_hole(w)) {
      if (z1!=z3) {
        MoveTo(hdc,x1,z1);
        LineTo(hdc,x1,z3);
      }
      if (z2!=z4) {
        MoveTo(hdc,x2,z2);
        LineTo(hdc,x2,z4);
      }
    }
    z3=xzmap2scrz(w->z1c);
    z4=xzmap2scrz(w->z2c);
    MoveTo(hdc,x1,z3);
    LineTo(hdc,x2,z4);
    if (is_hole(w)) {
      z1=z3;
      z2=z4;
    }
  }
  if (z1!=zc1) {
    MoveTo(hdc,x1,z1);
    LineTo(hdc,x1,zc1);
  }
  if (z2!=zc2) {
    MoveTo(hdc,x2,z2);
    LineTo(hdc,x2,zc2);
  }
}

// draws a sector in XZ view
static void drawsectorxz(HDC hdc,Tsector *s)
{
  Tline *l;
  for (l=s->lines;l;l=l->next)
    drawlinexz(hdc,l,s);
}

// draws the XZ view
void drawxz(HDC hdc)
{
  HDC hsrc=CreateCompatibleDC(hdc);
  int w=viewxz.right-viewxz.left;
  int h=viewxz.bottom-viewxz.top;
  RECT r1={0,0,w,h};
  if (w!=bmpxzw || h!=bmpxzh) DeleteObject(hbmpxz),hbmpxz=NULL;
  if (!hbmpxz) {
    hbmpxz=CreateCompatibleBitmap(hdc,w,h);
    bmpxzw=w;
    bmpxzh=h;
  }
  SelectObject(hsrc,hbmpxz);
  FillRect(hsrc,&r1,brush1);
  // draws the grid
  if (gridfl) {
    SelectObject(hsrc,gridpen);
    float dd=scale*gridsize;
    if (dd>=3) {
      float x1=xzmap2scrx((coord3d)((int)(viewxzx0/gridsize)*gridsize));
      float y1=xzmap2scrz((coord3d)((int)(viewxzz0/gridsize)*gridsize));
      for (;y1>=0;y1-=dd) {
        MoveTo(hsrc,0,y1);
        LineTo(hsrc,w,y1);
      }
      for (;x1<w;x1+=dd) {
        MoveTo(hsrc,x1,0);
        LineTo(hsrc,x1,h);
      }
    }
  }
  // draws the sectors
  SelectObject(hsrc,linepen);
  if (map) {
    Tcluster *c;
    for (c=map->clusters;c;c=c->next)
      if (c->visible) {
        Tsector *s;
        for (s=c->sectors;s;s=s->next)
          if (s!=cursector) drawsectorxz(hsrc,s);
      }
  }
  // draws the current sector
  if (cursector) {
    SelectObject(hsrc,selspen);
    drawsectorxz(hsrc,cursector);
  }
  // draws the current line
  if (curline) {
    SelectObject(hsrc,selspen);
    drawlinexz(hsrc,curline,curlinesec);
  }
  BitBlt(hdc,viewxz.left,viewxz.top,w,h,hsrc,0,0,SRCCOPY);
  DeleteDC(hsrc);
}

// draws a line in YZ view
static void drawlineyz(HDC hdc,Tline *l,Tsector *s)
{
  int x1=yzmap2scry(l->v1);
  int x2=yzmap2scry(l->v2);
  if ((x1<0 && x2<0) || (x1>vieww3 && x2>vieww3)) return;
  coord3d f1=s->getzf(verts[l->v1].x,verts[l->v1].y);
  coord3d f2=s->getzf(verts[l->v2].x,verts[l->v2].y);
  coord3d c1=s->getzc(verts[l->v1].x,verts[l->v1].y);
  coord3d c2=s->getzc(verts[l->v2].x,verts[l->v2].y);
  int z1=yzmap2scrz(f1);
  int z2=yzmap2scrz(f2);
  int zc1=yzmap2scrz(c1);
  int zc2=yzmap2scrz(c2);
  // draws the floor and ceiling
  MoveTo(hdc,x1,z1);
  LineTo(hdc,x2,z2);
  MoveTo(hdc,x1,zc1);
  LineTo(hdc,x2,zc2);

  Twall *w;
  int z3=z1,z4=z2;
  // draws the walls
  for (w=*l->walls;w;w=w->next) {
    if (is_hole(w)) {
      if (z1!=z3) {
        MoveTo(hdc,x1,z1);
        LineTo(hdc,x1,z3);
      }
      if (z2!=z4) {
        MoveTo(hdc,x2,z2);
        LineTo(hdc,x2,z4);
      }
    }
    z3=yzmap2scrz(w->z1c);
    z4=yzmap2scrz(w->z2c);
    MoveTo(hdc,x1,z3);
    LineTo(hdc,x2,z4);
    if (is_hole(w)) {
      z1=z3;
      z2=z4;
    }
  }
  if (z1!=zc1) {
    MoveTo(hdc,x1,z1);
    LineTo(hdc,x1,zc1);
  }
  if (z2!=zc2) {
    MoveTo(hdc,x2,z2);
    LineTo(hdc,x2,zc2);
  }
}

// draws a sector in YZ view
static void drawsectoryz(HDC hdc,Tsector *s)
{
  Tline *l;
  for (l=s->lines;l;l=l->next)
    drawlineyz(hdc,l,s);
}

// draws the YZ view
void drawyz(HDC hdc)
{
  HDC hsrc=CreateCompatibleDC(hdc);
  int w=viewyz.right-viewyz.left;
  int h=viewyz.bottom-viewyz.top;
  RECT r1={0,0,w,h};
  if (w!=bmpyzw || h!=bmpyzh) DeleteObject(hbmpyz),hbmpyz=NULL;
  if (!hbmpyz) {
    hbmpyz=CreateCompatibleBitmap(hdc,w,h);
    bmpyzw=w;
    bmpyzh=h;
  }
  // draws the grid
  SelectObject(hsrc,hbmpyz);
  FillRect(hsrc,&r1,brush1);
  if (gridfl) {
    SelectObject(hsrc,gridpen);
    float dd=scale*gridsize;
    if (dd>=3) {
      float x1=yzmap2scry((coord3d)((int)(viewyzy0/gridsize)*gridsize));
      float y1=yzmap2scrz((coord3d)((int)(viewyzz0/gridsize)*gridsize));
      for (;y1>=0;y1-=dd) {
        MoveTo(hsrc,0,y1);
        LineTo(hsrc,w,y1);
      }
      for (;x1<w;x1+=dd) {
        MoveTo(hsrc,x1,0);
        LineTo(hsrc,x1,h);
      }
    }
  }
  // draws the sectors
  SelectObject(hsrc,linepen);
  if (map) {
    Tcluster *c;
    for (c=map->clusters;c;c=c->next)
      if (c->visible) {
        Tsector *s;
        for (s=c->sectors;s;s=s->next)
          if (s!=cursector) drawsectoryz(hsrc,s);
      }
  }
  // draws the current sector
  if (cursector) {
    SelectObject(hsrc,selspen);
    drawsectoryz(hsrc,cursector);
  }
  // draws the current line
  if (curline) {
    SelectObject(hsrc,selspen);
    drawlineyz(hsrc,curline,curlinesec);
  }
  BitBlt(hdc,viewyz.left,viewyz.top,w,h,hsrc,0,0,SRCCOPY);
  DeleteDC(hsrc);
}

// draws the frame
void drawframe(HDC hdc)
{
  SelectObject(hdc,linepen);
  SelectObject(hdc,GetStockObject(NULL_BRUSH));
  if (viewxy.bottom) Rectangle(hdc,viewxy.left-1,viewxy.top-1,viewxy.right+1,viewxy.bottom+1);
  if (viewxz.bottom) Rectangle(hdc,viewxz.left-1,viewxz.top-1,viewxz.right+1,viewxz.bottom+1);
  if (viewyz.bottom) Rectangle(hdc,viewyz.left-1,viewyz.top-1,viewyz.right+1,viewyz.bottom+1);
  if (view3d.bottom) Rectangle(hdc,view3d.left-1,view3d.top-1,view3d.right+1,view3d.bottom+1);
}

// frees the allocated resources
static void draw2d_done( void )
{
  if (linepen) DeleteObject(linepen),linepen=NULL;
  if (gridpen) DeleteObject(gridpen),gridpen=NULL;
  if (selvpen) DeleteObject(selvpen),selvpen=NULL;
  if (boxpen) DeleteObject(boxpen),boxpen=NULL;
  if (selspen) DeleteObject(selspen),selspen=NULL;
  if (contpen) DeleteObject(contpen),contpen=NULL;
  if (vertpen) DeleteObject(vertpen),vertpen=NULL;
  if (sppen) DeleteObject(sppen),sppen=NULL;
  if (cppen) DeleteObject(cppen),cppen=NULL;
  if (hbmpxy) DeleteObject(hbmpxy),hbmpxy=NULL;
  if (hbmpxz) DeleteObject(hbmpxz),hbmpxz=NULL;
  if (hbmpyz) DeleteObject(hbmpyz),hbmpyz=NULL;
}

// initialization
void draw2d_init( void )
{
  linepen=CreatePen(PS_SOLID,0,RGB(0,0,0));
  gridpen=CreatePen(PS_SOLID,0,RGB(192,192,192));
  selvpen=CreatePen(PS_SOLID,4,RGB(255,0,0));
  boxpen=CreatePen(PS_DOT,0,RGB(128,128,128));
  selspen=CreatePen(PS_SOLID,0,RGB(255,0,0));
  contpen=CreatePen(PS_SOLID,0,RGB(0,0,255));
  vertpen=CreatePen(PS_SOLID,3,RGB(0,0,0));
  sppen=CreatePen(PS_SOLID,2,RGB(255,0,0));
  cppen=CreatePen(PS_SOLID,2,RGB(0,0,255));
  brush1=(HBRUSH)GetStockObject(WHITE_BRUSH);
  atexit(draw2d_done);
}
