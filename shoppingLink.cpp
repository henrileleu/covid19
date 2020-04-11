#include "pch.h"
#include "shoppingLink.h"


shoppingLink::shoppingLink(individuals * i1)
	:link(i1, 10, 1, lShooping)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


shoppingLink::~shoppingLink()
{
}
