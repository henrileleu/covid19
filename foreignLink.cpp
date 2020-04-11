#include "pch.h"
#include "foreignLink.h"


foreignLink::foreignLink(individuals * i1) :link(i1, 36000000, 1, lForeign)
{
	_frequency = { 1,1,1,1,1,1,1 };

}


foreignLink::~foreignLink()
{
}
