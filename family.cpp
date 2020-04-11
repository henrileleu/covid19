#include "pch.h"
#include "family.h"


family::family()
{
}

family::family(int x, int y, vlsRandGenerator & rnd, const Parameters & p): _position({x,y}), nFriends(0), _isLinked(false)
{
	float r = rnd();
	int type(0);

	// First define family type
	for (int i = 0; i < 5; ++i)
	{
		r -= p(familyType, i);
		if (r <= 0)
		{
			type = i+1;
			break;
		}
	}

	int age(0);
	int ageChild(0);
	int sex(0);

	// number of friends
	_socialReach = rnd.poisson(p(AvgFriends));

	// For each type of families generate individuals
	int numChildren(0);
	_type = type;
	std::vector<int> childrenAges;
	int ageNextChild;
	switch (type)
	{
	case Single:

		sex = rnd() > p(familySingleMale, 6) ? 1 : 0;
		age = p.getPrecalculated(sex == 0 ? familySingleMale : familySingleFemale, rnd());
		_members.emplace_back(age, sex, rnd, p, this);

		break;

	case CoupleOnly:

		age = p.getPrecalculated(familyCoupleOnly, rnd());
		_members.emplace_back(age, 0, rnd, p, this);
		age = static_cast<int>(age - rnd.normal(p(AvgCoupleDistance, 0), p(AvgCoupleDistance, 1)));
		_members.emplace_back(age, 1, rnd, p, this);

		break;

	case CoupleWithChildren:
	case SingleParent:

		// First calculated the age of the first child as everything will be based on this
		ageChild = static_cast<int>(rnd() * p(familyUnder20, 1));
		childrenAges.push_back(ageChild);

		// Number of children
		numChildren = rnd.poisson(p(nChildren));
		
		for (int i = 0; i < numChildren - 1; ++i)
		{
			ageNextChild = static_cast<int>(rnd.normal(p(AvgSiblingDistance, 0), p(AvgSiblingDistance, 1)));
			ageChild = ageNextChild > 0 ? ageChild + ageNextChild : ageNextChild; // Keep min value
			childrenAges.push_back(childrenAges[0] + ageNextChild);
		}

		// Parents
		age = static_cast<int>(ageChild + rnd.normal(p(AvgAgeFirstChild, 0), p(AvgAgeFirstChild, 1)));
		_members.emplace_back(age, type== CoupleWithChildren ? 1 : (rnd() < 0.65 ? 0 : 1), rnd, p, this);

		if (type == CoupleWithChildren)
		{
			age = static_cast<int>(age + rnd.normal(p(AvgCoupleDistance, 0), p(AvgCoupleDistance, 1)));
			_members.emplace_back(age, 0, rnd, p, this);
		}

		// Childrens
		for (size_t i = 0; i < childrenAges.size(); ++i)
		{
			_members.emplace_back(childrenAges[i]<0?0: childrenAges[i], rnd() < 0.5 ? 1 : 0, rnd, p, this);
		}
		
		
		break;

	case Extended:

		// To be improved...
		// For now, base is 1 adult (female) + 1 child + 1 grand parent
		// First calculated the age of the first child as everything will be based on this
		ageChild = static_cast<int>(rnd() * p(familyUnder20, 1));
		childrenAges.push_back(ageChild);

		// Parents
		age = static_cast<int>(ageChild + rnd.normal(p(AvgAgeFirstChild, 0), p(AvgAgeFirstChild, 1)));
		_members.emplace_back(age, 1, rnd, p, this);

		// Children
		sex = rnd() < 0.5 ? 1 : 0;
		_members.emplace_back(ageChild, sex, rnd, p, this);

		// Grand Parents
		_members.emplace_back(static_cast<int>(age + rnd.normal(p(AvgAgeFirstChild, 0), p(AvgAgeFirstChild, 1))), rnd() < 0.7 ? 1 : 0, rnd, p, this);

		// Check how many additional members
		int nOtherMembers = rnd.poisson(p(AverageHouseHoldExtended));
		// Number of children
		numChildren = rnd.poisson(p(nChildren));
		
		for (int i = 0; i < numChildren - 1; ++i)
		{
			ageNextChild = static_cast<int>(rnd.normal(p(AvgSiblingDistance, 0), p(AvgSiblingDistance, 1)));
			ageChild = ageNextChild > 0 ? ageChild + ageNextChild : ageNextChild; // Keep min value
			childrenAges.push_back(childrenAges[0] + ageNextChild);
		}

		nOtherMembers -= numChildren;

		if (nOtherMembers > 0)
		{
			_members.emplace_back(static_cast<int>(age + rnd.normal(p(AvgCoupleDistance, 0), p(AvgCoupleDistance, 1))), 0, rnd, p, this);
		}

		nOtherMembers--;

		if (nOtherMembers > 0)
		{
			for (int i = 1; i < nOtherMembers; ++i) _members.emplace_back(static_cast<int>(age + rnd.normal(p(AvgAgeFirstChild, 0), p(AvgAgeFirstChild, 1))), rnd() < 0.5 ? 1 : 0, rnd, p, this);
		}

		break;

	}

	_size = _members.size();

}


family::~family()
{
}

std::vector<individuals *> family::getIndividuals()
{
	std::vector<individuals*> r;
	for (size_t i = 0; i < _size; ++i)
	{
		r.push_back(&(_members[i]));
	}
	return r;
}

std::vector<individuals *> family::getIndividuals(int a)
{
	std::vector<individuals *> r;
	for (size_t i = 0; i < _size; ++i)
	{
		if (_members[i].getAge() == a) r.push_back(&(_members[i]));
	}
	return r;
}

individuals * family::getHead()
{
	return &(_members[0]);
}

std::array<int, 2> family::getPosition() const
{
	return _position;
}

bool family::someIsSick(int t) const
{
	for (size_t i = 0; i < _members.size(); ++i)
	{
		if (_members[i].isInfected(t)) return true;
	}
	return false;
}

int family::getType() const
{
	return _type;
}

std::vector<std::array<int, 2>> family::caracteristics() const
{
	std::vector<std::array<int, 2>> r;
	for (int i = 0; i < _members.size(); i++)
	{
		r.push_back(_members[i].getCaracteristics());
	}
	return r;
}

bool family::linkCreated()
{
	return _isLinked;
}

void family::createLink()
{
	// set size
	_size = _members.size();


	// Family members created let's link them up
	for (size_t i = 0; i < _size; ++i)
	{
		for (size_t j = i + 1; j < _size; ++j)
		{

			link l1 = sameRoofIntrafamilialLink(&_members[j]);
			_members[i].addLink(l1);
			link l2 = sameRoofIntrafamilialLink(&_members[i]);
			_members[j].addLink(l2);

		}
	}

	_isLinked = true;
}

unsigned int family::getSocialReach()
{
	return _socialReach;
}

void family::saveFriends(std::vector<family*> f )
{
	for (std::vector<family*>::iterator i = f.begin(); i < f.end(); i++) _friends.push_back(*i);
}

bool family::addFriend(family * f, bool forced)
{
	if (_friends.size() < nFriends || forced == true) { _friends.push_back(f); return true; }
	return false;
}

int family::getNumberofFriends() const
{
	return nFriends - static_cast<int>(_friends.size());
}

int family::getTotalFriends() const
{
	return  static_cast<int>(_friends.size());
}

tbb::concurrent_vector<family*> family::getFriends() const
{
	return _friends;
}

int family::getSize() const
{
	return static_cast<int>(_members.size());
}

