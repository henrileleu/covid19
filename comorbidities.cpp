#include "pch.h"
#include "comorbidities.h"


comorbidities::comorbidities()
{
}

comorbidities::comorbidities(int age, int sex, vlsRandGenerator & rnd, const Parameters & p)
{

	// Table for comorbidiy rate is 
	// 18-34 35-44 45-54 55-64 65-74 74+
	int index(5);
	if (age < 35) index = 0;
	else if (age < 45) index = 1;
	else if (age < 55) index = 2;
	else if (age < 65) index = 3;
	else if (age < 75) index = 4;
	
	// Go through all comorbidities
	for (int i = 0; i < nComorbidities; ++i)
	{
		if (rnd() < p(ComorbidityRate, index + sex * 6 + i * 6 * 2)) _list[i] = true; else _list[i] = false;
	}

}


comorbidities::~comorbidities()
{
}

bool comorbidities::getComorbidity(int i) const
{
	return _list[i];
}