#include "pch.h"
#include "schoolLink.h"


schoolLink::schoolLink(individuals * i1)
	:link(i1, 360, 2, lSchool)
{
	_frequency = { 1,1,1,1,1,0,0 };
}


schoolLink::~schoolLink()
{
}
