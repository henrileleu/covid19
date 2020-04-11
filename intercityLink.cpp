#include "pch.h"
#include "intercityLink.h"


intercityLink::intercityLink(individuals * i1)
	:link(i1, 720, 1, lIntercity)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


intercityLink::~intercityLink()
{
}



