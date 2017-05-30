//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Tmapitem.cpp - class MapItem implementation                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _MAPITEM_CPP
#include "MapItem.h"
#include "read.h"
#include "write.h"

namespace boom {

	// saves the map item to the current file
	void MapItem::save()
	{
		wrchar( options );
	}

	// loads the map item from the current file
	bool MapItem::load()
	{
		options = rdchar();
		return true;
	}

	//marks the map item as 'dirty'
	void MapItem::dirty()
	{
		options |= kOptionDirty;
	}
}
