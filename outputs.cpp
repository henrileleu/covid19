#include "pch.h"
#include "outputs.h"


outputs::outputs(std::string s): root(s), name("default.csv")
{
}

void outputs::setFile(std::string s)
{
	name = s;
	std::string r2f = root + "\\" + s;
	std::fstream fs;
	fs.open(r2f, std::fstream::out);
	fs.close();
}

outputs::~outputs()
{
}

void outputs::AgePyramide(std::vector<location> &l, int nThreads)
{
	tbb::task_group g;

	// Initiate result array
	std::array<int, 200 * nFamilyType> a;
	for (int i = 0; i < 200 * nFamilyType; i++) a[i] = 0;

	std::vector<std::array<int, 200 * nFamilyType>> resultsAge;
	for (int k = 0; k < nThreads; ++k)
	{
		resultsAge.push_back(a);
	}
	

	tbb::concurrent_vector<family> * f;
	// Loop locations
	for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
	{
		f = location->getFamilies();

		// Loop families and count, using multithreads
		size_t i(0), j(f->size());
		while (i < j)
		{
			for (int k = 0; k < nThreads; ++k)
			{
				std::array<int, 200 * nFamilyType> * resultsAgeThread = &(resultsAge[k]);

				g.run(
					[=] {

					// Get family members
					std::vector<std::array<int, 2>> c = (*f)[i].caracteristics();
					int type = (*f)[i].getType() - 1;
					for (int i = 0; i < c.size(); i++) {
						int pos(c[i][0] + 100 * c[i][1] + 200 * type);
						resultsAgeThread->at(pos)++;
					}
				});
				i++;
				if (i >= j) break;
				g.wait();
			}
		}
		
	}

	// assemble
	for (int k = 0; k < nThreads; ++k)
	{
		for (int i = 0; i < 200 * nFamilyType; i++) a[i] += resultsAge[k][i];
	}

	// Now put it out there
	std::string r2f = root + "\\" + name;
	std::fstream fs;
	fs.open(r2f, std::fstream::app);

	fs << "Type" << "," << "Age" << "," << "Female" << "," << "Male" << std::endl;
	for (int j = 0; j < nFamilyType; j++) for (int i = 0; i < 100; i++) fs << j << "," << i << "," << a[i + 200 * j] << "," << a[i + 100 + 200 * j] << std::endl;

	fs.close();
}

void outputs::printFriendLinks(std::vector<location>&l, int nThreads)
{
	tbb::task_group g;


	// Initiate result array
	std::vector<std::vector<int>> nodes;
	std::vector<std::vector<std::array<int,2>>> links;
	
	for (int k = 0; k < nThreads; ++k)
	{
		std::vector<int> a;
		std::vector<std::array<int, 2>> b;
		nodes.push_back(a);
		links.push_back(b);
	}

	// Loop locations
	tbb::concurrent_vector<family> * f;
	int size;
	for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
	{
		size = location->getSize();
		f = location->getFamilies();
	
		size_t i(0), j(f->size());
		while (i < j)
		{
			for (int k = 0; k < nThreads; ++k)
			{
				std::vector<int> * nodesThread = &(nodes[k]);
				std::vector<std::array<int, 2>> * linksThread = &(links[k]);

				g.run(
					[=] {
					
					std::array<int, 2> p1 = (*f)[i].getPosition();

					// Add node
					int a(p1[0] * size + p1[1]);
					nodesThread->emplace_back(a);

					tbb::concurrent_vector<family *> friends = (*f)[i].getFriends();
					for (int j = 0; j < friends.size(); j++)
					{
						std::array<int, 2> p2 = (friends[j])->getPosition();
						linksThread->emplace_back(std::array<int, 2>({ p1[0] * size + p1[1], p2[0] * size + p2[1] }));
					}
					
				});
				i++;
				if (i >= j) break;
				g.wait();
			}
		}

	}

	// Now put it out there
	std::string r2f = root + "\\" + name;
	std::fstream fs;
	fs.open(r2f, std::fstream::app);
	
	// Nodes
	fs << "{ \n  \"nodes\" : [\n";

	fs << "{\"name\":\"" << nodes[0][0] << "\"}";

	for (int k = 0; k < nThreads; ++k)
	{
		for (int i = 0; i < nodes[k].size(); ++i)
		{
			if (k == 0 && i == 0) continue;
			fs << ",\n{\"name\":\"" << nodes[k][i] << "\"}";
		}
	}

	fs << "  \n],\n";

	//Links
	fs << "  \"links\" : [\n";

	if (links[0].size() != 0) {

		fs << "{\"source\":" << links[0][0][0] << ",\"target\":" << links[0][0][1] << ",\"value\":\"\",\"col\":\"link-line\"}";

		for (int k = 0; k < nThreads; ++k)
		{
			for (int i = 0; i < links[k].size(); ++i)
			{
				if (k == 0 && i == 0) continue;
				fs << ",\n{\"source\":" << links[k][i][0] << ",\"target\":" << links[k][i][1] << ",\"value\":\"\",\"col\":\"link-line\"}";
			}
		}
	}

	fs << "  ]\n}\n";


	fs.close();


}

