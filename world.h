#pragma once

class world
{
public:


	// **** Functions :			******** //
	// Init with size, number of individuals & densities a, b
	world(int size, long int n, Parameters & p);
	~world();

	// Get functions
	std::vector<location> * getLocations();

private:

	// **** Variables :			******** //
	// **** - Size : size of the grid to put the cities (int)
	// **** - N : Size of the total population (long long int)
	// **** - List is the list of places included in the world
	// ********************************* //
	int _size;
	long long int _n;
	std::vector<location> _list;
	Parameters _p;

	// Random number generators
	int nThreads;
	std::vector<VSLStreamStatePtr> stream;
	std::vector<vlsRandGenerator> RandomGenerator;
	

	const std::array<std::array<int, 2>, 8> neighborPositions =
	{ {{ -1,-1 }, { -1,0 }, { -1,1 }, { 0,1 }, { 1,1 }, { 1,0 }, { 1,-1 }, { 0,-1 }} };

	// ******************************
	// Private functions
	//
	// ******************************


	int getPosition(std::array<int, 2>);

	// Storage for Links
	std::vector<link> _storage;

};

