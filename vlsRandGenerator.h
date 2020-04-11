#pragma once
#include <mkl.h>
#include <vector>

class vlsRandGenerator
{
public:
	vlsRandGenerator(VSLStreamStatePtr stream);
	~vlsRandGenerator();
	float operator() ();
	int operator() (int);
	int * operator() (int, int);
	int * operator() (int, int, int);
	float gamma(float alpha, float beta) const;
	float weibull(float alpha, float beta) const;
	int poisson(float lambda) const;
	float normal(float alpha, float sigma) const;
	int uniform();
	const static unsigned size = 2000;
private:
	float r[size];
	int ri[size];
	int index;
	int indexi;
	int maxi;
	VSLStreamStatePtr stream;

	// Generate sequence
	void gen();
	void geni(int);
};

