#include "pch.h"
#include "SameRoofIntrafamilialLink.h"


sameRoofIntrafamilialLink::sameRoofIntrafamilialLink(individuals * i1):
	intrafamilialLink(i1, lIntrafamilialHousehold)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


sameRoofIntrafamilialLink::~sameRoofIntrafamilialLink()
{
}
