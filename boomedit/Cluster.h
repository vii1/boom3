//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   cluster.h - cluster editing interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _CLUSTER_H
#define _CLUSTER_H

#include "boom.h"

// initializes the clusters (sets all clusters to visible)
void init_clusters( void );
// finds the cluster, containing the sector s0
Tcluster *findcluster(Tsector *s0);
// finds the cluster, containing the line l0
Tcluster *findcluster(Tline *l0);
// hides the cluster c
void hidecluster(Tcluster *c);
// shows the cluster c
void showcluster(Tcluster *c);
//creates a new cluster
void newcluster( void );

#endif
