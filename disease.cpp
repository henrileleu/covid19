#include "pch.h"
#include "disease.h"


disease::disease(): _infected(INT_MAX), _symptomatic(INT_MAX), _hasSymptoms(false), _severity(0), _diagnosed(INT_MAX), _hospitalized(INT_MAX), _isolated(INT_MAX), _icu(INT_MAX), 
	_dead(INT_MAX), _cured(INT_MAX)
{
}


disease::~disease()
{
}

bool disease::isInfected(int t) const
{
	return _hasSymptoms && _infected<=t;
}

bool disease::isSymptomatic(int t) const
{
	return _hasSymptoms && _symptomatic <= t;
}

int disease::severity() const
{
	return _severity;
}

bool disease::diagnosed(int t) const
{
	return _diagnosed <= t;
}

bool disease::hospitalized(int t) const
{
	return _hospitalized <= t;
}

bool disease::isolated(int t) const
{
	return _isolated <= t;
}

bool disease::ICU(int t) const
{
	return _icu <= t;
}

bool disease::dead(int t) const
{
	return _dead <= t;
}

bool disease::cured(int t) const
{
	return _cured <= t;
}

std::array<int, 2> disease::getContaminationWindow() const
{
	int minT = _infected;
	int maxT(_infected);
	if (_hospitalized != INT_MAX) maxT = maxT > _hospitalized ? maxT : _hospitalized;
	if (_isolated != INT_MAX) maxT = maxT > _isolated ? maxT : _isolated;
	if (_dead != INT_MAX) maxT = maxT > _dead ? maxT : _dead;
	if (_cured != INT_MAX) maxT = maxT > _cured ? maxT : _cured;

	return std::array<int, 2>({ minT+1, maxT });
}

bool disease::wasInfected(int t ) const
{
	return _infected <= t;
}

int disease::infectionTime() const
{
	return _infected;
}

std::array<std::array<int, 2>, 4> disease::getTracking() const
{
	int end(_dead > _cured ? _cured : _dead);
	return std::array<std::array<int, 2>, 4>({ { {_diagnosed, end },{_hospitalized, end},{_icu, end},{_cured, _dead }} });
}

std::array<int, 2> disease::getDiagnosedWindow() const
{
	return std::array<int, 2>({ _infected, _diagnosed });
}

void disease::setIsolate(int t)
{
	_isolated = t;
}

bool disease::contaminates(int simTime, individuals * ind, vlsRandGenerator & rnd, const Parameters & p, int distance, int time, bool hospitalLink, bool _socialDistancing)
{

	// Check if patients is isolated, dead or in the hospital, or cured
	// In all theses cases he cannot infected anybody, except for hospital if this is a link to a hospital worker
	if ((!hospitalLink && hospitalized(simTime)) || isolated(simTime) || dead(simTime) || cured(simTime)) return false;

	// If connection is already sink nothing to do
	if (ind->wasInfected(simTime)) return false;

	// Gradual infectiosity until time of clinical onset
	float contagiosity(1);
	if (simTime < _symptomatic) contagiosity = static_cast<float>(1.0f / pow(2, _symptomatic - simTime));

	// Reduced contagiosity of asymptomatics
	if (!_hasSymptoms) contagiosity = 0.01f;

	// Check if individuals is contaminated
	if (rnd() > (_socialDistancing ? p(socialDistancing) : 1) * p(contaminationRisk) * time * contagiosity / (distance * distance)) return false;

	// Set contamination
	(*ind).contaminated(rnd, p, simTime);

	return true;

}

bool disease::contaminates(int simTime, individuals * ind, vlsRandGenerator & rnd, const Parameters & p, int time, int distance)
{

	return contaminates(simTime, ind, rnd, p, time, distance, false, false);

}
void disease::contaminated(individuals const * ind, vlsRandGenerator & rnd, const Parameters & p, int t)
{
	contaminated(ind, rnd, p, t, 1);
}
void disease::contaminated(individuals const * ind, vlsRandGenerator & rnd, const Parameters & p, int t, float reducedDiagnosis)
{
	_infected = t;
	_isolated = INT_MAX;
	// Set severity (estimated severe in diagnosed case only)
	float r = rnd();
	int age((*ind).getAge());
	
	age = age + ind->getNumberofComorbidities() * 5; // add 5 year per comorbidities, will do better later
	
	// Pr Severe & Critical
	float ageSevere = age < p(prSevereAgeTh) ? p(prSevereAgeTh) / 100.0f : static_cast<float>(age) / 100.0f;
	float s0(((p(prAsymp, 0) - (p(prAsymp, 1))*p(prDiagnosed))/10) * age + p(prAsymp, 1)*p(prDiagnosed)), s1((1/p(PrDeath))*p(prCritical,0) * exp ((static_cast<float>(age)/100.0f) * p(prCritical, 1))), s2(p(prSevere, 0) * ageSevere * ageSevere + p(prSevere, 1) * ageSevere + p(prSevere, 2));

	// Symptoms
	_symptomatic = static_cast<int>(t + rnd.weibull(p(delaySymptomatic, 0), p(delaySymptomatic, 1)));

	// Asympt
	s0 = s0 < 0 ? 0 : s0;
	r -= s0;

	if (r < 0) { // Asymptomatic

	}
	else
	{
		// Diagnosed
		_diagnosed = static_cast<int>(_symptomatic + rnd.weibull(p(delayDiagnosis, 0), p(delayDiagnosis, 1)));

		// Adjust for diagnosed
		float denominator(1);
		r = rnd();
		r = r - s1 * denominator;
		if (r < 0) _severity = 2;
		else if (r < s2 * denominator) _severity = 1;
		
	}
	// Symptoms
	_hasSymptoms = _severity == 0?rnd() < (p(prSymptomatic)*(1-s1-s2)):true; // Only mild patient are symptomatic
	
	// Hospitalized
	if (_severity > 0) _hospitalized = static_cast<int>(t + rnd.weibull(p(delayHospitalized, 0), p(delayHospitalized, 1)));
	if (_hospitalized < _diagnosed) _diagnosed = _hospitalized;

	// ICU

	if (_severity == 2) { _icu = _hospitalized+2; }

	// Dead
	// For now we will assume that only critical case die (Based on China CDC Report)
	if (_severity == 2)
	{
		if (rnd() < p(PrDeath) * p(il9))
		{
			_dead = static_cast<int>(_icu + rnd.weibull(p(delayICUDeath, 0), p(delayICUDeath, 1)));
		}
		else
		{
			_cured = static_cast<int>(_icu + rnd.weibull(p(delayRecovery, 0), p(delayRecovery, 1)));
		}
	}

	// Cured
	if (_severity < 2) _cured = static_cast<int>((_severity==0?_symptomatic:_hospitalized) + +rnd.weibull(p(delayRecovery, 0), p(delayRecovery, 1)));
	if (_cured < _diagnosed) _diagnosed = INT_MAX;
	
	
}

void disease::contaminated()
{
	_infected = 0;
	_symptomatic = 0;
}

// Kills patient if no ICU is available at time needed
void disease::noICU(int simTime)
{
	_icu = INT_MAX; _dead = simTime;
}

// Return the infection status regardless of symptoms
bool disease::test(int simTime, bool isolate)
{
	if (dead(simTime) || cured(simTime)) return false;

	if (simTime > _infected)
	{
	
		_diagnosed = simTime + 1;
		if (isolate) _isolated = simTime;
		return true;

	}

	return false;
}


