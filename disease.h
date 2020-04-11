#pragma once

// Foward dec.
class individuals;

class disease
{
public:
	disease();
	~disease();

	/* Get Functions : Return Status at a simulation time */
	bool isInfected(int) const;
	bool isSymptomatic(int) const;
	int severity() const;
	bool diagnosed(int) const;
	bool hospitalized(int) const;
	bool isolated(int) const;
	bool ICU(int) const;
	bool dead(int) const;
	bool cured(int) const;
	std::array<int, 2> getContaminationWindow() const;
	bool wasInfected(int) const;
	int infectionTime() const;
	std::array < std::array<int, 2>, 4> getTracking() const;
	std::array<int, 2> getDiagnosedWindow() const;
	void setIsolate(int);

	bool contaminates(int simTime,  individuals * ind, vlsRandGenerator & rnd, const Parameters & p, int time, int distance, bool hospitalLink, bool socialDistancing);
	bool contaminates(int simTime, individuals * ind, vlsRandGenerator & rnd, const Parameters & p, int time, int distance);
	void contaminated(individuals const * ind, vlsRandGenerator & rnd, const Parameters & p, int t, float reduceDiagnosis);
	void contaminated(individuals const * ind, vlsRandGenerator & rnd, const Parameters & p, int t);
	void contaminated();

	// Special functions
	void noICU(int);
	bool test(int simTime, bool isolate);

private:
	/* Variables are based on time (days) */
	int _infected;
	int _symptomatic;
	bool _hasSymptoms;
	int _severity;
	int _diagnosed;
	int _hospitalized;
	int _isolated;
	int _icu;
	int _dead;
	int _cured;
};

