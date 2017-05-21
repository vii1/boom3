//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   select.cpp - selection routines implementation                     //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _SELECT_CPP
#include "select.h"
#include "boomedit.h"
#include "draw2d.h"
#include "vertex.h"
#include "sector.h"
#include "contour.h"
#include "line.h"

#include <math.h>

struct Tvertlist {
  Tvertex *v;
  coord3d x,y;
  Tvertlist *next;
};

static Tvertlist *selv;
static double delta;
static coord3d curzfc,curzcc;

#define DELTA 5

//// vertex selection
// deselects all vertices
void deselectvertices( void )
{
  Tvertlist *v,*n;
  for (v=selv;v;v=n) {
    n=v->next;
    free(v);
  }
  selv=NULL;
}

// marks vertex as selected
void selectvertex(Tvertex *v)
{
  if (vertvisible(v-verts)) {
    Tvertlist *q=(Tvertlist *)malloc(sizeof(Tvertlist));
    q->next=selv;
    q->v=v;
    q->x=v->x;
    q->y=v->y;
    selv=q;
  }
}

// removes the vertex from the selection list
static void deselectvertex(Tvertex *vr)
{
  for (Tvertlist **v=&selv;*v;v=&((*v)->next))
    if ((*v)->v==vr) {
      Tvertlist *q=(*v)->next;
      free(*v);
      *v=q;
      break;
    }
}

// deselects all hidden vertices
void deselecthidden( void )
{
 l:
  for (Tvertlist *v=selv;v;v=v->next)
    if (!vertvisible(v->v-verts)) {
      deselectvertex(v->v);
      goto l;
    }
}

// checks if two points are close
static bool nearv(coord3d x1,coord3d y1,coord3d x2,coord3d y2)
{
  return ((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)<delta);
}

// checks if the vertex is currently selected
bool vertexselected(int sx,int sy)
{
  delta=(DELTA/scale)*(DELTA/scale);
  coord3d x=xyscr2mapx(sx);
  coord3d y=xyscr2mapy(sy);
  for (Tvertlist *v=selv;v;v=v->next) {
    if (nearv(x,y,v->v->x,v->v->y)) return true;
  }
  return false;
}

// checks if the vertex is currently selected
static bool vertexselected(Tvertex *vr)
{
  for (Tvertlist *v=selv;v;v=v->next) {
    if (v->v==vr) return true;
  }
  return false;
}

// selects a vertex near given position
bool selectvertex(int sx,int sy)
{
  delta=(DELTA/scale)*(DELTA/scale);
  coord3d x=xyscr2mapx(sx);
  coord3d y=xyscr2mapy(sy);
  for (int i=0;i<vertsnum;i++) {
    if (nearv(x,y,verts[i].x,verts[i].y)) {
      if (!vertexselected(verts+i)) selectvertex(verts+i);
      else deselectvertex(verts+i);
      cursector=NULL;
      curline=NULL;
      return true;
    }
  }
  return false;
}

// selects all vertices in the rectangle
void selectvertexbox(RECT *r)
{
  coord3d x1=xyscr2mapx(r->left+viewxy.left);
  coord3d y1=xyscr2mapy(r->top+viewxy.top);
  coord3d x2=xyscr2mapx(r->right+viewxy.left);
  coord3d y2=xyscr2mapy(r->bottom+viewxy.top);
  if (x1>x2) {
    coord3d q=x2;
    x2=x1;
    x1=q;
  }
  if (y1>y2) {
    coord3d q=y2;
    y2=y1;
    y1=q;
  }
  for (int i=0;i<vertsnum;i++) {
    if (verts[i].x>x1 && verts[i].x<x2 &&verts[i].y>y1 && verts[i].y<y2) {
      if (!vertexselected(verts+i)) selectvertex(verts+i);
    }
  }
  cursector=NULL;
  curline=NULL;
}

