//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Wall.cpp - class Wall implementation                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _WALL_CPP
#include "boom.h"
#include "draw.h"
#include "read.h"
#include "write.h"

#include <stdio.h>

Wall::Wall():
  MapItem()
{
#ifdef EDITOR
  next=NULL;
#endif
}

// saves the wall to the current file
void Wall::save(Map *m,Cluster *c,Sector *s,Line *l)
{
  wrchar(wtWALL);
  savewall();
}

// saves the wall information
void Wall::savewall( void )
{
  MapItem::save();
  // writes the texture
  wrlong( texture );
  // writes the heights
  wrfloat( z1c );
  wrfloat( z2c );
}

// loads the wall from the current file
bool Wall::load()
{
  MapItem::load();
  // reads the texture
  texture=rdlong();

  // reads the heights
  z1c=rdfloat();
  z2c=rdfloat();
  return true;
}

// initializes the wall after the loading
void Wall::postload(Map *m,Cluster *c,Sector *s,Line *l)
{
}

// draws the wall
void Wall::draw(Monotone* mp)
{
  setdrawtexture(texture);
  drawmp(mp);
}
