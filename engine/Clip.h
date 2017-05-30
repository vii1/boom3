//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   Clip.h - class Clip interface                                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOM_CLIP_H_
#define _BOOM_CLIP_H_

#include "types.h"
#include "Trap.h"

namespace boom {

	// Clip is a Monotone that can clip trapezoids with itself  
	class Clip : public Monotone
	{
	public:
		Trap *beg, *end, **lbeg, *last;

		void cut( coord2d x1, coord2d x2 );
		void restore( Trap *t );
		void clip( coord2d x1, coord2d x2, coord2d y11, coord2d y12, coord2d y21, coord2d y22, Monotone *poly );
		void addbeg( Monotone *poly );
		void addend( Monotone *poly );
	};

} // namespace boom

	// splits [a1,a2] with ratio b1:b2
#define splitab(a1,a2,b1,b2) (((a1)*(b2)+(a2)*(b1))/((b1)+(b2)))


#endif
