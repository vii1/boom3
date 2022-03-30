#include <stdio.h>
// #include <math.h>
#include <assert.h>
#include "debug.h"

bool       debug_key = false;
mono_print debug;

Demo::Demo() : m_playPos( NULL )
{
}

Demo* Demo::loadFromFile( const char* path )
{
	FILE* f = fopen( path, "rb" );
	if( f == NULL ) return NULL;
	size_t length = 0;
	fread( &length, sizeof( length ), 1, f );
	if( length == 0 ) {
		fclose( f );
		return NULL;
	}
	Demo* demo = new Demo();
	demo->m_frames.resize( length );
	fread( demo->m_frames.begin(), sizeof( DemoFrame ), length, f );
	fclose( f );
	return demo;
}

void Demo::saveToFile( const char* path ) const
{
	if( m_frames.size() == 0 ) return;
	FILE* f = fopen( path, "wb" );
	if( f == NULL ) return;
	size_t length = m_frames.size();
	fwrite( &length, sizeof( length ), 1, f );
	fwrite( m_frames.begin(), sizeof( DemoFrame ), length, f );
	fclose( f );
}

template<typename T>
static inline T lerp( long time1, T value1, long time2, T value2, long timeNew )
{
	return ( time2 - timeNew ) * ( value2 - value1 ) / ( time1 - time2 ) + value2;
}

#define EPS  0.01
#define EPS3 ( EPS * EPS + EPS * EPS + EPS * EPS )
static inline bool linear( long time1, const Tvector& value1, long time2, const Tvector& value2, long timeNew, const Tvector& target )
{
	coord3d dx = lerp( time1, value1.x, time2, value2.x, timeNew ) - target.x;
	coord3d dy = lerp( time1, value1.y, time2, value2.y, timeNew ) - target.y;
	coord3d dz = lerp( time1, value1.z, time2, value2.z, timeNew ) - target.z;
	return ( dx * dx + dy * dy + dz * dz ) < EPS3;
}

static inline bool linear(
  long time1, const angle( value1 )[], long time2, const angle( value2 )[], long timeNew, const angle( target )[] )
{
	angle dx = lerp( time1, value1[0], time2, value2[0], timeNew ) - target[0];
	angle dy = lerp( time1, value1[1], time2, value2[1], timeNew ) - target[1];
	angle dz = lerp( time1, value1[2], time2, value2[2], timeNew ) - target[2];
	return ( dx * dx + dy * dy + dz * dz ) < EPS3;
}

#undef EPS3
#undef EPS

static inline bool linear( const DemoFrame& f1, const DemoFrame& f2, const DemoFrame& last )
{
	return linear( f1.timestamp, f1.viewPos, last.timestamp, last.viewPos, f2.timestamp, f2.viewPos ) &&
	  linear( f1.timestamp, f1.angles, last.timestamp, last.angles, f2.timestamp, f2.angles );
}

void Demo::record( long time, const Tvector& viewPos, const angle ( &angles )[3] )
{
	DemoFrame frame( time, viewPos, angles );
	if( m_frames.size() >= 2 ) {
		// si el penúltimo, último y actual frame son colineales, sustituye el último frame
		const DemoFrame& f2 = *( m_frames.end() - 1 );
		const DemoFrame& f1 = *( m_frames.end() - 2 );
		if( linear( f1, f2, frame ) ) {
			m_frames.assign( m_frames.size() - 1, frame );
			return;
		}
	}
	m_frames.push_back( frame );
}

template<typename T>
static inline void copy3( T* dst, const T* src )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void Demo::play( long timestamp, Tvector* viewPosOut, angle anglesOut[3] )
{
	assert( viewPosOut != NULL );
	if( m_frames.size() == 0 ) return;
	if( m_playPos == NULL ) m_playPos = m_frames.begin();
	while( m_playPos < m_frames.end() && m_playPos->timestamp < timestamp ) m_playPos++;
	if( m_playPos >= m_frames.end() ) {
		const DemoFrame& frame = m_frames.back();
		*viewPosOut            = frame.viewPos;
		copy3( anglesOut, frame.angles );
	} else if( m_playPos <= m_frames.begin() ) {
		const DemoFrame& frame = m_frames.front();
		*viewPosOut            = frame.viewPos;
		copy3( anglesOut, frame.angles );
	} else if( m_playPos->timestamp > timestamp ) {
		const DemoFrame* prev = m_playPos - 1;
		viewPosOut->x = lerp(prev->timestamp, prev->viewPos.x, m_playPos->timestamp, m_playPos->viewPos.x, timestamp);
	} else {   // m_playPos->timestamp == timestamp
		*viewPosOut = m_playPos->viewPos;
		copy3( anglesOut, m_playPos->angles );
	}
}
