#ifndef STRUCTS_H
#define STRUCTS_H

#include "Exam_HelperStructs.h"

struct DiscoveredHouse
{
	HouseInfo m_HouseInfo;
	float m_TimeSinceVisit;
	bool m_IsVisited;
};

#endif
