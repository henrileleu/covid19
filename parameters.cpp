#include "pch.h"
#include "parameters.h"
using namespace std;

Parameters::Parameters()
{
}

Parameters::Parameters(std::array<float, n_vars> m)
{
	variables = m;

}

Parameters::Parameters(std::string file, char delim)
{
	//temporay variables
	string line;

	//Open variable file for reading
	ifstream myfile;


	myfile.open(file);

	int i = 0;
	//read file by line & transfer to a vector that will hold the values
	while (getline(myfile, line).good()) {

		if (line == "") { continue; }
		//put in temp vector
		variables[i] = stof(line);
		++i;

	}
	//close file when job is done
	myfile.close();

	// Do the precalculations
	familySingleMaleTable = precalculate(familySingleMale);
	familySingleFemaleTable = precalculate(familySingleFemale);
	familyCoupleOnlyTable = precalculate(familyCoupleOnly);
}




Parameters::~Parameters()
{
}

float Parameters::operator() (int name, int namesub) const
{


	return variables[name + namesub];


}

float Parameters::operator() (int name) const
{


	return variables[name];

}


void Parameters::set(int name, int namesub, float value)
{

	variables[name + namesub] = value;


}

void Parameters::set(int name, float value)
{
	variables[name] = value;

}

std::array<float, n_vars> Parameters::copy() const
{
	return variables;
}

void Parameters::set(std::array<float, n_vars> v)
{
	variables = v;
}

const std::array<float, n_vars>* Parameters::getMapPtr()
{
	return &variables;
}

int Parameters::getPrecalculated(int table, float p) const
{
	unsigned int a(static_cast<unsigned int>(p * _precision + 0.5));
	a = a >= _precision ? 99 : a;
	switch (table)
	{
	case familySingleMale:
		return familySingleMaleTable[a];
		break;
	case familySingleFemale:
		return familySingleFemaleTable[a];
		break;
	case familyCoupleOnly:
		return familyCoupleOnlyTable[a];
		break;
	default:
		return 0;
	}
}

std::array<int, Parameters::_precision> Parameters::precalculate(int var)
{
	std::array<int, _precision> result = {};
	// Calculate the probabilities at each age for a 100 age group 
	// Then reverse the table
	float r[_precision];
	float age;
	for (int i = 20; i < 100; i++)
	{
		r[i] = 0;
		for (int j = 0; j < 6; j++)
		{
			age = static_cast<float>(i / 100.0f);
			r[i] += variables[var + j] * static_cast<float>(pow(age, 5-j)); // 5 degree polynome
		}
	}

	// Reverse
	int j(0);
	for (int i = 20; i < _precision; i++)
	{
		while (j <= static_cast<int>(r[i] * _precision + 0.5)) { if (j >= _precision) break;  result[j] = i; j++; }
	}
	// Complete
	if (j>0) for (int i = j; i < _precision; i++) result[i] = result[j-1];

	return result;
}
