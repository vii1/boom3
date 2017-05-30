//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Map.cpp - class Map implementation                               //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//#define _TMAP_CPP
#include "boom.h"
#include "geom.h"
#include "draw.h"
#include "clip.h"
#include "read.h"
#include "write.h"
#include "player.h"

//standard includes
#include <cassert>
#include <cstdio>
#include <cstring>

namespace boom {

	Map::Map(Boom& b)
		: m_boom(b)
	{
		clusters = NULL;
		clustersnum = 0;
		start_x = 0;
		start_y = 0;
		start_z = 0;
	};

	// saves the map to the current file
	void Map::save()
	{
		// writes the map file identifier
		wrbuf( MAP_ID, 4 );
		wrlong( MAP_VERSION );

		// writes the start position
		wrfloat( start_x );
		wrfloat( start_y );
		wrfloat( start_z );

		// writes all the clusters
		wrlong( clustersnum );
		Cluster *c = clusters;
		int i;
		for( i = 0; i < clustersnum; i++, NEXTCLUSTER( c ) )
			c->save( this );

		// writes all the vertices
		wrlong( m_boom.vertsnum );
		for( i = 0; i < m_boom.vertsnum; i++ ) {
			wrfloat( m_boom.verts[i].x );
			wrfloat( m_boom.verts[i].y );
		}
	}

	// loads the map from a file
	bool Map::load()
	{
		unload();
		// reads the map identifier
		char magic[4];
		rdbuf( magic, 4 );
		if( strncmp( magic, MAP_ID, 4) != 0 ) return false;
		if( rdlong() != MAP_VERSION ) return false;

		// reads the start position
		start_x = rdfloat();
		start_y = rdfloat();
		start_z = rdfloat();

		// reads all the clusters
		int n = rdlong();
		if( n ) {
			CONSTRUCT_AND_LOAD_ARRAY( clusters, Cluster, n );
		}
		clustersnum = n;

		// reads all the vertices
		m_boom.vertsnum = rdlong();
	#ifndef EDITOR
		m_boom.verts = (Vertex *)malloc( m_boom.vertsnum * sizeof( Vertex ) );
	#else
		m_boom.verts = (Vertex *)malloc( MAX_VERTICES * sizeof( Vertex ) );
	#endif
		if( !m_boom.verts ) return false;
		int i;
		for( i = 0; i < m_boom.vertsnum; i++ ) {
			m_boom.verts[i].x = rdfloat();
			m_boom.verts[i].y = rdfloat();
		}

		// initializes all items in the map
		postload();
		return true;
	}

	// initializes all items in the map after the loading
	void Map::postload( void )
	{
		// calls postload() for all clusters
		Cluster *c = clusters;
		for( int i = 0; i < clustersnum; i++, NEXTCLUSTER( c ) )
			c->postload( this );
	}

	// unloads the map (releases all resources allocated by load())
	void Map::unload()
	{
		// unloads all the clusters
		if( clusters ) {
			Cluster *c = clusters;
			for( int i = 0; i < clustersnum; i++, NEXTCLUSTER( c ) )
				c->unload();
			DESTRUCT_ARRAY( clusters );
		}

		// frees the memory for the vertices
		if( m_boom.verts ) free( m_boom.verts );
		m_boom.verts = NULL;
	}

	// returns the address of the cluster with number "target"
	Cluster *Map::getcluster( int target )
	{
	#ifndef EDITOR
		// in case of array
		return clusters + target;
	#else
		// in case of linked list
		Cluster *c = clusters;
		for( int i = 0; i < target; i++ )
			c = c->next;
		return c;
	#endif
	}

