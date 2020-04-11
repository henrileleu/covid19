#pragma once
class outputs
{
public:
	outputs(std::string s);
	void setFile(std::string s);
	~outputs();

	// Outputs
	void AgePyramide(std::vector<location> &, int);
	void printFriendLinks(std::vector<location> &, int);
	std::array<float,max_days> infectionCurve(std::vector<location> &);
	void printTracking(std::vector<location> &);
	void estimateR0(std::vector<location> &, int);
	void overMortality(std::vector<location> &, int, Parameters & p);

private:
	std::string root;
	std::string name;

	static int const nFamilyType = 5;
};

