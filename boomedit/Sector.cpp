//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   sector.cpp - sector editing implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _SECTOR_CPP
#include "boomedit.h"
#include "sector.h"
#include "line.h"
#include "contour.h"
#include "select.h"
#include "cluster.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>

// deletes the sector sc and all lines in it. deletes all holes that lead to sc
void delsector(Tsector *sc)
{
  Tline *l,*n;
  for (l=sc->lines;l;l=n) {
    n=l->next;
    delline(l);
  }
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      bool f=false;
      for (l=s->lines;l;l=l->next) {
       qqq:
        Twall **w;
        for (w=l->walls;*w;w=&(*w)->next) {
          if (is_hole(*w) && ((Thole *)(*w))->sector==sc) {
            Twall *wn=(*w)->next;
            delete *w;
            *w=wn;
            l->wallsnum--;
            if (w!=l->walls && wn) {
              wn=(*w)->next;
              delete *w;
              *w=wn;
              l->wallsnum--;
            }
            if (l->wallsnum==0) {
              l->wallsnum=1;
              Twall *w=new Twall();
              *l->walls=w;
              w->next=NULL;
              w->options=waSOLID;
              w->texture=defwalltxt;
              w->z1c=s->getzc(verts[l->v1].x,verts[l->v1].y);
              w->z2c=s->getzc(verts[l->v2].x,verts[l->v2].y);
            }
            f=true;
            goto qqq;
          }
        }
      }
      if (f) s->changeheight();
    }
  }
  delete sc;
}

// recalculates the bounding box of s. updates line's options
void updatesector(Tsector *s)
{
  coord3d minx=1000000;
  coord3d maxx=-1000000;
  coord3d miny=1000000;
  coord3d maxy=-1000000;
  Tline *l;
  for (l=s->lines;l;l=l->next) {
    coord3d x1=verts[l->v1].x;
    coord3d y1=verts[l->v1].y;
    if (minx>x1) minx=x1;
    if (miny>y1) miny=y1;
    if (maxx<x1) maxx=x1;
    if (maxy<y1) maxy=y1;
    if (l->next==l->nextc) l->options&=~liCONTOUR_END;
    else l->options|=liCONTOUR_END;
  }
  s->minx=minx;
  s->maxx=maxx;
  s->miny=miny;
  s->maxy=maxy;
}

// deletes a sector
void delsector(Tmap *m,Tcluster *c,Tsector *s)
{
  Tsector **ss;
  for (ss=&c->sectors;*ss;ss=&(*ss)->next)
    if (*ss==s) {
      *ss=s->next;
      break;
    }
  c->sectorsnum--;
  delsector(s);
}

// changes all refferences of s1 to s2
static void replacesector(Tsector *s1,Tsector *s2)
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      for (l=s->lines;l;l=l->next) {
        Twall *w;
        for (w=*l->walls;w;w=w->next) {
          if (is_hole(w)) {
            Thole *h=(Thole *)w;
            if (h->sector==s1) h->sector=s2;
          }
        }
      }
    }
  }
}

// adds sector s2 to s1
bool joinsectors(Tsector *s1,Tsector *s2)
{
  Tcluster *c1=findcluster(s1);
  Tcluster *c2=findcluster(s2);
  if (c1!=c2) return false;
  Tline *l;
  for (l=s1->lines;l->next;l=l->next);
  l->next=s2->lines;
  s1->linesnum+=s2->linesnum;
  s2->lines=NULL;
  s2->linesnum=0;
  mergecontours(s1);
  replacesector(s2,s1);
  delsector(map,c2,s2);
  s1->changeheight();
  updatesector(s1);
  return true;
}

// creates a sector inside the contour that begins with l
void insertsector(Tsector *s,Tline *l)
{
  Tnode *n=NULL,*n1;
  Tline *ln;
  // deletes all lines in the contour
  for (;l;l=ln) {
    ln=l->next;
    n1=(Tnode *)malloc(sizeof(Tnode));
    n1->x=verts[l->v1].x;
    n1->y=verts[l->v1].y;
    n1->next=n;
    n=n1;
    delline(l);
  }
  // creates a new sector inside
  cursector=NULL;
  curline=NULL;
  addcontour(n);
  for (;n;n=n1) {
    n1=n->next;
    free(n);
  }
}

// changes the cluster of the sector s0 to c0
void changecluster(Tsector *s0,Tcluster *c0)
{
  Tcluster *c1=findcluster(s0);
  Tsector **ss;
  for (ss=&c1->sectors;*ss;ss=&(*ss)->next)
    if (*ss==s0) break;
  *ss=s0->next;
  c1->sectorsnum--;
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      for (l=s->lines;l;l=l->next) {
        Twall **w;
        for (w=l->walls;*w;w=&(*w)->next) {
          if (is_hole(*w) && ((Thole *)(*w))->sector==s0) {
            // changes all holes to s0 into portals and vice versa
            if (is_portal(*w)) {
              if (c==c0) {
                Thole *h=new Thole();
                h->next=(*w)->next;
                h->texture=(*w)->texture;
                h->z1c=(*w)->z1c;
                h->z2c=(*w)->z2c;
                h->sector=s0;
                h->options=0;
                delete *w;
                *w=h;
              }
              else ((Tportal *)(*w))->target=c0;
            }
            else {
              Tportal *p=new Tportal();
              p->next=(*w)->next;
              p->texture=(*w)->texture;
              p->z1c=(*w)->z1c;
              p->z2c=(*w)->z2c;
              p->sector=s0;
              p->target=c0;
              p->options=waPORTAL;
              delete *w;
              *w=p;
            }
          }
        }
      }
    }
  }
  {
    Tline *l;
    for (l=s0->lines;l;l=l->next) {
      Twall **w;
      for (w=l->walls;*w;w=&(*w)->next) {
        if (is_hole(*w)) {
          if (is_portal(*w)) {
            // changes all portals from s0 to c0 into holes
            if (((Tportal *)(*w))->target==c0) {
              Thole *h=new Thole();
              h->next=(*w)->next;
              h->texture=(*w)->texture;
              h->z1c=(*w)->z1c;
              h->z2c=(*w)->z2c;
              h->sector=((Thole *)(*w))->sector;
              h->options=0;
              delete *w;
              *w=h;
            }
          }
          else {
            // changes all holes in s0 into portals
            Tportal *p=new Tportal();
            p->next=(*w)->next;
            p->texture=(*w)->texture;
            p->z1c=(*w)->z1c;
            p->z2c=(*w)->z2c;
            p->sector=((Thole *)(*w))->sector;
            p->target=findcluster(p->sector);
            p->options=waPORTAL;
            delete *w;
            *w=p;
          }
        }
      }
    }
  }
  s0->next=c0->sectors;
  c0->sectors=s0;
  c0->sectorsnum++;
}
