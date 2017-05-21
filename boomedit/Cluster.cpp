//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   cluster.cpp - cluster editing implementation                       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _CLUSTER_CPP
#include "cluster.h"
#include "vertex.h"
#include "select.h"

#include "boom.h"

#include <stdio.h>

// initializes the clusters (sets all clusters to visible)
void init_clusters( void )
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next)
    c->visible=true;
}

// finds the cluster, containing the sector s0
Tcluster *findcluster(Tsector *s0)
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next)
      if (s==s0) return c;
  }
  return 0;
}

// finds the cluster, containing the line l0
Tcluster *findcluster(Tline *l0)
{
  Tcluster *c;
  for (c=map->clusters;c;c=c->next) {
    Tsector *s;
    for (s=c->sectors;s;s=s->next) {
      Tline *l;
      for (l=s->lines;l;l=l->next)
        if (l==l0) return c;
    }
  }
  return 0;
}

// hides the cluster c
void hidecluster(Tcluster *c)
{
  c->visible=false;
  showvertices();
  if (cursector && findcluster(cursector)==c) cursector=NULL;
  if (curline && findcluster(curline)==c) curline=NULL;
  deselecthidden();
}

// shows the cluster c
void showcluster(Tcluster *c)
{
  c->visible=true;
  showvertices();
}

//creates a new cluster
void newcluster( void )
{
  Tcluster **cc;
  for (cc=&map->clusters;*cc;cc=&(*cc)->next);
  *cc=new Tcluster();
  (*cc)->sectors=NULL;
  (*cc)->sectorsnum=0;
  (*cc)->next=NULL;
  (*cc)->visible=true;
  (*cc)->options=0;
  map->clustersnum++;
}