// changes the position of the selected vertices with a vector
void dragvertex(coord3d dx,coord3d dy)
{
  if (gridfl && selv && !selv->next) {
    selv->v->x=snaptogrid(selv->x+dx);
    selv->v->y=snaptogrid(selv->y+dy);
  }
  else for (Tvertlist *v=selv;v;v=v->next) {
    v->v->x=v->x+dx;
    v->v->y=v->y+dy;
  }
}

// if a sector has a common line with s0, creates a hole/portal between them
static void makehole(Tcluster *c0,Tsector *s0)
{
  Tline *l0;
  bool flmerge0=false;
  for (l0=s0->lines;l0;l0=l0->next) {
    Tcluster *c;
    coord3d x1=verts[l0->v1].x;
    coord3d y1=verts[l0->v1].y;
    coord3d x2=verts[l0->v2].x;
    coord3d y2=verts[l0->v2].y;
    coord3d z1f=s0->getzf(x1,y1);
    coord3d z1c=s0->getzc(x1,y1);
    coord3d z2f=s0->getzf(x2,y2);
    coord3d z2c=s0->getzc(x2,y2);
    for (c=map->clusters;c;c=c->next) {
      Tsector *s;
      for (s=c->sectors;s;s=s->next) {
        if (s==s0) continue;
        {
          Twall **w;
          for (w=l0->walls;*w;w=&(*w)->next)
            if (is_hole(*w)) {
              if (((Thole *)(*w))->sector==s) break; // if they are already connected with a hole
            }
          if (*w) continue;
        }
        bool fl=false;
        bool flmerge=false;
        if (s->getzf(x1,y1)<=z1c && s->getzf(x2,y2)<=z2c &&
            s->getzc(x1,y1)>=z1f && s->getzc(x2,y2)>=z2f) {
          Tline *l;
          for (l=s->lines;l;l=l->next) {
            if (l->v1==l0->v2 && l->v2==l0->v1) {
              // if the sectors have common line on the outside, creates a hole/portal
              Thole *h0,*h1;
              if (c0==c) {
                h0=new Thole();
                h1=new Thole();
                h0->options=0;
                h1->options=0;
              }
              else {
                h0=new Tportal();
                h1=new Tportal();
                h0->options=waPORTAL;
                h1->options=waPORTAL;
                ((Tportal *)h0)->target=c;
                ((Tportal *)h1)->target=c0;
              }
              h0->texture=-1;
              h0->sector=s;
              h0->next=*l0->walls;
              *l0->walls=h0;
              l0->wallsnum++;
              h1->texture=-1;
              h1->sector=s0;

              Twall **ww;
              for (ww=l->walls;*ww;ww=&(*ww)->next)
                if ((*ww)->z1c>=z1c && (*ww)->z2c>=z2c) break;
              h1->next=*ww;
              *ww=h1;
              l->wallsnum++;
              fl=true;
            }
            else if (l->v1==l0->v1 && l->v2==l0->v2) {
              // if the sectors have common line on the inside, copies the line from the outer sector into the inner one
              Tline *l1=l,*l2=l0;
              if (contourarea(l1)<contourarea(l2)) l1=l0,l2=l;
              if (l2->walls) {
                Twall *w,*n;
                for (w=*l2->walls;w;w=n) {
                  n=w->next;
                  delete w;
                }
              }
              // copies all the walls
              *l2->walls=NULL;
              l2->wallsnum=l1->wallsnum;
              if (l1->walls) {
                Twall *w1,**w2;
                w2=l2->walls;
                for (w1=*l1->walls;w1;w1=w1->next) {
                  if (is_portal(w1)) {
                    *w2=new Tportal();
                    ((Tportal *)*w2)->target=((Tportal *)w1)->target;
                    ((Tportal *)*w2)->sector=((Tportal *)w1)->sector;
                  }
                  else if (is_hole(w1)) {
                    *w2=new Thole();
                    ((Thole *)*w2)->sector=((Thole *)w1)->sector;
                  }
                  else {
                    *w2=new Twall();
                  }
                  (*w2)->texture=w1->texture;
                  (*w2)->z1c=w1->z1c;
                  (*w2)->z2c=w1->z2c;
                  (*w2)->options=w1->options;
                  w2=&(*w2)->next;
                  *w2=NULL;
                }
              }
              if (l1==l0) flmerge0=true;
              else flmerge=true;
            }
          }
        }
        if (flmerge)
          mergecontours(s);
        if (fl) s->changeheight(); // updates the heights of the floor and ceiling
      }
    }
  }
  mergecontours(s0);
  s0->changeheight();
}

