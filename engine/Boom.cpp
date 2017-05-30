#include "Boom.h"
#include "Read.h"
#include "Texture.h"
#include "Player.h"
#include "Trap.h"

namespace boom {

	// loads a map from file fn
	bool Boom::map_init( char *fn )
	{
		verts = nullptr;
		vertsnum = 0;
		map_done();
		map = new Map();
		if( fn ) {
			if( !read_start( fn ) || !map->load() ) {
				read_end();
				map_done();
				return false;
			}
			loadtextures();
			read_end();
		}
		cur_cluster = nullptr;
		player_init( map->start_x, map->start_y, map->start_z );
		return true;
	}

	// frees all allocated resources
	void Boom::map_done()
	{
		if( map ) {
			map->unload();
			delete map;
			map = nullptr;
		}
	}

	// draws the map on the screen
	bool Boom::map_draw()
	{
		if( !cur_cluster ) return false;
		if( scr_ox <= 0 || scr_oy <= 0 ) return true;
		map->draw();
		return true;
	}

	// deletes a linked list of Ttraps
	void Boom::deltraps( Trap *t )
	{
		Trap *n;
		for( ; t; t = n ) {
			n = t->next;
			delete t;
		}
	}

	//Set the current clip. All stuff that are drawn on the screen are first
	//clipped with the current clip.
	void Boom::set_cur_clip( Clip *clip )
	{
		cur_clip = clip;
	}

} // namespace boom
