#pragma once
#include "link.h"
class transportLink :
	public link
{
public:
	transportLink(individuals * i1, int);
	~transportLink();
};

