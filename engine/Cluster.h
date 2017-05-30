//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Cluster.h - class Cluster interface                              //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOM_CLUSTER_H_
#define _BOOM_CLUSTER_H_

namespace boom {
	struct Cont;

	// Cluster contains all the information for one cluster in the map
	class Cluster : public MapItem
	{
	protected:
		Cont *cont;  // contours with the visible lines
		int contn;  // number of the contours
		int conti;  // index of the current contour

	public:
		static const int kOptionVisited = 0x02;  // the cluster is visited during the drawing the current frame

		Sector *sectors; // sectors in the cluster
		int sectorsnum;   // number of the sectors in the cluster

		Cluster();
		~Cluster();

		// saves the cluster to the current file
		void save( Map *m );
		// loads the cluster from the current file
		bool load();
		// unloads the cluster (releases all resources allocated by load())
		void unload();
		// initializes the cluster after the loading
		void postload( Map *m );

		// sorts the contours in the cluster
		void sort_contours( int n );
		// removes the contours after the drawing is complete
		void reset_contours();
		// draws the current cluster
		void draw();
		// checks if the point (x,y) is inside the cluster
		Sector *inside( coord3d x, coord3d y );

	#ifdef EDITOR
		Cluster *next;
		bool visible;
	#endif
	};

#ifndef EDITOR
#define NEXTCLUSTER(c) (c)++
#else
#define NEXTCLUSTER(c) (c)=(c)->next
#endif

}

#endif
