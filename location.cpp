#include "pch.h"
#include "location.h"


location::location(long int size, float density, int x, int y, bool publicTransportation,
	std::vector<vlsRandGenerator> & RandomGenerator, int nThreads, const Parameters & p) : _size(size), _density(density), _location({ x,y }),
	_publicTransportation(publicTransportation), nThreads(nThreads), trackingCases(true), trackPosition(0), testOnlySymptomatic(true), selfTestIfSymptomatic(false)
{

	// Location is initialized
	std::cout << "Location " << x << "," << y << ":" << size << "\n";

	// Set flags for isolation
	for (int i = 0; i < nLinks; i++) {
		_IsolationFlags[i] = INT_MAX; _CanTrack[i]
			= true;
	}
	_CanTrack[lWorkTransport] = false; _CanTrack[lGoingOutTransport] = false; _CanTrack[lShooping] = false;

	// init trakers
	for (int i = 0; i < max_days; i++) for (int j = 0; j < 200; j++) {
		_incidence[j][i] = 0;  _hospitalized[j][i] = 0;  _ICU[j][i] = 0;  _cured[j][i] = 0; _diagnosed[j][i] = 0; _dead[j][i] = 0; _needICU[j][i] = 0;
	}
	for (int i = 0; i < max_days; i++) _ICUAvailable[i] = 0;

	// Calculated available beds
	hospBeds = static_cast<int>(p(HospBedsDensity) * size);
	ICUBeds = static_cast<int>(p(ICUBedsDensity) * size);

	// First create Families & Individuals

	std::cout << "Creating families\n";

	createPopulation(RandomGenerator, p);

	std::cout << "Linking up friends\n";

	// Create friends
	tbb::task_group g;
	
	// Create a social world
	struct socialPlace {
		int x;
		int y;
		family * f;
		int reach;
	} places;
	int static const piecesSize(1000);
	for (int pieces = 0; pieces < static_cast<int>(_families.size() / piecesSize) + 1; pieces++)
	{

		tbb::concurrent_vector<socialPlace> socialWorld;

		// Populate
		for (int i = pieces* piecesSize; i < ((pieces*piecesSize + piecesSize + 100)>_families.size()? _families.size() : (pieces * piecesSize + piecesSize+100)); i++)
		{
			places.f = &_families[i];
			places.reach = static_cast<size_t>(_families[i].getSocialReach());
			socialWorld.push_back(places);
		}

		// Shuffle
		int seed = RandomGenerator[0].uniform();
		std::shuffle(socialWorld.begin(), socialWorld.end(), std::default_random_engine(seed));


		// Random space them
		int * spaces(RandomGenerator[0](10, vlsRandGenerator::size));
		size_t pos(0);
		int rIndex(0);
		for (size_t i = 0; i < socialWorld.size(); i++)
		{
			socialWorld[i].x = spaces[rIndex];
			socialWorld[i].y = spaces[rIndex + 1];
			if (pos >= socialWorld.size()) break;
			rIndex++;
			if (rIndex + 1 >= vlsRandGenerator::size) { spaces = RandomGenerator[0](10, vlsRandGenerator::size); rIndex = 0; }
		}

		size_t i(0);
		size_t socialSize(static_cast<size_t>(sqrt(socialWorld.size())));
		while (i < socialWorld.size())
		{

			//Using task group to attribute the Random stream	

			for (int k = 0; k < nThreads; ++k)
			{

				vlsRandGenerator * rnd = &(RandomGenerator[k]);
				g.run(
					[=] {

					// Number of friends
					int socialReach(socialWorld[i].reach);
					int socialReachSquares((socialReach - socialReach % 10) / 10 + 1);
					std::vector<family *> f;

					// Set up starting position to look for friends
					int i_int(static_cast<int>(i)), socialSize_int(static_cast<int>(socialSize));

					//if (i % 100 == 0) std::cout << "i = " << pieces * piecesSize + i_int << std::endl;

					int startx(i_int % socialSize_int - socialReachSquares); startx = startx < 0 ? 0 : startx;
					int starty((i_int - i_int % socialSize_int) / socialSize_int - socialReachSquares); starty = starty < 0 ? 0 : starty;
					int xCells(0), yCells(0);
					for (size_t x = static_cast<size_t>(startx); x < static_cast<size_t>(startx) + socialReachSquares; x++)
					{
						xCells++;
						for (size_t y = static_cast<size_t>(starty); y < static_cast<size_t>(starty) + socialReachSquares; y++)
						{
							yCells++;

							size_t pos(x + y * socialSize);

							if (pos == i) continue; // skip self

							int dist(distance(socialWorld[i].x, socialWorld[i].y, (xCells - 1) * 10 + socialWorld[pos].x, (yCells - 1) * 10 + socialWorld[pos].y));

							if (dist > socialReach) continue;

							if (socialWorld[pos].reach >= socialReach)
							{
								socialWorld[i].f->addFriend(socialWorld[pos].f, true);
								f.push_back(socialWorld[pos].f);
							}

						}

					}
					// Save to family
					socialWorld[i].f->saveFriends(f);
				});
				i++;
				if (i >= socialWorld.size()) break;

			}
			g.wait();

		}
	}

	std::cout << "Setting-up employement\n";

	// Create employables
	float empRate(p(employmentRate));
	for (size_t i = 0; i < _families.size(); ++i)
	{
		std::vector<individuals *> tempInd(_families[i].getIndividuals());
		for (std::vector<individuals *>::iterator j = tempInd.begin(); j < tempInd.end(); ++j)
		{
			int age((*j)->getAge());
			if (20 < age && age <= 65 && RandomGenerator[0]() < empRate) _employables.push_back((*j));
		}
	}

	// Shuffle
	int seed = RandomGenerator[0].uniform();
	std::shuffle(_employables.begin(), _employables.end(), std::default_random_engine(seed));
	size_t smallCompanies(static_cast<size_t>(p(ProportionsmallCompanies)*_employables.size()));
	size_t employees(_employables.size());
	for (size_t i = 0; i < smallCompanies; i = i + 2)
	{
		for (size_t j = i; j < i + 2; j++) if (j < employees) _employables[i]->setWork(2, static_cast<int>(i), static_cast<int>(j));
	}
	for (size_t i = smallCompanies; i < employees; i = i + 5)
	{
		for (size_t j = i; j < i + 5; j++) if (j < employees) _employables[i]->setWork(5, static_cast<int>(i), static_cast<int>(j));
	}

	// Estimated pop;
	population = 0;
	for (int i = 0; i < _families.size(); i++) { population += _families[i].getSize(); }

	// Other stuff needed for links
	_schools = estimateSectorSize((p(familyType,1) + p(familyType, 2))*p(nChildren) * _families.size() / (20*p(schoolClassSize)));
	_shopping = estimateSectorSize(p(nShopping) * _families.size());
	
}


