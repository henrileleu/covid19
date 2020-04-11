// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

// TODO: add headers that you want to pre-compile here
#include <array>
#include <vector>
#include <math.h>
#include <algorithm>  
#include <random>
#include <mutex>
#include "tbb/concurrent_vector.h"
#include "parameters.h"
#include "vlsRandGenerator.h"
#include "tbb/tbb.h"
#include "comorbidities.h"
#include "disease.h"
#include "individuals.h"
#include "link.h"
#include "location.h"
#include "family.h"
#include "eventLink.h"
#include "foreignLink.h"
#include "friendLink.h"
#include "intercityLink.h"
#include "sameRoofIntrafamilialLink.h"
#include "schoolLink.h"
#include "shoppingLink.h"
#include "transportLink.h"
#include "worktransportLink.h"
#include "gointOutTransportLink.h"
#include "workLink.h"
#include "world.h"
#include "outputs.h"
#endif //PCH_H
