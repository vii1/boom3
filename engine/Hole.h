//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Hole.h - class Hole interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _THOLE_H
#define _THOLE_H

// Hole is a wall without texture that leads to another sector
class Hole: public Wall
{
public:
  Sector *sector;  // target sector

  Hole();

  // saves the hole to the current file
  virtual void save(Map *m,Cluster *c,Sector *s,Line *l);
  // loads the hole from the current file
  virtual bool load();
  // initializes the hole after the loading
  virtual void postload(Map *m,Cluster *c,Sector *s,Line *l);
};

#endif
