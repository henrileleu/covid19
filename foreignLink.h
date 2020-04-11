#pragma once
#include "link.h"

// *************** foreign Link *****************
// This a link to a special individual that represent the probability of an 
// individual being contaminated from outside the country
class foreignLink :
	public link
{
public:
	foreignLink(individuals * i1);
	~foreignLink();
};

