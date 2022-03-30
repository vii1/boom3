#ifndef __DEMO_H__
#define __DEMO_H__

#include <vector>
#include "engine/Types.h"

struct DemoFrame {
	long timestamp;
	Tvector viewPos;
	angle angles[3];

	DemoFrame() {}
	DemoFrame(long timestamp, const Tvector& viewPos, const angle (&angles)[3]) {
		this->timestamp = timestamp;
		this->viewPos = viewPos;
		this->angles[0] = angles[0];
		this->angles[1] = angles[1];
		this->angles[2] = angles[2];
	}
};

class Demo {
public:
	Demo();
	static Demo* loadFromFile(const char* path);
	void saveToFile(const char* path) const;
	void record(long timestamp, const Tvector& viewPos, const angle (&angles)[3]);
	bool play(long timestamp, Tvector* viewPosOut, angle anglesOut[3]);
private:
	std::vector<DemoFrame>::const_iterator m_playPos;
	std::vector<DemoFrame> m_frames;
};

#endif // __DEMO_H__
