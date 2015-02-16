/*****************************************************************************

    Copyright 2010
    Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.


   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sca_synchronization_layer.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 25.08.2009

   SVN Version       :  $Revision: 1108 $
   SVN last checkin  :  $Date: 2011-01-09 12:31:33 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_synchronization_layer.cpp 1108 2011-01-09 11:31:33Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/impl/synchronization/sca_synchronization_alg.h"
#include "scams/impl/synchronization/sca_synchronization_layer.h"


#include <iostream>
using namespace std;

namespace sca_core
{
namespace sca_implementation
{





sca_synchronization_layer::sca_synchronization_layer():  alg(NULL)
{
	alg=new sca_core::sca_implementation::sca_synchronization_alg();
}

/////////////////////////////////////////////////////////

sca_synchronization_layer::~sca_synchronization_layer()
{
	delete alg;
	alg=NULL;
}

/////////////////////////////////////////////////////////

void sca_synchronization_layer::registrate_solver_instance(sca_synchronization_obj_if* solver)
{
  solvers.push_back(solver);
}

/////////////////////////////////////////////////////////

void sca_synchronization_layer::initialize()
{
  alg->initialize(solvers);

#ifdef SCA_IMPLEMENTATION_DEBUG
 std::cout << "\t" << "Sysnchronization layer initialized: " << solvers.size() << std::endl << std::endl;
#endif
}

/////////////////////////////////////////////////////////


}
}
