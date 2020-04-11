#pragma once
class family
{
public:
	family();
	family(int x, int y, vlsRandGenerator & rnd, const Parameters & p);
	~family();

	// *** Get functions *****
	std::vector<individuals *> getIndividuals();
	std::vector<individuals *> getIndividuals(int); 
	individuals * getHead();
	std::array<int, 2> getPosition() const;
	
	bool someIsSick(int) const;
	int getType() const;
	std::vector<std::array<int, 2>> caracteristics() const;

	// **** Function to created intrafamilial links *******
	bool linkCreated();
	void createLink();

	// ******** Friends **************
	unsigned int getSocialReach();
	void saveFriends(std::vector<family *>);
	bool addFriend(family *, bool);
	int getNumberofFriends() const;
	int getTotalFriends() const;
	tbb::concurrent_vector<family *> getFriends() const;
	int getSize() const;
private:
	// **** Variables :			******** //
	// **** - Size : Number of families in the city int)
	// **** - Individuals : array of class family
	// **** - position : position in the city (int, int)
	// ********************************* //
	size_t _size;
	std::vector<individuals> _members;
	std::array<int, 2> _position;
	tbb::concurrent_vector<family *> _friends;
	unsigned int nFriends;
	int _type;
	unsigned int _socialReach;
	bool _isLinked;

	// Type of families
	enum familyTypeList :int
	{
		Single = 1,
		CoupleOnly = 2,
		CoupleWithChildren = 3,
		SingleParent = 4,
		Extended = 5

	};
};

