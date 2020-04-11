#include "pch.h"
#include "friendLink.h"


friendLink::friendLink(individuals * i1)
	:link(i1, 180, 1, lFriend)
{
		_frequency = { 1,1,1,1,1,1,1 };

}



friendLink::~friendLink()
{
}
