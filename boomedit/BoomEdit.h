//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   boomedit.h - editor's main data and function definitions           //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOMEDIT_H
#define _BOOMEDIT_H

#include "engine/types.h"
#include "win32/win32.h"
#include "engine/boom.h"

#ifdef _BOOMEDIT_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

// size and position of the four views
GLOBAL RECT viewxy,viewyz,viewxz,view3d;
GLOBAL int viewh1,viewh2,viewh3;
GLOBAL int vieww1,vieww2,vieww3;
GLOBAL coord3d viewxyx0,viewxyy0,viewxzx0,viewxzz0,viewyzy0,viewyzz0;
// current scale
GLOBAL float scale;
// current grid size
GLOBAL int gridsize;
GLOBAL bool gridfl; // is the grid visible
GLOBAL bool startpfl;  // is the start position visible
GLOBAL bool currentpfl;  // is the current position visible

// converts from map to screen coordinates in XY view
inline int xymap2scrx(int i)
{
  return (verts[i].x-viewxyx0)*scale;
}

inline int xymap2scry(int i)
{
  return viewh1-(verts[i].y-viewxyy0)*scale;
}

inline int xymap2scrx(coord3d x)
{
  return (x-viewxyx0)*scale;
}

inline int xymap2scry(coord3d y)
{
  return viewh1-(y-viewxyy0)*scale;
}






// converts from map to screen coordinates in XZ view
inline int xzmap2scrx(int i)
{
  return (verts[i].x-viewxzx0)*scale;
}

inline int xzmap2scrz(coord3d z)
{
  return viewh2-(z-viewxzz0)*scale;
}

inline int xzmap2scrx(coord3d x)
{
  return (x-viewxzx0)*scale;
}





// converts from map to screen coordinates in YZ view
inline int yzmap2scry(int i)
{
  return (verts[i].y-viewyzy0)*scale;
}

inline int yzmap2scrz(coord3d z)
{
  return viewh3-(z-viewyzz0)*scale;
}

inline int yzmap2scry(coord3d y)
{
  return (y-viewyzy0)*scale;
}








// converts from screen to map coordinates in XY view
inline coord3d xyscr2mapx(int x)
{
  return (x-viewxy.left)/scale+viewxyx0;
}

inline coord3d xyscr2mapy(int y)
{
  return (viewxy.bottom-y)/scale+viewxyy0;
}

// converts from screen to map coordinates in XZ view
inline coord3d xzscr2mapx(int x)
{
  return (x-viewxz.left)/scale+viewxzx0;
}

inline coord3d xzscr2mapz(int y)
{
  return (viewxz.bottom-y)/scale+viewxzz0;
}

// converts from screen to map coordinates in YZ view
inline coord3d yzscr2mapy(int x)
{
  return (x-viewyz.left)/scale+viewyzy0;
}

inline coord3d yzscr2mapz(int y)
{
  return (viewyz.bottom-y)/scale+viewyzz0;
}

// draws the vertex selection box
void drawselbox(HDC hdc);
// draws the currently edited contour
void drawcontour(HDC hdc);
// sets the message in the status bar
void setmessage(char *s,bool timeout=false);
// set the "dirty" flag of the map
void setdirty(bool state);
// edits the properties of the new sector
// returns the parent cluster
Tcluster *newsector( void );

#undef GLOBAL

#endif
