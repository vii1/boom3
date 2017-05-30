//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Portal.cpp - class Portal implementation                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _TPORTAL_CPP
#include "boom.h"
#include "read.h"
#include "write.h"


Portal::Portal():
  Hole()
{
}

// saves the portal to the current file
void Portal::save(Map *m,Cluster *c,Sector *s,Line *l)
{
  wrchar(wtPORTAL);
  savewall();
#ifdef EDITOR
  int si;
  Sector *s0;
  for (si=0,s0=target->sectors;s0!=sector;si++,NEXTSECTOR(s0));
  // writes the index of the target sector
  wrlong(si);
  int ci;
  Cluster *c0;
  for (ci=0,c0=m->clusters;c0!=target;ci++,NEXTCLUSTER(c0));
  // writes the index of the target cluster
  wrlong(ci);
#else
  // writes the index of the target sector
  wrlong(sector-target->sectors);
  // writes the index of the target cluster
  wrlong(target-m->clusters);
#endif
}

// loads the portal from the current file
bool Portal::load()
{
  Hole::load();
  // reads the index of the target cluster
  *(long *)&target = rdlong();
  return true;
}

// initializes the portal after the loading
void Portal::postload(Map *m,Cluster *c,Sector *s,Line *l)
{
  // converts cluster index to cluster pointer
  target=m->getcluster(*(long *)&target);
  // converts sector index to sector pointer
#ifdef EDITOR
  int si;
  Sector *s0;
  for (si=0,s0=target->sectors;si<*(long *)&sector;si++,NEXTSECTOR(s0));
  sector=s0;
#else
  sector=target->sectors+*(long *)&sector;
#endif
}

// draws the portal
void Portal::draw(Monotone *mp)
{
  // stores the current clip
  Clip *old=cur_clip;

  // sets the portal to be the current clip
  xmin=mp->xmin;
  xmax=mp->xmax;
  ymin=mp->ymin;
  ymax=mp->ymax;
  traps=mp->traps;
  set_cur_clip(this);

  // draws the target cluster
  target->draw();

  // restores the current clip
  mp->traps=traps;
  cur_clip=old;
}
