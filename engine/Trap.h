//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Trap.h - Trap and Monotone interface                            //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TTRAP_H
#define _TTRAP_H

#include "types.h"

#define COUNT_TRAPS

#ifdef _TTRAP_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

#ifdef COUNT_TRAPS
GLOBAL int total_traps; // total number of trapezoids
#endif
GLOBAL int csegment; // current segment index

#undef GLOBAL

// A single trapezoid in a Monotone
// y11 +----
//     |    ----+ y21
//     |        |
//     |        |
//     |        |
//     |     ---+ y22
//     |  ---
// y12 +--
//     x1       x2
class Trap
{
public:
  coord2d x2; // second x coordinate
  int un,dn;  // indices of the up and down segments
  coord2d y11,y12,y21,y22;  // the four y coordinates
  Trap *next;  // next trapezoid in the list

  // different kind of constructors to initialize the object
  Trap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22);
  Trap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,int _un,int _dn);
  Trap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,Trap *_next);
  Trap(coord2d _x2,coord2d _y11,coord2d _y12,coord2d _y21,coord2d _y22,int _un,int _dn,Trap *_next);
  ~Trap( void );
};  

class Monotone
{
  public:
    coord2d xmin,xmax;  // bounding box
    coord2d ymin,ymax;
    Trap *traps;  // list of trapezoids
};

#endif
