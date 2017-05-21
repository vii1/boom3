//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   select.h - selection routines interface                            //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _SELECT_H
#define _SELECT_H

#include "types.h"
#include "boom.h"

#include <windows.h>

//// vertex selection
// checks if the vertex is currently selected
bool vertexselected(int sx,int sy);
// selects a vertex near given position
bool selectvertex(int sx,int sy);
// marks vertex as selected
void selectvertex(Tvertex *v);
// selects all vertices in the rectangle
void selectvertexbox(RECT *r);
// changes the position of the selected vertices with a vector
void dragvertex(coord3d dx,coord3d dy);
// deselects all vertices
void deselectvertices( void );
// deselects all hidden vertices
void deselecthidden( void );
// moves all selected vertices to their new position
void updateverts( void );
// draws all selected vertices
void drawselectedverts(HDC hdc);
// if two sectors have common line, creates a hole/portal between them
void makeholes( void );

//// line selection
bool selectline(int sx,int sy);

//// sector selection
// finds the sector containing (sx,sy) (screen coordinates)
Tsector *findsector(int sx,int sy);
// finds the sector containing (x,y) (map coordinates)
Tsector *findsectorf(coord3d x,coord3d y);
// selects a sector
void selectsector(Tsector *s);
// selects the sector containing (sx,sy)
bool selectsector(int sx,int sy);
// checks if the floor of cursector is near (sx,sy) in XZ view
bool selectfloorxz(int sx,int sy);
// checks if the ceiling of cursector is near (sx,sy) in XZ view
bool selectceilingxz(int sx,int sy);
// checks if the floor of cursector is near (sx,sy) in YZ view
bool selectflooryz(int sx,int sy);
// checks if the ceiling of cursector is near (sx,sy) in YZ view
bool selectceilingyz(int sx,int sy);
// changes the height of the floor
void dragfloor(coord3d dz);
// changes the height of the ceiling
void dragceiling(coord3d dz);
// updates the floor and ceiling when the dragging stops
void updateflcl( void );

#ifdef _SELECT_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL Tsector *cursector; // currently selected sector
GLOBAL Tline *curline; // currently selected line
GLOBAL Tsector *curlinesec; // secttor containing currently selected line

#undef GLOBAL

#endif
