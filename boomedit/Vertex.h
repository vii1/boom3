//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   vertex.h - vertex editing interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _VERTEX_H
#define _VERTEX_H

#include "boom.h"

// calculates the number of the refferences to each vertex
void initverts( void );
// adds a vertex (x,y). returns the index
int addvertex(coord3d x,coord3d y);
// adds a refference to the vertex v
void refvertex(int v);
// removes a refference to the vertex v
void delvertex(int v);
// tries to merge the vertex v with another
bool mergevertex(int v);
// finds the closest node from the grid
coord3d snaptogrid(coord3d x);

// shows the vertices from the visible clusters
void showvertices( void );
// checks if the vertex v is visible
bool vertvisible(int v);
// removes the unused vertices
void compactverts( void );

// if the point (x,y) is near a line, divides the line with it
Tvertex *insertvertex(coord3d x,coord3d y);
#endif
