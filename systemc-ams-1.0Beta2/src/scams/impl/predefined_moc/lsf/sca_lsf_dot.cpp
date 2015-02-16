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

  sca_lsf_dot.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.01.2010

   SVN Version       :  $Revision: 1102 $
   SVN last checkin  :  $Date: 2011-01-02 23:18:22 +0100 (Sun, 02 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_dot.cpp 1102 2011-01-02 22:18:22Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/lsf/sca_lsf_dot.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_lsf
{
sca_dot::sca_dot(sc_core::sc_module_name, double k_) :
	x("x"), y("y"), k("k", k_)
{
	first_call=true;
}

const char* sca_dot::kind() const
{
	return "sca_lsf::sca_dot";
}

void sca_dot::matrix_stamps()
{
	if(first_call && !sca_ac_analysis::sca_ac_is_running())
	{
		//y=0
		B(y,y) = -1.0;
	}
	else
	{
		B(y,y) = -1.0;
		A(y,x) =  k;
	}


    add_post_solve_method(SCA_VMPTR(sca_dot::d0_init));
}

void sca_dot::d0_init()
{
    B(y,y) = -1.0;
    A(y,x) =  k;
    first_call=false;

    request_reinit();

    remove_post_solve_method(SCA_VMPTR(sca_dot::d0_init));
}

} //namespace sca_eln

