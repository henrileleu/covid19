#include "pch.h"
#include "eventLink.h"


eventLink::eventLink(individuals * i1)
	:link(i1, 120, 2, lEvent)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


eventLink::~eventLink()
{
}
