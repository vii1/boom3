//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Portal.h - class Portal interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TPORTAL_H
#define _TPORTAL_H

#include "Tclip.h"

// Portal is a wall without texture that leads to another cluster
// it combines the features of hole and clip
class Portal: public Hole, public Clip
{
public:
  Cluster *target; // target cluster

  Portal();

  // saves the portal to the current file
  virtual void save(Map *m,Cluster *c,Sector *s,Line *l);
  // loads the portal from the current file
  virtual bool load();
  // initializes the portal after the loading
  virtual void postload(Map *m,Cluster *c,Sector *s,Line *l);
  // draws the portal
  virtual void draw(Monotone *mp);
};

#endif
