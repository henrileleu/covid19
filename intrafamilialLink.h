#pragma once
#include "link.h"

class intrafamilialLink :
	public link
{
public:
	intrafamilialLink(individuals * i1, int type);
	~intrafamilialLink();
};

