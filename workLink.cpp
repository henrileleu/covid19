#include "pch.h"
#include "workLink.h"


workLink::workLink(individuals * i1)
	:link(i1, 480, 2, lWork)
{
	_frequency = { 1,1,1,1,1,0,0 };
}


workLink::~workLink()
{
}
