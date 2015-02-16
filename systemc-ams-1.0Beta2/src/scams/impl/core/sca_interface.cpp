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

 sca_interface.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 13.05.2009

 SVN Version       :  $Revision: 1039 $
 SVN last checkin  :  $Date: 2010-08-17 11:11:52 +0200 (Tue, 17 Aug 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_interface.cpp 1039 2010-08-17 09:11:52Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include <systemc>
#include "scams/core/sca_interface.h"
#include "scams/core/sca_time.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/core/sca_object_manager.h"

using namespace sca_core;
using namespace sca_core::sca_implementation;

namespace sca_core
{

//may be not required
sca_interface::sca_interface()
{
	sca_core::sca_implementation::sca_get_curr_simcontext()->get_sca_object_manager()->insert_interface(
			this);
}

//disabled copy constructor
sca_interface::sca_interface(const sca_core::sca_interface&)
{
}

//disabled operator
sca_interface& sca_interface::operator=(const sca_core::sca_interface&)
{
	return *this;
}

}