// if two sectors have common line, creates a hole/portal between them
void makeholes( void )
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next)
      makehole(c,s);
  }
}

// moves all selected vertices to their new position
void updateverts( void )
{
  Tvertlist *v;
  for (v=selv;v;v=v->next) {
    v->x=v->v->x;
    v->y=v->v->y;
  }
  for (v=selv;v;v=v->next) {
    int vi=v->v-verts;
    Tcluster *c;
    for (c=map->clusters;c;c=c->next) {
      Tsector *s;
      for (s=c->sectors;s;s=s->next) {
        // updates the lengths of the lines
        Tline *l;
        bool fl=false;
        for (l=s->lines;l;l=l->next) {
          if (l->v1==vi || l->v2==vi) {
            coord3d dx=verts[l->v2].x-verts[l->v1].x;
            coord3d dy=verts[l->v2].y-verts[l->v1].y;
            l->len=sqrt(dx*dx+dy*dy);
            fl=true;
          }
        }
        if (fl) updatesector(s);
      }
    }
  }
  if (selv && !selv->next) {
    // if only one vertex is selected, tries to merge it with another
    if (mergevertex(selv->v-verts)) {
      deselectvertices();
      makeholes();
    }
  }
}

// draws all selected vertices
void drawselectedverts(HDC hdc)
{
  for (Tvertlist *v=selv;v;v=v->next)
    if (vertvisible(v->v-verts)) {
      int x=xymap2scrx(v->v->x);
      int y=xymap2scry(v->v->y);
      MoveTo(hdc,x,y);
      LineTo(hdc,x+1,y);
    }
}

//// line selection
// selects line near (sx,sy) (screen coordinates)
bool selectline(int sx,int sy)
{
  coord3d x=xyscr2mapx(sx);
  coord3d y=xyscr2mapy(sy);
  if (!findline(&curline,&curlinesec,x,y)) return false;
  cursector=NULL;
  return true;
}

//// sector selection
// finds the sector containing (x,y) (map coordinates)
Tsector *findsectorf(coord3d x,coord3d y)
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next)
    if (c->visible) {
      Tsector *s;
      for (s=c->sectors;s;s=s->next)
        if (s->inside(x,y)) return s;
    }
  return NULL;
}

// finds the sector containing (sx,sy) (screen coordinates)
Tsector *findsector(int sx,int sy)
{
  coord3d x=xyscr2mapx(sx);
  coord3d y=xyscr2mapy(sy);
  Tcluster *c;
  for (c=map->clusters;c;c=c->next)
    if (c->visible) {
      Tsector *s;
      for (s=c->sectors;s;s=s->next)
        if (s->inside(x,y)) return s;
    }
  return NULL;
}

static coord3d minheight;

// selects a sector
void selectsector(Tsector *s)
{
  cursector=s;
  curline=NULL;
  if (s) {
    deselectvertices();
    curline=NULL;
  }
  curzfc=cursector->zfc;
  curzcc=cursector->zcc;
  minheight=10000;
  for (Tline *l=cursector->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    coord3d dz=cursector->getzc(x1,y1)-cursector->getzf(x1,y1);
    if (minheight>dz) minheight=dz;
  }
}

