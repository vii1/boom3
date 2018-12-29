//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Sector.h - class Sector interface                                //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _TSECTOR_H
#define _TSECTOR_H

#include "Trap.h"
#include "Texture.h"

struct Cont;

enum
{
  seLAST_SECTOR = 0x01, //it's the last sector
};

// Cluster contains all the information for one sector in the cluster
class Sector: public MapItem
{
public:
  Line *lines; // lines in the sector
  int linesnum; // number of the lines in the sector

  float zfa,zfb,zfc,zca,zcb,zcc;  // floor and ceiling equation coeficients
  int tfloor, tceiling; // floor and ceiling textures
  coord3d minx,maxx,miny,maxy;  // sector bounding box

  Sector();

  // saves the sector to the current file
  void save(Map *m,Cluster *c);
  // loads the sector from the current file
  bool load();
  // unloads the sector (releases all resources allocated by load())
  void unload();
  // initializes the sector after the loading
  void postload(Map *m,Cluster *c);

  // creates the contours from the lines of the sector
  int build_contours(Cont *c);
  // fills polygon fp with the texture of the floor
  void draw_floor(Monotone *mp);
  // fills polygon fp with the texture of the ceiling
  void draw_ceiling(Monotone *mp);
  // returns the height of the floor at point (x,y)
  coord3d getzf(coord3d x,coord3d y);
  // returns the height of the ceiling at point (x,y)
  coord3d getzc(coord3d x,coord3d y);
  // must be called when the height of the floor or ceiling is changed
  void changeheight( void );
  // returns the line with ends v1 and v2
  Line *getline(int v1,int v2);

  // implementation of these functions is in collide.cpp
  // checks if (x,y) is inside the sector (in 2D)
  bool inside(coord3d x,coord3d y);
  // checks if (x,y,z) is inside the sector (in 3D)
  bool inside(coord3d x,coord3d y,coord3d z);
  // checks if the sphere with center (x,y,z) intersects the sector
  int collision(coord3d x,coord3d y,coord3d z);

#ifdef EDITOR
  Sector *next;
#endif
};

#ifndef EDITOR
#define NEXTSECTOR(s) (s)++
#else
#define NEXTSECTOR(s) (s)=(s)->next
#endif


#define MAX_LINES_PER_CONTOUR 20

// contour (used in realtime sorting)
struct Cont
{
  Sector *s; // sector of the contour
  Line *l1;  // first line of the contour
  int n;      // number of the lines in the contour
  coord2d x1,x2;  // minimal and maximal x coordinate of the projection on the screen
  coord3d minz,maxz;  // minimal and maximal z coordinate in camera space
  coord2d x[MAX_LINES_PER_CONTOUR]; // x coordinates of the projection on the screen
};

#endif