location::~location()
{
}

void location::setNeighbor(location * l, int position)
{
	_neigbours[position] = l;
}

tbb::concurrent_vector<family>* location::getFamilies()
{
	return &_families;
}

float location::getAvgDistance()
{
	return 0.0;
}

long int location::getSize()
{
	return _size;
}

int location::getPopSize() const
{
	return population;
}

std::array<int, 2> location::getLocation()
{
	return _location;
}

individuals * location::getHead(unsigned int a)
{
	if (a > _families.size()) return nullptr;
	return _families[a].getHead();
}

std::vector<individuals*> location::getInfected()
{
	return _infected;
}

std::array<std::array<int, max_days>, 200> location::getTracking(int a) const
{
	switch (a)
	{
	case 0:
		return _incidence;
		break;
	case 1:
		return _diagnosed;
		break;
	case 2:
		return _hospitalized;
		break;
	case 3:
		return _ICU;
		break;
	case 4:
		return _cured;
		break;
	case 5:
		return _dead;
		break;
	case 6:
		return _needICU;
		break;
	default:
		return std::array<std::array<int, max_days>, 200>();
	}
}

std::vector<individuals*> location::incidence(individuals * ind, int simTime, vlsRandGenerator & rnd, const Parameters & p)
{
	return ind->contaminates(simTime,rnd,p, _IsolationFlags, socialDistanceFlag);
}

