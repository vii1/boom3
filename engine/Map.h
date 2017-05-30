//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Map.h - class Map interface                                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOM_MAP_H
#define _BOOM_MAP_H

#define MAP_VERSION 10
#define MAP_ID "MOOB"

// maximum number of vertices per map (only when EDITOR is defined)
#define MAX_VERTICES 10000

namespace boom {

	// Map contains all the information for one level
	class Map
	{
	public:
		Cluster *clusters; // clusters in the map
		int clustersnum;    // number of the clusters in the map
		coord3d start_x, start_y, start_z; // start position

		Map( Boom& b );
		// saves the map to the current file
		void save();
		// loads the map from the current file
		bool load();
		// unloads the map (releases all resources allocated by load())
		void unload();
		// initializes all items in the map after the loading
		void postload( void );
		// returns the address of the cluster with number "target"
		Cluster *getcluster( int target );

		// draws the map from the current point of view
		void draw();

		// implementation of these functions is in collide.cpp
		// moves the current point of view with the vector (dx,dy,dz) and sets the
		// view angles to ax,ay and az
	  // returns true if successful
		bool move( coord3d dx, coord3d dy, coord3d dz, angle ax, angle ay, angle az );
		// checks for collision with the map. returns the new z value in *z
		bool collision( coord3d x, coord3d y, coord3d *z );
	private:
		Boom& m_boom;
	};

}
#endif // _BOOM_MAP_H_
