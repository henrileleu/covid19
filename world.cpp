#include "pch.h"
#include "world.h"

// For now only two tips of location are implemented
// - Cities higher density
// - Suburbs / Countryside lower density
world::world(int size, long int n, Parameters & p) : _size(size), _n(n), nThreads(tbb::task_scheduler_init::default_num_threads()), _p(p)
{

	std::cout << "Initiating random numbers\n";

	// Initiation Random Number generators
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


	// Estimated numbers and density
	std::array<int, 2> locationsN;
	locationsN[0] = ((size + 1) / 2)*((size + 1) / 2); // Cities
	locationsN[1] = (size*size) - locationsN[0];

	std::array<float, 2> densities;
	densities[0] = p(var::cityDensity);
	densities[1] = n * p(var::urban) / (n * p(var::countryDensity) - n * (1 - p(var::urban)) * p(var::cityDensity));

	// Location will form a checker
	//   1 2 3 4 5 6
	// 1 C S C S C S
	// 2 S S S S S S
	// 3 C S C S C S
	// 4 S S S S S S
	// 5 C S C S C S
	// 6 S S S S S S

	std::cout << "Creating the locations\n";

	// Cities are on odd line et row
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if ((j+1) % 2 == 1 && (i+1) % 2 == 1) // cities
			{
				_list.emplace_back(n / locationsN[0], densities[0], i, j, true, RandomGenerator, nThreads, p);
			}
			else // Countryside
			{
				_list.emplace_back(n / locationsN[1], densities[1], i, j, false, RandomGenerator, nThreads, p);
			}
		}
	}
	
	for (std::vector<location>::iterator i = _list.begin(); i > _list.end(); ++i)
	{
		std::array<int, 2> l = (*i).getLocation();
		for (int j = 0; j < 8; ++j)
		{
			// Outside of boundaries
			if (l[0] + neighborPositions[j][0] < size || l[0] + neighborPositions[j][0] > size || l[1] + neighborPositions[j][1] < size || l[1] + neighborPositions[j][1] > size)
			{
				// Set to null
				(*i).setNeighbor(nullptr,j);

			}
			else
			{
				(*i).setNeighbor(&(*i), j);
			}
		}
	}

	// Intercity link to be defined here
	if (_list.size() > 1)
	{
		for (unsigned int i = 0; i < _list.size(); i++)
		{

			for (unsigned int j = i + 1; j < _list.size(); j++)
			{
				
				// To do;


			}
		}
	}

	// Word link
	individuals patientZero;
	patientZero.setPatientZero();
	int i(0);
	while (i < max_days)
	{
		
		int nInfected = (i==0?1:RandomGenerator[0].poisson(p(AvgInternationalContaminated)));
		int * r0 = RandomGenerator[0](static_cast<int>(_list.size()), nInfected); // Choose a city

		for (int j = 0; j < nInfected; ++j)
		{
			// Choose a random family in location
			int n = _list[r0[j]].getSize();
			int r1 = RandomGenerator[0].uniform()%n;

			// Selected individual to infect
			individuals * contaminatedInd = _list[r0[j]].getHead(r1);

			link l = foreignLink(contaminatedInd);
			patientZero.addLink(l, i);

		}

		
		i += RandomGenerator[0].poisson(p(AvgDelaiInternational));


	}


	patientZero.sortOneTimeLink();

	int stopContainement(INT_MAX);
	// Run simulation
	for (int i = 0; i < max_days; i++)
	{
		std::cout << "Day : " << i;

		for (int j = 0; j < _list.size(); j++)
		{
			// First look at the non imported cases
			_list[j].nextDay(i, RandomGenerator, p);

			// Then imported
			std::vector<individuals *> internationalContamination = _list[j].incidence(&patientZero, i, RandomGenerator[0], p);
			_list[j].addNewlyInfected(internationalContamination, i, RandomGenerator, p);

			// Tracking
			float diag(_list[j].getNumberDiagnosed(i));

			std::cout << " ____ Diag : " << diag << std::endl;
			_list[j].trackCases(i, RandomGenerator, p);

			// Containement
			if (diag > p(ContainementThreshold) && stopContainement == INT_MAX)
			{
				stopContainement = i;
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lGoingOutTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lForeign);
				_list[j].disableTracking();
				std::cout << "Containement\n";
			}
			/* Social dist & Trackin */
			/*_list[j].setSocialDistancing(true);
			
			_list[j].enableTracking(true);
			_list[j].setTrackingCapability(lWorkTransport, true);
			_list[j].setTrackingCapability(lGoingOutTransport, true);
			_list[j].setTrackingCapability(lShooping, true);
			/* ************* Avril ************ */
			/*if (i == stopContainement + 16 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
				
			}
			
			/* ************ Progressif ******************/
			/*if (i == stopContainement + 10 * 7)
			{

				_list[j].containement(70, lForeign);
				_list[j].containement(70, lFriend);
				_list[j].containement(70, lWork);
				_list[j].containement(70, lSchool);
				_list[j].containement(70, lShooping);
				_list[j].containement(70, lEvent);
				_list[j].containement(70, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			
			if (i == stopContainement + 18 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			
			/* Prog 40 puis 65*/
			/*if (i == stopContainement + 10 * 7)
			{
				_list[j].containement(65, lWorkTransport);
				_list[j].containement(65, lWork);
				_list[j].containement(65, lSchool);
				_list[j].containement(65, lShooping);
				std::cout << "Containement stopped\n";
			}

			if (i == stopContainement + 18 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			/* Alternated*/
			/*if (i == stopContainement + 10 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 11 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 13 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 14 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 16 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 17 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 19 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 20 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 23 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 24 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 26 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 27 * 7)
			{
				_list[j].containement(0, lForeign);
				_list[j].containement(0, lFriend);
				_list[j].containement(0, lWorkTransport);
				_list[j].containement(0, lWork);
				_list[j].containement(0, lSchool);
				_list[j].containement(0, lShooping);
				_list[j].containement(0, lEvent);
				_list[j].containement(0, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 29 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			
			/* Tracking + Testing */
			/*if (i == stopContainement + 10 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			if (i == stopContainement + 6 * 7)
			{
				_list[j].setSelfTesting(true);
				_list[j].enableTracking(true);
				_list[j].setTrackingCapability(lWorkTransport, true);
				_list[j].setTrackingCapability(lGoingOutTransport, true);
				_list[j].setTrackingCapability(lShooping, true);
				// ** Social Distancing
				//_list[j].setSocialDistancing(true);
			}
			if (i == stopContainement + 10 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			

			/*if (i == stopContainement + 22 * 7)
			{
				_list[j].containement(65, lForeign);
				_list[j].containement(65, lWorkTransport);
				_list[j].containement(65, lWork);
				_list[j].containement(65, lSchool);
				_list[j].containement(65, lShooping);
				_list[j].containement(65, lEvent);
				_list[j].containement(65, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}


			if (i == stopContainement + 34 * 7)
			{
				_list[j].containement(INT_MAX, lForeign);
				_list[j].containement(INT_MAX, lFriend);
				_list[j].containement(INT_MAX, lWorkTransport);
				_list[j].containement(INT_MAX, lWork);
				_list[j].containement(INT_MAX, lSchool);
				_list[j].containement(INT_MAX, lShooping);
				_list[j].containement(INT_MAX, lEvent);
				_list[j].containement(INT_MAX, lGoingOutTransport);
				std::cout << "Containement stopped\n";
			}
			/* Prog 40 puis 65 */
				if (i == stopContainement +3 * 7)
						{
							_list[j].setSelfTesting(true);
							//_list[j].enableTracking(true);
							//_list[j].setTrackingCapability(lWorkTransport, true);
							//_list[j].setTrackingCapability(lGoingOutTransport, true);
							//_list[j].setTrackingCapability(lShooping, true);
							_list[j].containement(60, lForeign);
							_list[j].containement(60, lFriend);
							_list[j].containement(60, lWorkTransport);
							_list[j].containement(60, lWork);
							_list[j].containement(60, lSchool);
							_list[j].containement(60, lShooping);
							_list[j].containement(60, lEvent);
							_list[j].containement(60, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

				if (i == stopContainement + 3 + 11 * 7)
				{
					_list[j].containement(INT_MAX, lForeign);
					_list[j].containement(INT_MAX, lFriend);
					_list[j].containement(INT_MAX, lWorkTransport);
					_list[j].containement(INT_MAX, lWork);
					_list[j].containement(INT_MAX, lSchool);
					_list[j].containement(INT_MAX, lShooping);
					_list[j].containement(INT_MAX, lEvent);
					_list[j].containement(INT_MAX, lGoingOutTransport);
					std::cout << "Containement stopped\n";
				}
						

						/* Scenario collège des économistes */
						
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].setSelfTesting(true);
							_list[j].enableTracking(true);
							_list[j].setTrackingCapability(lWorkTransport, true);
							_list[j].setTrackingCapability(lGoingOutTransport, true);
							_list[j].setTrackingCapability(lShooping, true);
							_list[j].setSocialDistancing(true);
						}*/
						/* Scénario 1A F*/
						/*if (i == stopContainement + 11 * 7)
						{
							_list[j].containement(0, lForeign);
							_list[j].containement(0, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(0, lEvent);
							_list[j].containement(0, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}


						/* Scénario 1B O*/
						/*if (i == stopContainement + 11 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(INT_MAX, lShooping);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

						/* Scénario 1'A F*/
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(0, lForeign);
							_list[j].containement(0, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(0, lEvent);
							_list[j].containement(0, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}


						/* Scénario 1'B O*/
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

						/* Scénario 2A */
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(0, lForeign);
							_list[j].containement(0, lFriend);
							_list[j].containement(65, lWorkTransport);
							_list[j].containement(65, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(0, lEvent);
							_list[j].containement(0, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

						/* Scénario 2B */
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(65, lForeign);
							_list[j].containement(65, lFriend);
							_list[j].containement(65, lWorkTransport);
							_list[j].containement(65, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(65, lEvent);
							_list[j].containement(65, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

						/* Scénario 2A */
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(0, lForeign);
							_list[j].containement(0, lFriend);
							_list[j].containement(50, lWorkTransport);
							_list[j].containement(50, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(0, lEvent);
							_list[j].containement(0, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}

						/* Scénario 2B */
						/*if (i == stopContainement + 7 * 7)
						{
							_list[j].containement(50, lForeign);
							_list[j].containement(50, lFriend);
							_list[j].containement(50, lWorkTransport);
							_list[j].containement(50, lWork);
							_list[j].containement(13, lSchool);
							_list[j].containement(50, lEvent);
							_list[j].containement(50, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}
						if (i == stopContainement + 16 * 7)
						{
							_list[j].containement(INT_MAX, lForeign);
							_list[j].containement(INT_MAX, lFriend);
							_list[j].containement(INT_MAX, lWorkTransport);
							_list[j].containement(INT_MAX, lWork);
							_list[j].containement(INT_MAX, lSchool);
							_list[j].containement(INT_MAX, lEvent);
							_list[j].containement(INT_MAX, lGoingOutTransport);
							std::cout << "Containement stopped\n";
						}*/
		}
		
	}
	_list[0].printStuff();
}

world::~world()
{
	for (int i = 0; i < nThreads; ++i)
	{
		vslDeleteStream(&(stream[i]));
	}
}

std::vector<location>* world::getLocations()
{
	return &_list;
}


int world::getPosition(std::array<int, 2> coord)
{
	return coord[0] + _size * coord[1];
}