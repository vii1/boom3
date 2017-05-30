//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Hole.cpp - class Hole implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _THOLE_CPP
#include "boom.h"
#include "read.h"
#include "write.h"

Hole::Hole():
  Wall()
{
}

// saves the hole to the current file
void Hole::save(Map *m,Cluster *c,Sector *s,Line *l)
{
  wrchar(wtHOLE);
  savewall();
  // writes the index of the target sector
#ifdef EDITOR
  int si;
  Sector *s0;
  for (si=0,s0=c->sectors;s0!=sector;si++,NEXTSECTOR(s0));
  wrlong(si);
#else
  wrlong(sector-c->sectors);
#endif
}

// loads the hole from the current file
bool Hole::load()
{
  Wall::load();
  // reads the index of the target sector
  *(long *)&sector=rdlong();
  return true;
}

// initializes the hole after the loading
void Hole::postload(Map *m,Cluster *c,Sector *s,Line *l)
{
  // converts sector index to sector pointer
#ifdef EDITOR
  int si;
  Sector *s0;
  for (si=0,s0=c->sectors;si<*(long *)&sector;si++,NEXTSECTOR(s0));
  sector=s0;
#else
  sector=c->sectors+*(long *)&sector;
#endif
}
