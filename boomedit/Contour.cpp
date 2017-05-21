//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   contour.cpp - contour editing implementation                       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _CONTOUR_CPP
#include "contour.h"
#include "boomedit.h"
#include "select.h"
#include "vertex.h"
#include "line.h"
#include "sector.h"
#include "texture.h"

#include <math.h>
#include <malloc.h>

// calculates the area inside the contour c
static double area(Tnode *c)
{
  double a=0;
  coord3d x0=c->x;
  coord3d y0=c->y;
  for (c=c->next;c->next;c=c->next)
    a+=(c->y-y0)*(c->next->x-x0)-(c->x-x0)*(c->next->y-y0);
  return a;
}

// reverses the order of the vertices in the contour c
static void reverse(Tnode **c)
{
  Tnode *n0=NULL,*n1=*c;
  while (n1) {
    Tnode *q=n1->next;
    n1->next=n0;
    n0=n1;
    n1=q;
  }
  *c=n0;
}

// removes the duplicated lines in the sector s and fixes the contours
void mergecontours(Tsector *s)
{
  if (!s->lines) return;
  // removes the duplicated lines in the sector s
  Tline **l1,**l2;
  int ln=0;
  for (l1=&s->lines;*l1;l1=&(*l1)->next) {
    for (l2=&(*l1)->next;*l2;l2=&(*l2)->next)
      if ((*l1)->v1==(*l2)->v2 && (*l1)->v2==(*l2)->v1) {
        Tline *l=*l2;
        *l2=l->next;
        delline(l);
        l=*l1;
        *l1=l->next;
        delline(l);
        l2=l1;
        ln+=2;
        if (!*l1) break;
      }
    if (!*l1) break;
  }

  // for each line finds it's next
  Tline *l0=NULL,*lc,**ll0=&l0;
  while (s->lines) {
    lc=s->lines;
    Tline *l=lc;
    s->lines=s->lines->next;
   qqq1:
    for (l1=&s->lines;*l1;l1=&(*l1)->next) {
      if ((*l1)->v1==l->v2) {
        l->next=*l1;
        l->nextc=*l1;
        l->options&=~liCONTOUR_END;
        l=*l1;
        *l1=l->next;
        goto qqq1;
      }
    }
    l->next=NULL;
    l->nextc=lc;
    l->options|=liCONTOUR_END;
    *ll0=lc;
    ll0=&l->next;
  }

  s->lines=l0;
  s->linesnum-=ln;
}

// checks if sector s contains line with ends (x1,y1) and (x2,y2)
static int contain(Tsector *s,coord3d x1,coord3d y1,coord3d x2,coord3d y2)
{
  for (Tline *l=s->lines;l;l=l->next) {
    coord3d lx1=verts[l->v1].x;
    coord3d ly1=verts[l->v1].y;
    coord3d lx2=verts[l->v2].x;
    coord3d ly2=verts[l->v2].y;
    if (x1==lx1 && y1==ly1 && x2==lx2 && y2==ly2) return 1;
    if (x2==lx1 && y2==ly1 && x1==lx2 && y1==ly2) return -1;
  }
  return 0;
}

// checks if the contour cont is inside the sector s
static bool inside(Tsector *s,Tnode *cont)
{
  for (Tnode *c=cont;c;c=c->next) {
    coord3d x1=c->x;
    coord3d y1=c->y;
    coord3d x2=cont->x;
    coord3d y2=cont->y;
    if (c->next) {
      x2=c->next->x;
      y2=c->next->y;
    }
    int q=contain(s,x1,y1,x2,y2);
    if (q==-1) continue;
    if (q==1) return false;
    if (s->inside(x1,y1) && s->inside(x2,y2)) continue;
    if (!s->inside((x1+x2)/2,(y1+y2)/2)) return false;
  }
  return true;
}

