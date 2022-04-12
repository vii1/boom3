//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   vertex.cpp - vertex editing implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _VERTEX_CPP
#include "vertex.h"
#include "boomedit.h"
#include "line.h"
#include "sector.h"

#include <math.h>
#include <stdlib.h>

static int refs[MAX_VERTICES];
static bool viss[MAX_VERTICES];


// checks if the vertex v is visible
bool vertvisible(int v)
{
  return viss[v];
}

// shows the vertices from the visible clusters
void showvertices( void )
{
  memset(viss,0,sizeof(viss));
  Tcluster *c;
  for (c=map->clusters;c;c=c->next)
    if (c->visible) {
      Tsector *s;
      for (s=c->sectors;s;s=s->next) {
        Tline *l;
        for (l=s->lines;l;l=l->next)
          viss[l->v1]=true;
      }
  }
}

// calculates the number of the refferences to each vertex
void initverts( void )
{
  memset(refs,0,sizeof(refs));
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      for (l=s->lines;l;l=l->next) {
        refs[l->v1]++;
        refs[l->v2]++;
      }
    }
  }
  showvertices();
}

// adds a vertex (x,y). returns the index
int addvertex(coord3d x,coord3d y)
{
  int i;
  for (i=0;i<vertsnum;i++)
    if (verts[i].x==x && verts[i].y==y) {
      refs[i]++;
      return i;
    }
  if (!i) {
    verts=(Tvertex *)malloc(MAX_VERTICES*sizeof(Tvertex));
    initverts();
  }
  verts[i].x=x;
  verts[i].y=y;
  refs[i]++;
  viss[i]=true;
  vertsnum++;
  return i;
}

// adds a refference to the vertex v
void refvertex(int v)
{
  refs[v]++;
  viss[v]=true;
}

// removes a refference to the vertex v
void delvertex(int v)
{
  refs[v]--;
  if (refs[v]<=0) {
    verts[v].x=0;
    verts[v].y=0;
    viss[v]=false;
  }
}

// removes the unused vertices
void compactverts( void )
{
  for (int v=0;v<vertsnum;v++) {
    if (refs[v]==0) {
      vertsnum--;
      int i;
      for (i=v;i<vertsnum;i++) {
        verts[i]=verts[i+1];
        refs[i]=refs[i+1];
      }
      refs[i]=0;
      Tcluster *c;
      for (c=map->clusters;c;c=c->next) {
        Tsector *s;
        for (s=c->sectors;s;s=s->next) {
          Tline *l;
          for (l=s->lines;l;l=l->next) {
            if (l->v1>=v) l->v1--;
            if (l->v2>=v) l->v2--;
          }
        }
      }
      v--;
    }
  }
}

// finds the closest node from the grid
coord3d snaptogrid(coord3d x)
{
  if (!gridfl) return x;
  return gridsize*floor((x+gridsize/2)/gridsize);
}

// tries to merge the vertex v with another
bool mergevertex(int v)
{
  int i;
  for (i=0;i<vertsnum;i++)
    if (verts[i].x==verts[v].x && verts[i].y==verts[v].y && v!=i) break;
  if (i==vertsnum) return false;
  Tcluster *c;
 loop:
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      for (l=s->lines;l;l=l->next) {
        if (l->v1==v) {
          l->v1=i;
          refs[i]++;
        }
        if (l->v2==v) {
          l->v2=i;
          refs[i]++;
        }
        if (l->v1==l->v2) {
          delline(map,c,s,l);
          goto loop;
        }
      }
    }
  }
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) updatesector(s);
  }
  refs[v]=1;
  delvertex(v);
  compactverts();
  showvertices();
  setmessage("Vertex deleted",true);
  return true;
}

// if the point (x,y) is near a line, divides the line with it
Tvertex *insertvertex(coord3d x,coord3d y)
{
  Tline *l;
  Tsector *s;
  if (!findline(&l,&s,x,y)) return NULL;
  int v=addvertex(x,y);
  refvertex(v);
  Twall *w;
  for (w=*l->walls;w;w=w->next) {
    if (is_hole(w)) {
      Tsector *s=((Thole *)w)->sector;
      Tline *l1;
      for (l1=s->lines;l1;l1=l1->next)
        if (l1->v1==l->v2 && l1->v2==l->v1) {
          splitline(l1,v);
          refvertex(v);
          refvertex(v);
          s->linesnum++;
        }
    }
  }
  splitline(l,v);
  s->linesnum++;
  return verts+v;
}
