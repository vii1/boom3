#include "bnchmark.h"
#include <float.h>
#include <stdint.h>

using namespace std;

void Benchmark::getResults( Results& outResults )
{
	if( m_data.size() < 2 ) {
		outResults = Results();
		return;
	}
	outResults.time			= ( m_data.back().timestamp - m_data.front().timestamp ) / 1000.f;
	outResults.avg_fps		= 0;
	outResults.avg_msecs	= 0;
	outResults.max_fps		= 0;
	outResults.max_msecs	= 0;
	outResults.min_fps		= FLT_MAX;
	outResults.min_msecs	= INT_MAX;
	Entry*	 fpsWindowStart = m_data.begin();
	unsigned nframes		= 0;
	for( Entry* i = m_data.begin() + 1; i < m_data.end(); ++i ) {
		long dt = i->timestamp - fpsWindowStart->timestamp;
		if( dt >= 500 && i > fpsWindowStart ) {
			float fdt = dt / 1000.f;
			float fps = ( i - fpsWindowStart ) / fdt;
			outResults.avg_fps += fps;
			nframes++;
			if( fps > outResults.max_fps ) outResults.max_fps = fps;
			if( fps < outResults.min_fps ) outResults.min_fps = fps;
			do {
				fpsWindowStart++;
				dt = i->timestamp - fpsWindowStart->timestamp;
			} while( dt >= 500 );
		}
		dt = i->timestamp - ( i - 1 )->timestamp;
		outResults.avg_msecs += dt;
		if( dt > outResults.max_msecs ) outResults.max_msecs = dt;
		if( dt < outResults.min_msecs ) outResults.min_msecs = dt;
	}
	outResults.avg_fps /= nframes;
	outResults.avg_msecs /= m_data.size() - 1;
}

void Benchmark::report( FILE* file )
{
	if( file == NULL ) return;
	Results r;
	getResults( r );
	fprintf( file, "Total time (s): %.3f\n", r.time );
	fprintf( file, "Frame time (ms) Min: %d Avg: %d Max: %d\n", r.min_msecs, r.avg_msecs, r.max_msecs );
	fprintf( file, "Frames per second Min: %.2f Avg: %.2f Max: %.2f\n", r.min_fps, r.avg_fps, r.max_fps );
	fprintf( file, "-------------\n" );
}