// adds a new contour in the map
// if the contour is inside the current sector, creates an inner contour
// if the contour is inside some other sector, creates an inner sector
// otherwise creates an independant sector
void addcontour(Tnode *cont)
{
  if (area(cont)>0) reverse(&cont->next);
  if (cursector && inside(cursector,cont)) {
    // the contour is inside cursector
    Tnode *c=cont;
    for (Tline **ll=&cursector->lines;*ll;ll=&((*ll)->next));
    Tline **l0=ll;
    // creates a line for each segment in the contour
    // adds the contour to cursector
    int n;
    for (n=0;c;c=c->next,ll=&((*ll)->next),n++) {
      Tline *l=new Tline();
      l->v1=addvertex(c->x,c->y);
      if (c->next) l->v2=addvertex(c->next->x,c->next->y);
      else l->v2=addvertex(cont->x,cont->y);
      Twall *w=new Twall();
      l->walls=(Twall **)malloc(sizeof(Twall*));
      *l->walls=w;
      l->wallsnum=1;
      l->options=0;
      coord3d dx=verts[l->v2].x-verts[l->v1].x;
      coord3d dy=verts[l->v2].y-verts[l->v1].y;
      l->len=sqrt(dx*dx+dy*dy);
      w->next=NULL;
      w->texture=defwalltxt;
      w->z1c=cursector->getzc(verts[l->v1].x,verts[l->v1].y);
      w->z2c=cursector->getzc(verts[l->v2].x,verts[l->v2].y);
      w->options=waSOLID;
      *ll=l;
    }
    *ll=NULL;
    for (Tline *l=*l0;l->next;l=l->next)
      l->nextc=l->next;
    l->nextc=*l0;
    l->options=liCONTOUR_END;
    cursector->linesnum+=n;
    mergecontours(cursector);
    setmessage("Contour added",true);
  }
  else {
    // the contour is not inside cursector
    Tnode *cn=cont;
    Tcluster *c;
    Tsector *s;
    for (c=map->clusters;c;c=c->next)
      if (c->visible) {
        for (s=c->sectors;s;s=s->next) {
          if (inside(s,cn)) goto q10;
      }
    }
    s=NULL; // contour is not inside any sector
  q10:
    if (!s) {
      c=newsector(); // selects the cluster for the new sector
      if (!c) return;
    }
    Tsector *ns=new Tsector();
    if (s) {
      // if the contour is inside s, creates another contour with
      // opposite direction and adds it to s
      for (Tline **ll=&s->lines;*ll;ll=&((*ll)->next));
      Tline **l0=ll;
      int n;
      for (n=0,cn=cont;cn;cn=cn->next,ll=&((*ll)->next),n++) {
        Tline *l=new Tline();
        l->v1=addvertex(cn->x,cn->y);
        if (cn->next) l->v2=addvertex(cn->next->x,cn->next->y);
        else l->v2=addvertex(cont->x,cont->y);
        Thole *w=new Thole();
        l->walls=(Twall **)malloc(sizeof(Twall*));
        *l->walls=w;
        l->wallsnum=1;
        l->options=0;
        coord3d dx=verts[l->v2].x-verts[l->v1].x;
        coord3d dy=verts[l->v2].y-verts[l->v1].y;
        l->len=sqrt(dx*dx+dy*dy);
        w->next=NULL;
        w->texture=-1;
        w->z1c=s->getzc(verts[l->v1].x,verts[l->v1].y);
        w->z2c=s->getzc(verts[l->v2].x,verts[l->v2].y);
        w->options=0;
        w->sector=ns;
        *ll=l;
      }
      *ll=NULL;
      for (Tline *l=*l0;l->next;l=l->next)
        l->nextc=l->next;
      l->nextc=*l0;
      l->options=liCONTOUR_END;
      s->linesnum+=n;
    }
    reverse(&cont->next);
    if (s) {
      // copies the hights from s to ns
      ns->zfa=s->zfa;
      ns->zfb=s->zfb;
      ns->zfc=s->zfc;
      ns->zca=s->zca;
      ns->zcb=s->zcb;
      ns->zcc=s->zcc;
      ns->tfloor=s->tfloor;
      ns->tceiling=s->tceiling;
    }
    else {
      // initializes the heights in ns with the default valuse
      ns->zfa=0;
      ns->zfb=0;
      ns->zfc=defaultfloorz;
      ns->zca=0;
      ns->zcb=0;
      ns->zcc=defaultceilingz;
      ns->tfloor=deffloortxt;
      ns->tceiling=defceiltxt;
    }
    if (!s) {
      ns->next=c->sectors;
      c->sectors=ns;
    }
    else {
      ns->next=s->next;
      s->next=ns;
    }
    c->sectorsnum++;

    Tline **ll=&ns->lines;
    Tline **l0=ll;
    int n;
    cn=cont;
    ns->minx=ns->maxx=cn->x;
    ns->miny=ns->maxy=cn->y;
    // creates the lines and the walls for ns
    for (n=0,cn=cont;cn;cn=cn->next,ll=&((*ll)->next),n++) {
      if (ns->minx>cn->x) ns->minx=cn->x;
      if (ns->miny>cn->y) ns->miny=cn->y;
      if (ns->maxx<cn->x) ns->maxx=cn->x;
      if (ns->maxy<cn->y) ns->maxy=cn->y;
      Tline *l=new Tline();
      l->v1=addvertex(cn->x,cn->y);
      if (cn->next) l->v2=addvertex(cn->next->x,cn->next->y);
      else l->v2=addvertex(cont->x,cont->y);
      Tline *ls=NULL;
      if (s) {
        // checks if s has a line with the same vertices
        for (ls=s->lines;ls;ls=ls->next)
          if (ls->v1==l->v1 && ls->v2==l->v2) break;
      }
      if (ls) {
        // if there is such, moves it to ns
        *l=*ls;
        ls->walls=NULL;
        ls->wallsnum=0;
        Twall *w;
        // updates the target sector of all holes
        for (w=*l->walls;w;w=w->next) {
          if (is_hole(w)) {
            Tsector *ss=((Thole *)w)->sector;
            for (Tline *ll=ss->lines;ll;ll=ll->next) {
              if (ll->v1==l->v2 && ll->v2==l->v1) {
                Twall *ws;
                for (ws=*ll->walls;ws;ws=ws->next)
                  if (is_hole(ws) && ((Thole *)ws)->sector==s)
                    ((Thole *)ws)->sector=ns;
              }
            }
          }
        }
      }
      else {
        // otherwise creates a new line
        Twall *w;
        if (s) {
          Thole *h=new Thole();
          h->sector=s;
          h->options=0;
          h->texture=-1;
          w=h;
        }
        else {
          w=new Twall();
          w->options=waSOLID;
          w->texture=defwalltxt;
        }
        l->walls=(Twall **)malloc(sizeof(Twall*));
        *l->walls=w;
        l->wallsnum=1;
        l->options=0;
        coord3d dx=verts[l->v2].x-verts[l->v1].x;
        coord3d dy=verts[l->v2].y-verts[l->v1].y;
        l->len=sqrt(dx*dx+dy*dy);
        w->next=NULL;
        w->z1c=ns->getzc(verts[l->v1].x,verts[l->v1].y);
        w->z2c=ns->getzc(verts[l->v2].x,verts[l->v2].y);
      }
      *ll=l;
    }
    *ll=NULL;
    for (Tline *l=*l0;l->next;l=l->next)
      l->nextc=l->next;
    l->nextc=*l0;
    l->options=liCONTOUR_END;
    ns->linesnum=n;
    if (s) mergecontours(s);
    selectsector(ns);
    setmessage("Sector created",true);
  }
  makeholes();
}