// *********** IMPORTANT *********************
//
// This needs to be run after testing and individual 
// isolation as it will affect link creation (and save some compute time)
// *******************************************
void location::addNewlyInfected(std::vector<individuals*> infected, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	

	// For each newly infected individuals
	std::vector<individuals*>::iterator i = infected.begin();
	while (i < infected.end())
	{
		
				int age((*i)->getAge());
				int sex((*i)->getSex());
				

				// Create links
				if (age > 20)
				{
					createShoopingLink((*i), simTime, RandomGenerator, p);

					createWorkLink((*i), RandomGenerator, p);

					createTransportLink((*i), simTime, RandomGenerator, p);

					createFriendLink((*i), simTime, RandomGenerator, p);

					createEventLink((*i), simTime, RandomGenerator, p);
				}

				else
				{
					createSchoolLink((*i), RandomGenerator, p); // Check if school are isolated

				}

				// sort the oneTime
				(*i)->sortOneTimeLink();

				
			
			
			
			// Tracking
			std::array<std::array<int, 2>, 4> track = (*i)->getTracking();

			// Count
			_incidence[sex * 100 + age][simTime]++;
			if (track[0][0] < max_days) _diagnosed[sex * 100 + age][track[0][0]]++;
			// ICU
			if (track[2][0] != INT_MAX) // Needs ICU
			{
				for (int i = track[2][0]; i < (track[2][1] < max_days ? track[2][1] : max_days); i++) _needICU[sex * 100 + age][track[2][0]]++;
				if (_ICUAvailable[track[2][0]] > ICUBeds) // No beds available
				{
					(*i)->noICU(track[2][0]);
					track = (*i)->getTracking(); // Reload with death
				}
				else
				{
					for (int i = track[2][0]; i < (track[2][1] < max_days ? track[2][1] : max_days); i++) _ICU[sex * 100 + age][i]++;
					for (int i = track[2][0]; i < (track[2][1] < max_days ? track[2][1] : max_days); i++) _ICUAvailable[i]++;
				}
			}
			if (track[1][0] != INT_MAX) for (int i = track[1][0]; i < (track[1][1] < max_days? track[1][1]: max_days); i++) _hospitalized[sex * 100 + age][i]++;
			if (track[3][0] < max_days) _cured[sex * 100 + age][track[3][0]]++; 
			if (track[3][1] < max_days) _dead[sex * 100 + age][track[3][1]]++;
			
			_infected.push_back(*i);
i++;
	}

	
}

void location::nextDay(int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	std::vector<individuals *> contaminations;
	int index(-1);
	for (std::vector<individuals *>::iterator i = _infected.begin(); i < _infected.end(); i++)
	{
		index++;
		std::array<int, 2> ContaminationWindow = (*i)->getContaminationWindow();
		
		if (ContaminationWindow[1] < simTime)
		{
			(*i)->clearLinks();
			continue;

		}
		if (selfTestIfSymptomatic)
		{
			if ((*i)->isSymptomatic(simTime-1))
			{
				if ((*i)->test(simTime, false, true, RandomGenerator[0], p))
				{
					trackPosition = index;
					continue;
				}
			}
		}

		std::vector<individuals *> contaminationsFromAPatient = incidence(*i, simTime, RandomGenerator[0], p);

		for (std::vector<individuals *>::iterator j = contaminationsFromAPatient.begin(); j < contaminationsFromAPatient.end(); j++)
		{
			contaminations.push_back(*j);
		}

	}

	addNewlyInfected(contaminations, simTime, RandomGenerator, p);

	//std::cout << "Day " << simTime << ":" << contaminations.size() << "new infected\n" << _infected.size() << " infected total\n";

}

