#include "pch.h"
#include "workTransportLink.h"


workTransportLink::workTransportLink(individuals * i1):
	transportLink(i1, lWorkTransport)
{
	_frequency = { 1,1,1,1,1,0,0 };
}


workTransportLink::~workTransportLink()
{
}