// selects the sector containing (sx,sy)
bool selectsector(int sx,int sy)
{
  Tsector *s=findsector(sx,sy);
  if (!s) {
    cursector=NULL;
    return false;
  }
  selectsector(s);
  return true;
}

// checks if (x,y) is near the line segment (x1,y1)-(x2,y2)
static bool nearsegment(coord3d x1,coord3d y1,coord3d x2,coord3d y2,coord3d x,coord3d y)
{
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

// checks if the floor of cursector is near (sx,sy) in XZ view
bool selectfloorxz(int sx,int sy)
{
  coord3d x=xzscr2mapx(sx);
  coord3d z=xzscr2mapz(sy);
  delta=(DELTA/scale);
  for (Tline *l=cursector->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    coord3d x2=verts[l->v2].x;
    coord3d y2=verts[l->v2].y;
    coord3d z1=cursector->getzf(x1,y1);
    coord3d z2=cursector->getzf(x2,y2);
    if (nearsegment(x1,z1,x2,z2,x,z)) return true;
  }
  return false;
}

// checks if the ceiling of cursector is near (sx,sy) in XZ view
bool selectceilingxz(int sx,int sy)
{
  coord3d x=xzscr2mapx(sx);
  coord3d z=xzscr2mapz(sy);
  delta=(DELTA/scale);
  for (Tline *l=cursector->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    coord3d x2=verts[l->v2].x;
    coord3d y2=verts[l->v2].y;
    coord3d z1=cursector->getzc(x1,y1);
    coord3d z2=cursector->getzc(x2,y2);
    if (nearsegment(x1,z1,x2,z2,x,z)) return true;
  }
  return false;
}

// checks if the floor of cursector is near (sx,sy) in YZ view
bool selectflooryz(int sx,int sy)
{
  coord3d y=yzscr2mapy(sx);
  coord3d z=yzscr2mapz(sy);
  delta=(DELTA/scale);
  for (Tline *l=cursector->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    coord3d x2=verts[l->v2].x;
    coord3d y2=verts[l->v2].y;
    coord3d z1=cursector->getzf(x1,y1);
    coord3d z2=cursector->getzf(x2,y2);
    if (nearsegment(y1,z1,y2,z2,y,z)) return true;
  }
  return false;
}

// checks if the ceiling of cursector is near (sx,sy) in YZ view
bool selectceilingyz(int sx,int sy)
{
  coord3d y=yzscr2mapy(sx);
  coord3d z=yzscr2mapz(sy);
  delta=(DELTA/scale);
  for (Tline *l=cursector->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    coord3d x2=verts[l->v2].x;
    coord3d y2=verts[l->v2].y;
    coord3d z1=cursector->getzc(x1,y1);
    coord3d z2=cursector->getzc(x2,y2);
    if (nearsegment(y1,z1,y2,z2,y,z)) return true;
  }
  return false;
}

// changes the height of the floor
void dragfloor(coord3d dz)
{
  coord3d z=curzfc+dz;
  if (cursector->zfa==0 && cursector->zfb==0) z=snaptogrid(z);
  dz=z-curzfc;
  if (dz>minheight) dz=minheight,z=curzfc+dz;
  cursector->zfc=z;
  cursector->changeheight();
}

// changes the height of the ceiling
void dragceiling(coord3d dz)
{
  coord3d z=curzcc+dz;
  if (cursector->zca==0 && cursector->zcb==0) z=snaptogrid(z);
  dz=z-curzcc;
  if (dz<-minheight) dz=-minheight,z=curzcc+dz;
  cursector->zcc=z;
  cursector->changeheight();
}

// updates the floor and ceiling when the dragging stops
void updateflcl( void )
{
  minheight+=(cursector->zcc-curzcc)-(cursector->zfc-curzfc);
  curzfc=cursector->zfc;
  curzcc=cursector->zcc;
}