void location::trackCases(int simTime, std::vector<vlsRandGenerator>& RandomGenerator, const Parameters & p)
{
	if (!trackingCases) return; // Stop tracking,ex. if too many cases

	tbb::task_group g;

	size_t i(trackPosition), j(trackPosition);
	std::vector<size_t> jpos;
	for (int k = 0; k < nThreads; ++k) {
		jpos.push_back(trackPosition);
	}


	while (i < _infected.size())
	{
		for (int k = 0; k < nThreads; ++k) {

			vlsRandGenerator * rnd = &(RandomGenerator[k]);

			jpos[k] = i;

			g.run(
				[=, &jpos] {
				trackPosition = i;
				std::array<int, 3> d((_infected[i])->getDiagnosed()); // Get infection and diagnosed date
				if (d[0] == 1 || d[2] == INT_MAX)
				{
					return; // Already tracked or never diagnosed;
				}
				else if (d[2] > simTime)
				{
					jpos[k] = (i > jpos[k]) ? jpos[k] : i;
					return; // Not yet diagnosed, may need to come back to him
				}
				else
				{
					// Isolate individual
					_infected[i]->isolate(simTime);
					_infected[i]->setTracked();

					// Look for contacts
					int delay(1); // RandomGenerator[0].poisson(p(AvgTrackingTim)));
					std::vector<individuals*> contacts = (_infected[i])->trackedInfected(d[1]+delay, simTime, _CanTrack, _IsolationFlags); // Get all between infection date and diagnosis

					// For each contact, let test at +delay of diagnosis
					for (std::vector<individuals*>::iterator j = contacts.begin(); j < contacts.end(); j++)
					{
						(*j)->test(d[2] + delay, testOnlySymptomatic, true, (*rnd), p);
					}
				}
			});
			g.wait();
				i++;
				if (i >= _infected.size()) break;
			}
	}
	trackPosition = i;
	for (int k = 0; k < nThreads; ++k) { if (jpos[k] != 0) trackPosition = (trackPosition > jpos[k]) ? jpos[k] : trackPosition; }
	
}



float location::getNumberDiagnosed(int simTime) const
{
	float a = 0.0f;
	for (int j = 0; j < simTime + 1; j++) for (int i = 0; i < 200; i++) a += _diagnosed[i][j];
	return 100000.0f * a / population;
}

void location::disableTracking()
{
	trackingCases = false;
}

void location::enableTracking(int i)
{
	trackingCases = true;
	if (i == 1) testOnlySymptomatic = false;
}

void location::containement(int age, int link)
{
	_IsolationFlags[link] = age;
}

void location::setSelfTesting(bool b)
{
	selfTestIfSymptomatic = b;
}

void location::setTrackingCapability(int i , bool b)
{
	_CanTrack[i] = b;
}

void location::printStuff()
{
	int c(0); double d(0);
	// Avg Diag
	for (std::vector<individuals *>::iterator i = _infected.begin(); i < _infected.end(); i++)
	{
		std::array<int, 3> a = (*i)->getDiagnosed();
		if (a[2] != INT_MAX) {
			c++; d += a[2] -a[1];
		}
	}

	d = d / static_cast<double>(c);
	std::cout << "Average time for diagnosis : " << d << "\n";
}

void location::setSocialDistancing(bool b)
{
	socialDistanceFlag = b;
}

void location::serologyTesting(int simTime, std::vector<vlsRandGenerator>& RandomGenerator, const Parameters & p)
{
}

void location::initSerologyScreened(int nThreads, int seed)
{
	// create list of families divided in blocks
	/*for (int i = 0; i < nThreads; i++)
	{
		linkedIndividuals a;
		serologyScreened.push_back(a);
	}
	
	size_t n(_families.size());
	int l = static_cast<int>(n / nThreads);

	// Fill in the individuals
	tbb::task_group g;

	for (int k = 0; k < nThreads; ++k) {

		g.run(
			[=] {
			int index(0);
			individuals * next = nullptr;
			individuals * current = nullptr;
			
			// First element
			std::vector<individuals *> inds = _families[k*l].getIndividuals();
			int nInd(inds.size());
			current = inds[0];
			
			for (int i = k * l + 1; i < k*l + l; i++)
			{
				inds = _families[i].getIndividuals();
				nInd = inds.size();

				// First

				for (std::vector<individuals *>::iterator ind = inds.begin(); ind < inds.end(); ind++)
				{

					linkedIndividuals a;
					serologyScreened[k].

				}
			
			}
		});

	}
	g.wait();

	// shuffle
	// fill the individuals linked list */
}

int location::estimateSectorSize(float a)
{
	float avgDist = sqrt(_families.size() / a);
	return static_cast<int>(avgDist+0.5);
}

int location::distance(int x1, int y1, int x2, int y2)
{
	return static_cast<int>(sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)));
}

