#pragma once
/* Class that represents the contacts between individuals */
class link
{
public:
	link();
	link(individuals * i1, int duration, int distance, int type);
	~link();
	individuals * connected(int simTime, int) const;
	individuals * contactBetween(int, int) const;
	bool ChangeActiveState();
	int ChangeActiveState(bool);
	int ChangeActiveState(bool, int);
	std::array<int, 2> getProperties() const;
	int getType() const;
	int getTime() const;
	void setSimTime(int);
	individuals * getPair() const;
	void contaminated();

protected:
	// **** Variables :			******** //
	// - pair of individuals
	// - Time
	// - Frequency
	// - Probabilist
	// - Duration
	// - Distance
	// - Active
	// - Risk 
	std::array<int, 7> _frequency;
	int _type;
private:
	individuals * _pair;
	int _duration; // in Min
	int _distance; // in m
	bool _active;
	int _risk;
	int _simTime;
	bool _contaminated;
	
};

