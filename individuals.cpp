#include "pch.h"
#include "individuals.h"


individuals::individuals()
{
}

individuals::individuals(int age, int sex, vlsRandGenerator & rnd, const Parameters & p, family * f) :
	_age(age), _male(sex == 0), _comorbidities(comorbidities(age, sex, rnd, p)), works(0), _family(f), wasTracked(false), infectedIndividuals(0), testedPositive(false)
{

	// Works in hospital
	/*nurse = rnd() < p(NurseProportion);
	md = rnd() < p(MDProportion);*/
	if (_age > 99) _age = 99;

}


individuals::~individuals()
{
}

void individuals::addLink(link l)
{
	// Add to this one
	_reccurentLink.push_back(l);
}

void individuals::addLink(link l, int simTime)
{
	// Add to this one
	l.setSimTime(simTime);
	_oneTimeLink.push_back(l);

}

void individuals::addLink(std::vector<link> l)
{
	for (std::vector<link>::iterator i = l.begin(); i < l.end(); i++) addLink(*i);
}

bool individuals::hasLink(int type)
{
	for (tbb::concurrent_vector<link>::iterator i = _reccurentLink.begin(); i < _reccurentLink.end(); ++i)
	{
		if (i->getType() == type) return true;
	}
	return false;
}
void individuals::contaminated(vlsRandGenerator & rnd, const Parameters & p, int t)
{
	contaminated(rnd, p, t, 1);
}

void individuals::contaminated(vlsRandGenerator & rnd, const Parameters & p, int t, float reducedDiagnosis)
{
	_disease.contaminated(this, rnd, p, t, reducedDiagnosis);
	if (!(*getFamily()).linkCreated()) (*getFamily()).createLink();
}

// go through array of links if is infected
std::vector<individuals * > individuals::contaminates(int simTime, vlsRandGenerator & rnd, const Parameters & p, std::array<int, nLinks> _IsolationFlags, bool socialDistanceFlag)
{
	std::vector<individuals * > newInfected;
	individuals * ind;
	std::array<int, 2> linkProperties;
	

	int f(simTime % 7);

	// Recurrent Link
	for (tbb::concurrent_vector<link>::iterator i = _reccurentLink.begin(); i < _reccurentLink.end(); ++i)
	{
		

		// Get connected individual
		ind = i->connected(simTime,f);
		if (ind == nullptr) continue;
		
		if (ind->getAge() >= _IsolationFlags[i->getType()] && !ind->wasTestedPositive()) continue;
		// Replaced by biological age to account for comorbidities 
		//if (ind->getBiologicalAge() >= _IsolationFlags[i->getType()] && !ind->wasTestedPositive()) continue;


		// Get link properties
		linkProperties = i->getProperties();

		if (_disease.contaminates(simTime, ind, rnd, p, linkProperties[0], linkProperties[1],false, socialDistanceFlag))
		{
			(*i).contaminated();
			newInfected.push_back(ind);
		}
	}

	// One time at date of simTime
	for (tbb::concurrent_vector<link>::iterator i = indexOneTime; i < _oneTimeLink.end(); i++)
	{

		// Now sorted by time
		if (i->getTime() > simTime) break;

		// Get connected individual
		ind = i->connected(simTime, f);

		if (ind == nullptr) {
			indexOneTime = i; continue;
		} 
		// Replaced by biological age to account for comorbidities 
		if (ind->getAge() >= _IsolationFlags[i->getType()] && !ind->wasTestedPositive()) {
		//if (ind->getBiologicalAge() >= _IsolationFlags[i->getType()] && !ind->wasTestedPositive()) {
			indexOneTime = i; continue;
		}

		// Get link properties
		linkProperties = i->getProperties();

 		if (_disease.contaminates(simTime, ind, rnd, p, linkProperties[0], linkProperties[1], false, socialDistanceFlag))
		{
			newInfected.push_back(ind);
		}

		indexOneTime = i;
	}

	infectedIndividuals += static_cast<int>(newInfected.size());
	return newInfected;
}

std::array<int, 2> individuals::getContaminationWindow() const
{
	return _disease.getContaminationWindow();
}

bool individuals::isInfected(int t) const
{
	return _disease.isInfected(t);
}

bool individuals::wasInfected(int t) const
{
	return _disease.wasInfected(t);
}

void individuals::setPatientZero()
{
	_disease.contaminated();
}

int individuals::infectionTime() const
{
	return _disease.infectionTime();
}

void individuals::noICU(int t)
{
	_disease.noICU(t);
}

bool individuals::isSymptomatic(int t)
{
	return _disease.isSymptomatic(t);
}

void individuals::operator++(int)
{
	infectedIndividuals++;
}

