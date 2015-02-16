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

  sca_parameter_base.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.08.2009

   SVN Version       :  $Revision: 1055 $
   SVN last checkin  :  $Date: 2010-10-26 09:50:55 +0200 (Tue, 26 Oct 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_parameter_base.cpp 1055 2010-10-26 07:50:55Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc>
#include "scams/core/sca_parameter_base.h"


namespace sca_core
{

const char* sca_parameter_base::kind() const
{
	return "sca_core::sca_parameter_base";
}

void sca_parameter_base::lock()
{
	locked_flag=true;
}

void sca_parameter_base::unlock()
{
	locked_flag=false;
	unlock_flag=true;
}

bool sca_parameter_base::is_locked() const
{
	return locked_flag;
}

sca_parameter_base::sca_parameter_base()
{
	locked_flag=false;
	unlock_flag=false;
}


sca_parameter_base::sca_parameter_base(const char* nm): ::sc_core::sc_object(nm)
{
	locked_flag=false;
	unlock_flag=false;
}

sca_parameter_base::~sca_parameter_base()
{
}


// Disabled
sca_parameter_base::sca_parameter_base(const sca_core::sca_parameter_base&)
{
}

//disabled
sca_core::sca_parameter_base& sca_parameter_base::operator=(const sca_core::sca_parameter_base& p)
{
	return const_cast<sca_core::sca_parameter_base&>(p);
}

std::ostream& operator << (std::ostream& str, const sca_core::sca_parameter_base& par)
{
	par.print(str);
	return str;
}


}

