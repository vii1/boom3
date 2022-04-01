#ifndef __BNCHMARK_H__
#define __BNCHMARK_H__

#include <vector>
#include <cstdio>

class Benchmark
{
public:
	struct Results {
		float time;
		long  min_msecs, max_msecs, avg_msecs;
		float min_fps, max_fps, avg_fps;
	};
	void add( long timestamp ) { m_data.push_back( Entry( timestamp ) ); }
	void getResults( Results& outResults );
	void report( std::FILE* file = stdout );

private:
	struct Entry {
		Entry() {}
		Entry( long timestamp ) : timestamp( timestamp ) {}
		long timestamp;
	};
	std::vector<Entry> m_data;
};

#endif	 // __BNCHMARK_H__
