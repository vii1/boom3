//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   sector.h - sector editing interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _SECTOR_H
#define _SECTOR_H

#include "engine/boom.h"

// deletes the sector sc and all lines in it. deletes all holes that lead to sc
void delsector(Tsector *s);
// deletes a sector
void delsector(Tmap *m,Tcluster *c,Tsector *s);
// recalculates the bounding box of s. updates line's options
void updatesector(Tsector *s);
// adds sector s2 to s1
bool joinsectors(Tsector *s1,Tsector *s2);
// creates a sector inside the contour that begins with l
void insertsector(Tsector *s,Tline *l);
// changes the cluster of the sector s0 to c0
void changecluster(Tsector *s0,Tcluster *c0);

#ifdef _SECTOR_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL coord3d defaultfloorz,defaultceilingz;

#undef GLOBAL

#endif