// checks if the point (x,y) is inside the contour that begins with l
static bool inside(Tline *l,coord3d x,coord3d y)
{
  int n=0;
  Tline *l0=l;
  do {
    if (l->wallsnum!=1 || is_hole(*l->walls)) return false;
    coord3d x1=verts[l->v1].x-x;
    coord3d y1=verts[l->v1].y-y;
    coord3d x2=verts[l->v2].x-x;
    coord3d y2=verts[l->v2].y-y;
    if (x1<0 && x2<0) goto qqq;
    if (y1==y2) goto qqq;
    if (y1>y2) {
      coord3d q=y1;y1=y2;y2=q;
      q=x1;x1=x2;x2=q;
    }
    if (y2<=0) goto qqq;
    if (y1>0) goto qqq;
    if (x1==0 && x2==0) return true;
    if (x1>=0 && x2>=0) n++;
    else {
      double q=x1*y2-x2*y1;
      if (q==0) return true;
      if (q>0) n++;
    }
   qqq:
    l=l->nextc;
  } while (l!=l0);
  return (n&1)!=0;
}

// calculates the area of the contour that begins with l0
double contourarea(Tline *l0)
{
  double a=0;
  coord3d x0=verts[l0->v1].x;
  coord3d y0=verts[l0->v1].y;
  for (Tline *l=l0;l->nextc!=l0;l=l->nextc)
    a+=(verts[l->v2].y-y0)*(verts[l->nextc->v2].x-x0)-
       (verts[l->v2].x-x0)*(verts[l->nextc->v2].y-y0);
  return a;
}

// finds the contour with minimal area that contains the point (x,y)
bool selcontour(coord3d x,coord3d y)
{
  Tsector *smin=NULL;
  Tline **lmin;
  double amin=10000000000000;
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      // if the contour is a boundary of a sector, exit
      if (s->inside(x,y)) return false;
      Tline **ll;
      bool fl=true;
      for (ll=&s->lines;*ll;ll=&(*ll)->next) {
        if (fl) {
          fl=false;
          if (inside(*ll,x,y)) {
            double a=fabs(contourarea(*ll));
            if (a<amin) {
              amin=a;
              smin=s;
              lmin=ll;
            }
          }
        }
        else if ((*ll)->contour_end()) fl=true;
      }
    }
  }
  if (smin) {
    Tline *l=*lmin;
    Tline *l0;
    int li;
    for (li=1,l0=*lmin;!l0->contour_end();li++,l0=l0->next);
    *lmin=l0->next;
    l0->next=NULL;
    smin->linesnum-=li;
    insertsector(smin,l);
    return true;
  }
  return false;
}
