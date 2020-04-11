#include "pch.h"
#include "link.h"

link::link()
{
}

// Default constructor
link::link(individuals * i1, int duration, int distance, int type) :
	_active(true), _risk(0), _type(type), _distance(distance), _duration(duration), _frequency({ 1,1,1,1,1,1,1 }),
	_pair(i1), _simTime(-1), _contaminated(false)
{
}


link::~link()
{
}

individuals * link::connected(int simTime, int f) const
{
	if (_contaminated || _frequency[f] == 0 || !_active ) return nullptr;

	return _pair;

}

individuals * link::contactBetween(int s, int e) const
{
	if (!_active) return nullptr;
	if (_simTime == -1) // Regular contacts
	{
		for (int i = s; i < e; i++)
		{
			if (_frequency[(i % 7)] == 1) return _pair;
		}
		return nullptr;
	}
	else if (s <= _simTime && _simTime <= e) return _pair; // OneTime
	return nullptr;
}

bool link::ChangeActiveState()
{
	return ChangeActiveState(!_active);
}

int link::ChangeActiveState(bool a)
{
	return _active = a;
}

int link::ChangeActiveState(bool b, int a)
{
	if (_type == a) return ChangeActiveState(b);
	return _active;
}

std::array<int, 2> link::getProperties() const
{
	return std::array<int, 2>({ _distance , _duration });
}

int link::getType() const
{
	return _type;
}

int link::getTime() const
{
	return _simTime;
}

void link::setSimTime(int t)
{
	_simTime = t;
}

individuals * link::getPair() const
{
	return _active?_pair:nullptr;
}

void link::contaminated()
{
	_contaminated = true;
}