void location::createSchoolLink(individuals * ind, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	// If has School link then do nothing
	if (ind->hasLink(lSchool)) return;

	// ************* Create school link **********************
	int age((*ind).getAge());
	std::array<int, 2> position = ind->getFamily()->getPosition();
	
	// Vector containing classmates that will be link
	tbb::concurrent_vector<individuals *> classmates;

	// Figure out his sector
	int x((position[0] - position[0] % _schools) / _schools);
	int y((position[0] - position[0] % _schools) / _schools);
	
	int maxPos = static_cast<int>(_families.size()), fsize(static_cast<int>(sqrt(maxPos)));

	tbb::task_group g;
	
	// Find his classmates in the block
	int col(0);
	while (col < _schools)
	{
		for (int k = 0; k < nThreads; ++k) {
		
			vlsRandGenerator * rnd = &(RandomGenerator[k]);

			
			g.run(
				[=, &classmates] {
					int row(0);
					while (row < _schools)
					{
						// creates an array of all the children in the block
						int pos = (col +  y* _schools)*fsize + x * _schools + row;

						if (pos < maxPos) {
							std::vector<individuals *> familyMembers = _families[pos].getIndividuals(age);
							for (std::vector<individuals *>::iterator i = familyMembers.begin(); i < familyMembers.end(); ++i)
							{
								classmates.push_back(*i);
							}
						}
						else break;

						row++;
			
					}
			
			});
			
			col++;
			if (col >= _schools) break;
		}
		g.wait();
	}

	struct linksToAdd
	{
		individuals * ind;
		std::vector<link> l;
	};

	std::vector<linksToAdd> lta;

	//Let's link them up
	for (size_t i = 0; i < classmates.size(); ++i)
	{
		linksToAdd a;
		a.ind = classmates[i];

		for (size_t j = 0; j < classmates.size(); ++j)
		{
			if (i != j) a.l.push_back(schoolLink(classmates[j]));
		}

		lta.push_back(a);
	}
	int i(0);
	while (i < lta.size())
	{
		for (int k = 0; k < nThreads; ++k) {
			g.run(
				[=, &lta] {
					 lta[i].ind->addLink(lta[i].l);
			});
			i++;
			if (i >= lta.size()) break;
		}
		g.wait();

	}
}

void location::createShoopingLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{

	// Only adults
	if (ind->getAge() < 20) return;

	// Figure out his sector
	std::array<int, 2> position = ind->getFamily()->getPosition();
	int x((position[0] - position[0] % _shopping) / _shopping);
	int y((position[0] - position[0] % _shopping) / _shopping);

	int maxPos = static_cast<int>(_families.size()), fsize(static_cast<int>(sqrt(maxPos)));
	// Shopping
	// Assume meeting 6 people during shopping, going once a week, that means pulling 3 per person at random for each individual in a cluster
	
	// Choose a time 
	std::array<int, 2> time(ind->getContaminationWindow());
	int startTime(time[0]), nEncounters(static_cast<int>(p(AvgShoppingEncounters)));

	tbb::task_group g;

	// Go through contamination period and create links
	while (startTime < time[1])
	{
		for (int k = 0; k < nThreads; ++k) {

			vlsRandGenerator * rnd = &(RandomGenerator[k]);

			startTime += (*rnd).poisson(p(AvgShoppingFreq));
			if (startTime >= time[1]) break;

			g.run(
				[=] {
				
					// Get random coordinates
					int * coord = (*rnd)(_shopping, 2 * nEncounters);
					for (int i = 0; i < nEncounters; ++i)
					{
						// Choose 6 random coordinates
					int * coord = (*rnd)(_shopping, 2 * nEncounters);
						int pos = (coord[i] +y * _shopping)*fsize + x * _shopping + coord[i+ nEncounters];

						if (pos < maxPos) {
							individuals * ptrInd(_families[pos].getHead());
							link l = shoppingLink(ptrInd);
							ind->addLink(l, startTime);
							// ptrInd->addLink(&(_storage.back()), startTime); No used
							}
					}
				});
			
		}
		g.wait();
	}	
}

void location::createWorkLink(individuals * ind, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	
	// check if already has link
	if (ind->hasLink(lWork)) return;

	// Check if employed
	std::array<unsigned int, 3> work(ind->getWork());
	if (work[0] == 0) return;

	struct linksToAdd
	{
		individuals * ind;
		std::vector<link> l;
	};

	std::vector<linksToAdd> lta;

	tbb::task_group g;

	//Let's link them up
	for (size_t i = work[1]; i < work[2]+1; ++i)
	{
		linksToAdd a;
		a.ind = _employables[i];

		for (size_t j = work[1]; j < work[2]+1; ++j)
		{
			if (i != j) a.l.push_back(workLink(_employables[j]));
		}

		lta.push_back(a);
	}
	int i(0);
	while (i < lta.size())
	{
		for (int k = 0; k < nThreads; ++k) {
			g.run(
				[=, &lta] {
				lta[i].ind->addLink(lta[i].l);
			});
			i++;
			if (i >= lta.size()) break;
		}
		g.wait();

	}

}

