#pragma once

/* 
*** Class to gibe individuals comorbidities ***
Includes:
- Hypertention: HTA
- Smocking: Smoker
- Diabeties: Diabeties
- Chronic Obtructive Pulmonoary Disease: BPCO
- Coronary Dieases: Coronary Disease
*/
class comorbidities
{
public:
	// Default contructor: Does nothing 
	comorbidities();

	/* Constructor
	Takes 4 parameters: 
	- Age = Individual's age
	- Sex: Individual's sex
	- Rnd: RandomGenerator (see vlsRandGenerator class)
	- Parameters (see Parameters class)
	*/
	comorbidities(int age, int sex, vlsRandGenerator & rnd, const Parameters & p);

	// Default destructor
	~comorbidities();

	// List of comorbidities included
	static enum comorbiditiesEnum : int {
		cHTA = 0,
		cSmoker = 1,
		cDiabetes = 2,
		cBPCO = 3,
		cCoronaryDisease = 4
	};
	// Number of comorbidities
	static const int nComorbidities = 5;

	/* Function to check if individual has a given comorbidity
	Takes an integer corresponding to the comorbiditiesEnum and return a boolean if individual has comorbidity
	*/
	bool getComorbidity(int) const;
private:

	/* List of comordities */
	std::array<bool, nComorbidities> _list;
};

