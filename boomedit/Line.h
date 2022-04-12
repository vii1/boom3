//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   line.h - line editing interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _LINE_H
#define _LINE_H

#include "engine/boom.h"

// deletes the line l and all it's walls
void delline(Tline *l);
// deletes the line l and updates the map
void delline(Tmap *m,Tcluster *c,Tsector *s,Tline *l);
// finds a line near the point (x,y)
// first checks the lines in the sector containing (x,y)
bool findline(Tline **lp,Tsector **sp,coord3d x,coord3d y);
// divides the line l with the vertex v
void splitline(Tline *l,int v);
#endif
