//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   contour.h - contour editing interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _CONTOUR_H
#define _CONTOUR_H

#include "boom.h"

struct Tnode {
  coord3d x,y;
  Tnode *next;
};

// adds a new contour in the map
// if the contour is inside the current sector, creates an inner contour
// if the contour is inside some other sector, creates an inner sector
// otherwise creates an independant sector
void addcontour(Tnode *cont);
// removes the duplicated lines in the sector s and fixes the contours
void mergecontours(Tsector *s);
// finds the contour with minimal area that contains the point (x,y)
bool selcontour(coord3d x,coord3d y);
// calculates the area of the contour that begins with l0
double contourarea(Tline *l0);

#endif