int individuals::getInfectedIndividuals() const
{
	return infectedIndividuals;
}

bool individuals::isDead(int t) const
{
	return _disease.dead(t);
}

bool individuals::wasTestedPositive() const
{
	return testedPositive;
}

bool individuals::testSerology(int simTime, vlsRandGenerator & rnd, const Parameters & p)
{
	// Was diagnosed, skip the test and is considered OK
	if (_disease.diagnosed(simTime)) {
		testedPositive = true;
		return false;
	}

	bool infected(_disease.wasInfected(simTime));

	if (infected)
	{
		if (rnd() < p(serologySe, 0)) // Sensibility
		{
			testedPositive = true;
		}
	}
	else
	{
		if (rnd() < 1 - p(serologySe, 1)) // Specificity
		{
			testedPositive = true;
		}
	}

	return true;

}

// Test if individual is infected
// Restrict test to symptomatic or to all
// isolate positive patients
// Takes into account sensitiviy of the PCR (= false negative)
bool individuals::test(int simTime, bool symptoms, bool isolate, vlsRandGenerator & rnd, const Parameters & p)
{

	if ((symptoms && _disease.isSymptomatic(simTime)) || !symptoms)
	{
		if (rnd() < p(PCRSensitivity) /*&& rnd() < p(prSuccessTrack)*/) return _disease.test(simTime, isolate);
	}

	return false;

}

void individuals::isolate(int simTime)
{
	_disease.setIsolate(simTime);
}

std::array<int, 3> individuals::getDiagnosed() const
{
	std::array<int, 2> a = _disease.getDiagnosedWindow();
	return std::array<int, 3>({ wasTracked ? 1 : 0, a[0], a[1] });
}

std::vector<individuals*> individuals::trackedInfected(int s, int e, std::array<bool, nLinks> canTrack, std::array<int, nLinks> _IsolationFlags)
{
	std::vector<individuals*> contacts;

	individuals * ind;
	
	// ***** Go through links *************************
	// Recurrent Link
	for (tbb::concurrent_vector<link>::iterator i = _reccurentLink.begin(); i < _reccurentLink.end(); ++i)
	{

		/*if (_age >= _IsolationFlags[i->getType()]) continue;*/

		// Get connected individual
		ind = i->contactBetween(s, e);
		if (ind == nullptr) continue;
		
		contacts.push_back(ind);
	}

	// One time at date of simTime
	for (tbb::concurrent_vector<link>::iterator i = _oneTimeLink.begin(); i < _oneTimeLink.end(); i++)
	{
		/*if (_age >= _IsolationFlags[i->getType()]) continue;*/

		int ltime(i->getTime());
		// Now sorted by time
		if (ltime < s) continue;
		if (ltime > e) break;
		
		// Get connected individual
		if (!canTrack[i->getType()]) continue;

		ind = i->getPair();
		if (ind == nullptr) continue;

		contacts.push_back(ind);
	}

	return contacts;
}

void individuals::setTracked()
{
	wasTracked = true;
}

std::array<unsigned int, 3> individuals::getWork() const
{
	unsigned int w;
	w = works > 0 ? 1 : 0;
	return std::array<unsigned int, 3>({ w, _firstEmployee , _EmployeePosition });
}

void individuals::setWork(unsigned int s, unsigned int a, unsigned int b)
{
	works = s;  _firstEmployee = a; _EmployeePosition = b;
}

void individuals::sortOneTimeLink()
{
	std::sort(_oneTimeLink.begin(), _oneTimeLink.end(), [=](link i, link j) { return i.getTime() < j.getTime(); });
	indexOneTime = _oneTimeLink.begin();
}

void individuals::clearLinks()
{
	if (_reccurentLink.size() > 0) {
		_reccurentLink.clear();
		_reccurentLink.shrink_to_fit();
	}
	if (_oneTimeLink.size() > 0) {
		_oneTimeLink.clear();
		_oneTimeLink.shrink_to_fit();
	}
}

int individuals::getAge() const
{
	return _age;
}

int individuals::getBiologicalAge() const
{
	return _age + getNumberofComorbidities() * 5;
}

int individuals::getSex() const
{
	return _male?0:1;
}

std::array<std::array<int, 2>, 4> individuals::getTracking() const
{
	return _disease.getTracking();
}

std::array<int, 2> individuals::getCaracteristics() const
{
	return std::array<int, 2>({ _age,_male?0:1 });
}

int individuals::getNumberofComorbidities() const
{
	int n(0);
	for (int i = 0; i < comorbidities::nComorbidities; i++)
	{
		if (_comorbidities.getComorbidity(i)) n++;
	}
	return n;
}

family * individuals::getFamily()
{
	return _family;
}
