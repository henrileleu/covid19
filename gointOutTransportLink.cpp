#include "pch.h"
#include "gointOutTransportLink.h"


gointOutTransportLink::gointOutTransportLink(individuals * i1) :
	transportLink(i1, lGoingOutTransport)
{
	_frequency = { 1,1,1,1,1,1,1 };
}


gointOutTransportLink::~gointOutTransportLink()
{
}
