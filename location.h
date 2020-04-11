#pragma once

class location
{
public:

	

	// **** Functions :			******** //


	// Constructor
	// Arg: Size, density, x, y [for _location]
	location(long int size, float density, int x, int y, bool publicTransportation, 
		std::vector<vlsRandGenerator> & RandomGenerator, int nThreads, const Parameters & p);
	~location();

	// setNeighbor: Reference to a location class, and position 1-4 [N, NE, E, SE, S, SW, W, NW]
	void setNeighbor(location * l, int position);

	// Get functions
	tbb::concurrent_vector<family> * getFamilies();
	float getAvgDistance();
	long int getSize();
	int getPopSize() const;
	std::array<int, 2> getLocation();
	individuals * getHead(unsigned int);
	std::vector<individuals *> getInfected();
	std::array < std::array<int, max_days>, 200> getTracking(int) const;
	float getNumberDiagnosed(int simTime) const;

	// Infections
	std::vector<individuals *> incidence(individuals *, int, vlsRandGenerator & rnd, const Parameters & p);
	void addNewlyInfected(std::vector<individuals *>, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void nextDay(int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void trackCases(int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void disableTracking();
	void enableTracking(int);
	void containement(int age, int link);
	void setSelfTesting(bool);
	void setTrackingCapability(int, bool);
	void printStuff();
	void setSocialDistancing(bool);
	void serologyTesting(int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);

private:
	// **** Variables :			******** //
	// **** - Size : Number of families in the city (Big int)
	// **** - Density : Average distance between families (float)
	// **** - Location : Position in a 2-dim array (int, int)
	// **** - Neighbours : City on its boarder array(ref) 
	// **** - Public Transport : Yes/No
	// **** - Links : a vector of a the links in the city
	// ********************************* //
	long int _size;
	float _density;
	std::array<int, 2> _location;
	std::array<location *, 8> _neigbours;
	bool _publicTransportation;
	tbb::concurrent_vector<family> _families;
	int population;

	// Keeping track
	std::vector<individuals *> _infected;
	std::array < std::array<int, max_days>, 200> _incidence;
	std::array < std::array<int, max_days>, 200> _diagnosed;
	std::array < std::array<int, max_days>, 200> _hospitalized;
	std::array < std::array<int, max_days>, 200> _ICU;
	std::array < std::array<int, max_days>, 200> _needICU;
	std::array<int, max_days> _ICUAvailable;
	std::array < std::array<int, max_days>, 200> _cured;
	std::array < std::array<int, max_days>, 200> _dead;
	

	// Variables for number of available hospital beds and ICU beds
	int hospBeds;
	int ICUBeds;

	// Info to create links
	long int  _nIndividuals;
	std::vector<individuals *> _employables;
	int _schools;
	int _shopping;

	// Flags for isolation status
	std::array<int, nLinks> _IsolationFlags;
	std::array<bool, nLinks> _CanTrack;
	bool trackingCases;
	bool testOnlySymptomatic;
	size_t trackPosition;
	bool selfTestIfSymptomatic;
	bool socialDistanceFlag;

	// For testing
	struct linkedIndividuals
	{
		individuals * ind;
		individuals * next;
		individuals * prev;
	};
	std::vector<linkedIndividuals> serologyScreened;
	void initSerologyScreened(int, int);

	// Utilities
	int estimateSectorSize(float a);
	int distance(int, int, int, int);

	void createSchoolLink(individuals *, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void createShoopingLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void createWorkLink(individuals * ind, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void createTransportLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void createFriendLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
	void createEventLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);


	int nThreads;

	void createPopulation(std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p);
};

