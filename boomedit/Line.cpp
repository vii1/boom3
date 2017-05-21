//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   line.cpp - line editing implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _LINE_CPP
#include "line.h"
#include "sector.h"
#include "vertex.h"
#include "boomedit.h"
#include "select.h"

// deletes the line l and all it's walls
void delline(Tline *l)
{
  delvertex(l->v1);
  delvertex(l->v2);
  l->unload();
  delete l;
}

// deletes the line l and updates the map
void delline(Tmap *m,Tcluster *c,Tsector *s,Tline *l)
{
  Tline *l1,*l2;
  int ln;
  for (ln=1,l1=l->nextc;l1!=l;ln++,l1=l1->nextc);
  // if the lines in the contour <=3, delete the whole contour
  if (ln<=3) {
    s->linesnum-=ln;
    if (s->linesnum>=3) {
      for (l2=l;l2->next==l2->nextc;l2=l2->nextc);
      for (Tline *l1=s->lines;l1;l1=l1->next)
        if (l1->next==l2->nextc) l1->next=l2->next;
      if (s->lines==l2->nextc) s->lines=l2->next;
      for (;ln>0;ln--) {
        l2=l->nextc;
        delline(l);
        l=l2;
      }
    }
    // if this is the last contour, delete the whole sector
    else delsector(m,c,s);
  }
  else {
    s->linesnum--;
    for (Tline *l1=s->lines;l1;l1=l1->next) {
      if (l1->next==l) l1->next=l->next;
      if (l1->nextc==l) l1->nextc=l->nextc;
    }
    if (s->lines==l) s->lines=l->next;
    delline(l);
  }
}

#define DELTA 5

// checks if the point (x,y) is near the line l
static bool nearvert(Tline *l,coord3d x,coord3d y)
{
  double delta=DELTA/scale;
  coord3d x1=verts[l->v1].x;
  coord3d y1=verts[l->v1].y;
  coord3d x2=verts[l->v2].x;
  coord3d y2=verts[l->v2].y;
  coord3d dx=x2-x1;
  coord3d dy=y2-y1;
  double d=x*dy-y*dx-x1*y2+x2*y1;
  if (d*d>delta*delta*(dx*dx+dy*dy)) return false;
  if (x1>x2) {
    coord3d q=x1;x1=x2;x2=q;
  }
  if (y1>y2) {
    coord3d q=y1;y1=y2;y2=q;
  }
  return (x>x1-delta && x<x2+delta && y>y1-delta && y<y2+delta);
}

// divides the line l with the vertex v
void splitline(Tline *l,int v)
{
  Tline *ln=new Tline();
  ln->next=l->next;
  l->next=ln;
  ln->nextc=l->nextc;
  l->nextc=ln;
  ln->wallsnum=l->wallsnum;
  ln->v2=l->v2;
  l->v2=ln->v1=v;
  ln->len=l->len;
  ln->walls=(Twall **)malloc(sizeof(Twall *));
  Twall *w;
  Twall **w1=ln->walls;
  for (w=*l->walls;w;w=w->next) {
    if (is_portal(w)) {
      *w1=new Tportal();
      memcpy(*w1,w,sizeof(Tportal));
    }
    else if (is_hole(w)) {
      *w1=new Thole();
      memcpy(*w1,w,sizeof(Thole));
    }
    else {
      *w1=new Twall();
      memcpy(*w1,w,sizeof(Twall));
    }
    w1=&(*w1)->next;
  }
  *w1=NULL;
}

// finds a line near the point (x,y)
// first checks the lines in the sector containing (x,y)
bool findline(Tline **lp,Tsector **sp,coord3d x,coord3d y)
{
  Tcluster *c;
  Tsector *s=findsectorf(x,y);
  Tline *l;
  if (s) {
    for (l=s->lines;l;l=l->next)
      if (nearvert(l,x,y)) {
        *lp=l;
        *sp=s;
        return true;
      }
  }
  for (c=map->clusters;c;c=c->next)
    for (s=c->sectors;s;s=s->next)
      for (l=s->lines;l;l=l->next)
        if (nearvert(l,x,y)) {
          *lp=l;
          *sp=s;
          return true;
        }
  return false;
}

