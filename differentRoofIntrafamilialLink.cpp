#include "pch.h"
#include "differentRoofIntrafamilialLink.h"


differentRoofIntrafamilialLink::differentRoofIntrafamilialLink(individuals * i1) :
	intrafamilialLink(i1, lIntrafamilialExterior)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


differentRoofIntrafamilialLink::~differentRoofIntrafamilialLink()
{
}

