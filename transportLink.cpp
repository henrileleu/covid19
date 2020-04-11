#include "pch.h"
#include "transportLink.h"


transportLink::transportLink(individuals * i1, int type)
	:link(i1, 41, 2, type) // Assumption based on Paris////41NYC Census
{
}


transportLink::~transportLink()
{
}
