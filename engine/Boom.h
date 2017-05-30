//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   boom.h - includes all class definitions in the correct order       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOM_H
#define _BOOM_H

// define EDITOR to use linked list structures instead of arrays
// define DIRECT_3D for direct 3d projects
namespace boom {
	class Map;
	class Cluster;
	class Sector;
	class Line;
	class Wall;
	class Hole;
	class Portal;
}

#include "Mapitem.h"
#include "Map.h"
#include "Cluster.h"
#include "Sector.h"
#include "Line.h"
#include "Wall.h"
#include "Hole.h"
#include "Portal.h"

namespace boom {
	class Clip;
	class Trap;

	// I'm adding this class to hold all the formerly global objects. - vii
	class Boom
	{
	public:
		// From Map.h //

		// the sector containing the view point
		Sector *cur_sector;
		// the cluster containing the current sector. the drawing always begins from it
		Cluster *cur_cluster;
		// current map
		Map *map;
		// array with the vertices in the current map
		Tvertex *verts;
		// number of the vertices in the current map
		int vertsnum;

		// loads a map from file fn
		bool map_init( char *fn );
		// frees all allocated resources
		void map_done( void );
		// draws the map on the screen
		bool map_draw( void );

		// From Clip.h //

		Clip *cur_clip; //everything is clipped by the current clip
		coord2d gymin, gymax;

		//Set the current clip. All polygons that are drawn on the screen are first
		//clipped with the current clip.
		void set_cur_clip( Clip *clip );

		// deletes a linked list of Ttraps
		void deltraps( Trap *t );

		// From Cluster.h //

		int MIN_LX, MAX_LX;

		// From Collide.h //

		coord3d zfloor, zceiling; // current floor and ceiling height


	};
} // namespace boom

#endif
