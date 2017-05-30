//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Wall.h - class Wall interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TWALL_H
#define _TWALL_H

#include "Ttrap.h"
#include "Ttexture.h"


enum
{
  waSOLID  = 1, // the wall is solid (with texture)
  waPORTAL = 2  // the wall is portal to another cluster
};

// basic class for the walls in the engine
class Wall: public MapItem
{
public:
  int texture;
  coord3d z1c,z2c;  // heights of the top left and top right corners of the wall

  Wall();

  // saves the wall to the current file
  virtual void save(Map *m,Cluster *c,Sector *s,Line *l);
  // saves the wall information
  void savewall( void );
  // loads the wall from the current file
  virtual bool load();
  // initializes the wall after the loading
  virtual void postload(Map *m,Cluster *c,Sector *s,Line *l);

  // draws the wall
  virtual void draw(Monotone *fp);

#ifdef EDITOR
  Wall *next;
#endif
};

#define is_solid(w) (((w)->options&waSOLID)!=0)
#define is_portal(w) (((w)->options&waPORTAL)!=0)
#define is_visible(w) (((w)->options&(waPORTAL|waSOLID))!=0)
#define is_hole(w) (((w)->options&waSOLID)==0)

#ifndef EDITOR
#define NEXTWALL(w) (w)++
#else
#define NEXTWALL(w) (w)=&(*(w))->next
#endif

#endif
