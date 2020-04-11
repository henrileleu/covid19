#pragma once
#include "vars.h"
#include <string>
#include <iostream>
#include <fstream>
#include "stringSplit.h"


class Parameters {

public:

	Parameters();

	Parameters(std::array<float, n_vars>);

	Parameters(std::string file, char delim);

	~Parameters();

	float operator() (int name, int namesub) const;

	float operator() (int name) const;

	void set(int name, int namesub, float value);

	void set(int name, float value);

	std::array<float, n_vars> copy() const;

	void set(std::array<float, n_vars> variables);

	const std::array<float, n_vars> * getMapPtr();

	// get the precalculated
	int getPrecalculated(int table, float p) const;

	static const int _precision = 1000;

private:

	std::array<float, n_vars> variables;

	// Special precalculated variables
	std::array<int, _precision> precalculate(int var);
	std::array<int, _precision> familySingleMaleTable;
	std::array<int, _precision> familySingleFemaleTable;
	std::array<int, _precision> familyCoupleOnlyTable;
	
};