	// calculates the projection of the screen on a vertical plane
	// the result is a list of trapezoids
	static void makeclip( Clip *c )
	{
		coord2d vx[4], vy[4];
		int u, d, u1, d1;

		rotatexyp( -scr_ox, scr_foc, scr_oy, vx + 0, vy + 0 );
		rotatexyp( scr_ox - 1, scr_foc, scr_oy, vx + 1, vy + 1 );
		rotatexyp( scr_ox - 1, scr_foc, -scr_oy + 1, vx + 2, vy + 2 );
		rotatexyp( -scr_ox, scr_foc, -scr_oy + 1, vx + 3, vy + 3 );
		coord2d xmin, xmax, ymin, ymax;
		xmin = xmax = vx[0];
		ymin = ymax = vy[0];
		u = 0;
		int un = csegment++;
		int dn = csegment++;
		for( int i = 1; i < 4; i++ ) {
			if( xmin > vx[i] ) xmin = vx[i], u = i;
			if( xmax < vx[i] ) xmax = vx[i];
			if( ymin > vy[i] ) ymin = vy[i];
			if( ymax < vy[i] ) ymax = vy[i];
		}
		c->xmin = xmin;
		c->xmax = xmax;
		c->ymin = ymin;
		c->ymax = ymax;
		c->traps = NULL;
		Trap **t = &(c->traps);
		d = u;
		u1 = (u + 1) & 3;
		while( vx[u1] == vx[u] ) {
			u = u1;
			u1 = (u + 1) & 3;
		}
		d1 = (d + 3) & 3;
		while( vx[d1] == vx[d] ) {
			d = d1;
			d1 = (d + 3) & 3;
		}
		coord2d x1 = xmin;
		coord2d y11 = vy[u];
		coord2d y12 = vy[d];
		coord2d y21 = vy[u1];
		coord2d y22 = vy[d1];
		while( x1 < xmax ) {
			if( vx[u1] == vx[d1] ) {
				x1 = vx[u1];
				*t = new Trap( x1, y11, y12, y21, y22, un, dn, NULL );
				t = &((*t)->next);
				y11 = y21;
				y12 = y22;
				u = u1;
				u1 = (u + 1) & 3;
				d = d1;
				d1 = (d + 3) & 3;
				y21 = vy[u1];
				y22 = vy[d1];
				un = csegment++;
				dn = csegment++;
			} else if( vx[u1] < vx[d1] ) {
				coord2d y = splitab( y12, y22, vx[u1] - x1, vx[d1] - vx[u1] );
				x1 = vx[u1];
				*t = new Trap( x1, y11, y12, y21, y, un, dn, NULL );
				t = &((*t)->next);
				y11 = y21;
				y12 = y;
				u = u1;
				u1 = (u + 1) & 3;
				y21 = vy[u1];
				un = csegment++;
			} else { //vx[u1]>vx[d1]
				coord2d y = splitab( y11, y21, vx[d1] - x1, vx[u1] - vx[d1] );
				x1 = vx[d1];
				*t = new Trap( x1, y11, y12, y, y22, un, dn, NULL );
				t = &((*t)->next);
				y11 = y;
				y12 = y22;
				d = d1;
				d1 = (d + 3) & 3;
				y22 = vy[d1];
				dn = csegment++;
			}
		}
	}

	// draws the map from the current point of view
	void Map::draw( void )
	{
		if( cur_cluster ) {
			// initializes the current clip
			Clip c;
			csegment = 1;
			makeclip( &c );
			set_cur_clip( &c );
			gymax = c.ymax;
			gymin = c.ymin;
			MIN_LX = c.xmin;
			MAX_LX = c.xmax;

			// initializes all clusters
			int i;
			Cluster *cl;
			for( i = 0, cl = clusters; i < clustersnum; i++, NEXTCLUSTER( cl ) )
				cl->reset_contours();
			coord3d xx = -100 * sinz;
			coord3d yy = 100 * cosz;
			portx1 = view.x - yy;
			porty1 = view.y + xx;
			portdx = 2 * yy;
			portdy = -2 * xx;

			// draw the current cluster
			cur_cluster->draw();
			// frees the memory allocated for the current clip
			deltraps( c.traps );
		}
	}

} // namespace boom
