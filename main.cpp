// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	// Check number of arguments
	if (argc != 4) {
		printf("Incorrect number of parameters for function, %d passed", argc);  return 1;
	}

	cout << "Starting simulation\n";

	// Get parameters
	string _root(argv[1]);
	int size = atoi(argv[2]);
	long n = atol(argv[3]);
	
	int nThreads(tbb::task_scheduler_init::default_num_threads());
	
		// Define paths &  Parameters
	string root = _root;
	Parameters p(root + "\\params.csv", ',');

	outputs _outputs(root);

	cout << "Parameters loaded\nCreating the world\n";

	// Create the world
	world w(size,n,p);

	std::cout << "Outputing...";

	_outputs.setFile("ageStructure.csv");
	_outputs.AgePyramide(*w.getLocations(), nThreads);

	/*_outputs.setFile("visualization\\firends.json");
	_outputs.printFriendLinks(*w.getLocations(), nThreads);*/

	_outputs.setFile("curve.csv");
	std::array<float, max_days> a = _outputs.infectionCurve(*w.getLocations());
	_outputs.printTracking(*w.getLocations());

	_outputs.estimateR0(*w.getLocations(), 30);
	_outputs.estimateR0(*w.getLocations(), 60);
	_outputs.estimateR0(*w.getLocations(), 90);
	_outputs.estimateR0(*w.getLocations(), 120);
	_outputs.estimateR0(*w.getLocations(), 180);

	_outputs.setFile("overmortality.csv");
	_outputs.overMortality(*w.getLocations(), 1, p);

	/*string s;
	std::cin >> s;*/
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