void location::createTransportLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	if (!_publicTransportation) return;

	// Only adults (Assumption could be changed later
	int age(ind->getAge());
	if (age < 20) return;

	bool work(ind->getWork()[0] != 0);
	
	// Choose a time 
	std::array<int, 2> time(ind->getContaminationWindow());
	int nEncounters(static_cast<int>(p(AvgTransportEncounters)));
	
	tbb::task_group g;

	// Find frequency
	if (work)
	{
		// set every day workTransportLink
		int startTime = time[0];
			
		// Go through contamination period and create links
		while (startTime < time[1])
		{
			for (int k = 0; k < nThreads; ++k) {

				vlsRandGenerator * rnd = &(RandomGenerator[k]);

				startTime++;
				if (startTime >= time[1]) break;

				g.run(
					[=] {

					// Get random coordinates
					int * coord = (*rnd)(static_cast<int>(_employables.size()), nEncounters);
					for (int i = 0; i < nEncounters; ++i)
					{
						individuals * ptrInd(_employables[coord[i]]);
						if (ind != ptrInd)
						{
							link l = workTransportLink(ptrInd);
							ind->addLink(l, startTime);
						}
					}
				});
			}
			g.wait();
		}
	}
	
	// For all, related to families not individuals as it is for going out
	// Go through contamination period and create links
	int startTime(time[0]);
	int size = static_cast<int>(_families.size());
	nEncounters = static_cast<int>(p(AvgTransportEncountersLow));
	while (startTime  < time[1])
	{
		for (int k = 0; k < nThreads; ++k) {

			vlsRandGenerator * rnd = &(RandomGenerator[k]);
			startTime += (*rnd).poisson(p(AvgTripGoingOutFreq));

			g.run(
				[=] {
				// Get random coordinates
				int * coord = (*rnd)(size, nEncounters);
				for (int i = 0; i < nEncounters; ++i)
				{
					std::vector<individuals *> ptrInds(_families[coord[i]].getIndividuals());
					for (std::vector<individuals *>::iterator j = ptrInds.begin(); j < ptrInds.end(); ++j)
					{
						link l = gointOutTransportLink(*j);
						ind->addLink(l, startTime);

					}

				}
			});
		}
		g.wait();

	}
}

void location::createFriendLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{

	// check if already has link
	if (ind->hasLink(lFriend)) return;

	// Choose a time 
	std::array<int, 2> time(ind->getContaminationWindow());
	int startTime(time[0]);
	int nFriends(ind->getFamily()->getTotalFriends());

	if (nFriends == 0) return;

	tbb::concurrent_vector<family *> familyFriends(ind->getFamily()->getFriends());
	std::vector<individuals *> members(ind->getFamily()->getIndividuals());

	tbb::task_group g;
	int prev(startTime);

	while (startTime < time[1]) // Spacing to allow other invits for other friends 
	{
		for (int k = 0; k < nThreads; ++k) {

			vlsRandGenerator * rnd = &(RandomGenerator[k]);
			if (startTime == time[0]) startTime += (*rnd).poisson(p(AvgFriendEncountersFreq)/2);
			else startTime += (*rnd).poisson(p(AvgFriendEncountersFreq));
			std::shuffle(familyFriends.begin(), familyFriends.end(), std::default_random_engine((*rnd).uniform()));

			g.run(
				[=] {
				int n((*rnd).poisson(p(AvgFriendperEncounters)));
				//if (ind->getAge() > 65) n = n / 2;
				std::vector<individuals *> f;
				// Select friends that have contact during encounter
				for (int i = 0; i < (n > nFriends ? nFriends : n); ++i)
				{

					if (familyFriends[i]->someIsSick(startTime)) {}
					else
					{
						std::vector<individuals *> fm(familyFriends[i]->getIndividuals());
						for (std::vector<individuals *>::iterator j = fm.begin(); j < fm.end(); ++j)
						{
							f.push_back(*j);
						}
					}
				}

				struct linksToAdd
				{
					individuals * ind;
					std::vector<link> l;
					int time;
				};

				std::vector<linksToAdd> lta;

				// link them up
				// 1. family members to friends
				for (size_t i = 0; i < members.size(); i++)
				{
					linksToAdd a;
					a.time = startTime;
					a.ind = members[i];
					for (size_t j = 0; j < f.size(); j++) a.l.push_back(friendLink(f[j]));
					lta.push_back(a);
				}

				// 2. friends to family members
				for (size_t i = 0; i < f.size(); i++)
				{
					linksToAdd a;
					a.time = startTime;
					a.ind = f[i];
					for (size_t j = 0; j < members.size(); j++) a.l.push_back(friendLink(members[j]));
					lta.push_back(a);
				}

				// 3. Save
				for (size_t i = 0; i < lta.size(); i++) for (size_t j = 0; j < lta[i].l.size(); j++) lta[i].ind->addLink(lta[i].l[j], lta[i].time);
			});
		}
		g.wait();
	}

	// done
}

