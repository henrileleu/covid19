#include "pch.h"
#include "vlsRandGenerator.h"


vlsRandGenerator::vlsRandGenerator(VSLStreamStatePtr stream) : stream(stream), index(0), indexi(0), maxi(0)
{
	gen();
}


vlsRandGenerator::~vlsRandGenerator()
{
}

float vlsRandGenerator::operator()()
{
	if (index >= size) {
		index = 0;
		gen();
	}
	return r[index++];
}

int vlsRandGenerator::operator()(int n)
{
	if (index >= size || maxi != n) {
		indexi = 0;
		geni(n);
	}
	return ri[indexi++];
}

int * vlsRandGenerator::operator()(int m, int n)
{
	return operator()(m, n, 0);
}

int * vlsRandGenerator::operator()(int m, int n , int s)
{
	viRngUniform(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER, stream, n, ri, s, m);
	return ri;
}


float vlsRandGenerator::weibull(float alpha, float beta) const
{
	float _r[1];
	vsRngWeibull(VSL_RNG_METHOD_WEIBULL_ICDF, stream, 1, _r, alpha, 0, beta);
	return _r[0];

}
float vlsRandGenerator::gamma(float alpha, float beta) const
{
	float _r[1];
	vsRngGamma(VSL_RNG_METHOD_GAMMA_GNORM, stream, 1, _r, alpha, 0, beta);
	return _r[0];
}

int vlsRandGenerator::poisson(float lambda) const
{
	int _r[1];
	viRngPoisson(VSL_RNG_METHOD_POISSON_PTPE, stream, 1, _r, lambda);
	return _r[0];
}
float vlsRandGenerator::normal(float alpha, float sigma) const
{
	float _r[1];
	vsRngGaussian(VSL_RNG_METHOD_UNIFORM_STD, stream, 1, _r, alpha, sigma);
	return _r[0];
}

int vlsRandGenerator::uniform()
{
	int _r[1];
	viRngUniform(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER, stream, 1, _r, 0, INT_MAX);
	return _r[0];
}

void vlsRandGenerator::gen()
{
	vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, size, r, 0, 1);
}

void vlsRandGenerator::geni(int n)
{
	viRngUniform(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER, stream, size, ri, 0, n);
}