std::array<float, max_days> outputs::infectionCurve(std::vector<location>&l)
{
	// Init
	int population(0);
	std::array<int, max_days> r;
	for (int i = 0; i < max_days; i++)
	{
		r[i] = 0;
	}

	// Go through loc
	for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
	{
		population += (*location).getPopSize();
		std::array<std::array<int, max_days>, 200> _diagnosed = (*location).getTracking(1);
		for (int i = 0; i < max_days; i++) {
			int a(0);
			for (int j = 0; j < 200; j++)
			{
				a += _diagnosed[j][i];
			}
			r[i] += a;
		}
	}

	// Estimated the rate per 100,000
	std::array<float, max_days> fr;
	for (int i = 0; i < max_days; i++)
	{
		fr[i] = static_cast<float>(r[i] * 100000 / population);
	}

	return fr;
}

void outputs::printTracking(std::vector<location> & l)
{
	for (int k = 0; k < 7; k++)
	{
		std::array<std::array<int, max_days>, 200> r;
		for (int j = 0; j < 200; j++) for (int i = 0; i < max_days; i++) r[j][i] = 0;
		int population(0);

		// Go through loc
		for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
		{
			population += (*location).getPopSize();
			std::array<std::array<int, max_days>, 200> _a = (*location).getTracking(k);
			for (int i = 0; i < max_days; i++) {
				for (int j = 0; j < 200; j++)
				{
					r[j][i] += _a[j][i];
				}
			}
		}

		std::array<std::array<float, max_days>, 200> rf;
		for (int j = 0; j < 200; j++) for (int i = 0; i < max_days; i++) rf[j][i] = 100000.0f * static_cast<float>(r[j][i]) / static_cast<float>(population);

		// Now put it out there
		std::string r2f = root + "\\" + name;
		std::fstream fs;
		fs.open(r2f, std::fstream::app);
		
		if (k == 0) { for (int j = 0; j < 100; j++) fs << "," << "0" ; 	for (int j = 100; j < 200; j++) fs << "," << "1"; fs << std::endl;
		}
		
		if (k == 0) { for (int j = 0; j < 100; j++) fs << "," << j ; 	for (int j = 0; j < 100; j++) fs << "," << j; fs << std::endl;
		}
		for (int i = 0; i < max_days; i++)
		{
			fs << i ;
			for (int j = 0; j < 200; j++) fs << "," << rf[j][i] ;
			fs << std::endl;
		}

		fs.close();
	}
}

void outputs::estimateR0(std::vector<location>& l, int stop)
{
	float a(0.0f), n(0.0f);
	for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
	{

		std::vector<individuals *> infected = (*location).getInfected();
		
		for (std::vector<individuals *>::iterator i = infected.begin(); i < infected.end(); i++)
		{
			if ((*i)->infectionTime() <= stop) {
				int b((*i)->getInfectedIndividuals());
				if (b > 0) { a += b; n++; }
			}
		}
	}

	std::cout << "R0 at " << stop << " : " << (a / n) << "\n";

}

void outputs::overMortality(std::vector<location>& l, int, Parameters & p)
{
	int nThreads(1);
	std::vector<VSLStreamStatePtr> stream;
	std::vector<vlsRandGenerator> RandomGenerator; 
	stream.resize(nThreads);
	vslNewStream(&(stream[0]), VSL_BRNG_MCG31, 200882);

	for (int i = 1; i < nThreads; ++i)
	{
		vslCopyStream(&(stream[i]), stream[0]);

	}
	for (int i = 0; i < nThreads; ++i)
	{
		vslLeapfrogStream(stream[i], i, nThreads);
		RandomGenerator.emplace_back(vlsRandGenerator(stream[i]));
	}

	std::array<float, 200> corona, natural;
	for (int i = 0; i < 200; i++) {
		corona[i] = 0.0f; natural[i] = 0.0f;
	}

	for (std::vector<location>::iterator location = l.begin(); location < l.end(); location++)
	{

		std::vector<individuals *> infected = (*location).getInfected();

		for (std::vector<individuals *>::iterator i = infected.begin(); i < infected.end(); i++)
		{
			if ((*i)->isDead(max_days))
			{
				int age((*i)->getAge());
				int agec(age + (*i)->getNumberofComorbidities() * 5);
				int sex((*i)->getSex());
				if (RandomGenerator[0]() < p(prMortMale + (agec>100?100:agec) + 100 * sex)) natural[age + 100 * sex]++; else corona[age + 100 * sex]++;

			}

		}
	}

	// Now put it out there
	std::string r2f = root + "\\" + name;
	std::fstream fs;
	fs.open(r2f, std::fstream::app);

	for (int i = 0; i < 200; i++)
	{
		fs << (i < 100 ? 1 : 0) << "," << i % 100 << "," << corona[i] << "," << natural[i] << std::endl;
	}

	fs.close();
	

}

