#pragma once

// Foward declaration
class link;
class family;

class individuals
{
public:
	individuals();
	individuals(int age, int sex, vlsRandGenerator & rnd, const Parameters & p, family *);
	~individuals();

	// *************** Link Functions ********************* //
	void addLink(link l);
	void addLink(link l, int simeTime);
	void addLink(std::vector<link>);
	bool hasLink(int);

	// ***************  Get functions ********************* //
	// Get ind caracteristics
	int getAge() const;
	int getBiologicalAge() const;
	int getSex() const;
	std::array < std::array<int, 2>, 4> getTracking() const;

	std::array<int, 2> getCaracteristics() const;
	int getNumberofComorbidities() const;
	// Get family
	family * getFamily();

	// ************** Infection related functions **************** //
	// Contaminted is used if a individual is contaminated in order to "infect" him
	void contaminated(vlsRandGenerator & rnd, const Parameters & p, int t, float reducedDiagnosis);
	void contaminated(vlsRandGenerator & rnd, const Parameters & p, int t);
	// Contaminates is run for each contaminated individual on each link to see if they contaminates someone new
	std::vector<individuals *> contaminates(int simTime, vlsRandGenerator & rnd, const Parameters & p, std::array<int, nLinks>, bool socialDistanceFlag);
	std::array<int, 2> getContaminationWindow() const;
	bool isInfected(int t) const;
	bool wasInfected(int i) const;
	void setPatientZero();
	int infectionTime() const;
	void noICU(int);
	bool isSymptomatic(int);
	void operator++(int);
	int getInfectedIndividuals() const;
	bool isDead(int) const;
	bool wasTestedPositive() const;
	bool testSerology(int simTime, vlsRandGenerator & rnd, const Parameters & p);

	// ************* Intervention related functions *************** //
	// Test the patients
	bool test(int simTime, bool symptoms, bool isolate, vlsRandGenerator & rnd, const Parameters & p);
	// Isolate
	void isolate(int simTime);
	std::array<int,3> getDiagnosed() const;
	std::vector<individuals *> trackedInfected(int, int, std::array<bool, nLinks>, std::array<int, nLinks>);
	void setTracked();

	// ************* Links **************************//
	std::array<unsigned int,3> getWork() const;
	void setWork(unsigned int, unsigned int, unsigned int);
	void sortOneTimeLink();
	void clearLinks();

private:

	// **** Variables :			******** //
	// ********************************* //
	family * _family;
	int _age;
	bool _male;
	disease _disease;
	comorbidities _comorbidities;
	tbb::concurrent_vector<link> _reccurentLink;
	tbb::concurrent_vector<link> _oneTimeLink;
	tbb::concurrent_vector<link>::iterator indexOneTime;

	// Related to work
	unsigned int works;
	unsigned int _firstEmployee;
	unsigned int _EmployeePosition;

	// Tracks
	bool wasTracked;
	int infectedIndividuals;
	bool testedPositive;
	/*
	bool nurse;
	bool md;
	*/

};