void location::createEventLink(individuals * ind, int simTime, std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{

	// Choose a time 
	std::array<int, 2> time(ind->getContaminationWindow());
	int startTime(time[0]);
	int freq(RandomGenerator[0].poisson(p(AvgEventsPerIndividualPerWeek))); if (freq == 0) return;
	int delayEvents = static_cast<int>(365 / 2) / freq;
	startTime = delayEvents - startTime;
	int nEncounters(RandomGenerator[0].poisson(p(AvgEventPersons)/p(AvgHousehold)));
	tbb::task_group g;
	while (startTime < time[1]) // Spacing to allow other invits for other friends 
	{
		for (int k = 0; k < nThreads; ++k) {

			vlsRandGenerator * rnd = &(RandomGenerator[k]);
			startTime += delayEvents;
			if (startTime >= time[1]) break;
			g.run(
				[=] {
				// Get random coordinates
				int * coord = (*rnd)(static_cast<int>(_families.size()), nEncounters);
				for (int i = 0; i < nEncounters; ++i)
				{
					std::vector<individuals *> ptrInds(_families[coord[i]].getIndividuals()); // Assume that events are a family thing
					for (std::vector<individuals *>::iterator j = ptrInds.begin(); j < ptrInds.end(); ++j)
					{
						int age(ind->getAge());
						if (15 < age && age < 75) {
							link l = eventLink(*j);
							ind->addLink(l, startTime);
						}
					}

				}
			});

			
		}
		g.wait();
	}
}


void location::createPopulation(std::vector<vlsRandGenerator> & RandomGenerator, const Parameters & p)
{
	// Create families on n * n grid;
	long int n = static_cast<int>(sqrt(_size / p(AvgHousehold)));
	_size = n;
	_families.reserve(n*n);

	int j(0), i(0);
	tbb::task_group g;
	while (j*i < n*n)
	{
			//Using task group to attribute the Random stream	
			
		for (int k = 0; k < nThreads; ++k)
		{
			vlsRandGenerator * rnd = &(RandomGenerator[k]);
			g.run(
				[=] {
						_families.emplace_back(i, j, *rnd, p);
					}
			);
			j++;
			if (j > n && i <= n) { i++; j = 0; }
			if (j*i == n * n) break;
				
		}
		g.wait();

	}

	std::cout << "Population created\n";
		
}


/*
size_t i(0);
	while (i < _families.size())
	{
		//Using task group to attribute the Random stream

		for (int k = 0; k < nThreads; ++k)
		{

			vlsRandGenerator * rnd = &(RandomGenerator[k]);
			g.run(
				[=] {

					// Number of friends
					int nFriends(_families[i].getNumberofFriends());

					if (nFriends > 0) {

						// Random coordinates
						int * coord(rnd->operator()(static_cast<int>(_families.size()), nFriends, static_cast<int>(i + 1)));

						std::vector<family *> f;
						for (int j = 0; j < nFriends; j++)
						{
							if (i != coord[j] && _families[coord[j]].addFriend(&_families[i], true)) f.push_back(&_families[coord[j]]);
						}

						// Save to family
						_families[i].saveFriends(f);
					}

			});
			i++;
			if (i >= _families.size()) break;
		}
		g.wait();

	}
	*/