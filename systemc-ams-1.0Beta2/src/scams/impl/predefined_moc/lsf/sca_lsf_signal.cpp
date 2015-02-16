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

  sca_lsf_signal.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.01.2010

   SVN Version       :  $Revision: 977 $
   SVN last checkin  :  $Date: 2010-03-04 20:23:45 +0100 (Thu, 04 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_signal.cpp 977 2010-03-04 19:23:45Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include <systemc-ams>
#include "scams/predefined_moc/lsf/sca_lsf_signal.h"
#include "scams/impl/core/sca_solver_base.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"

namespace sca_lsf
{

sca_signal::sca_signal() :
	sca_conservative_signal(sc_core::sc_gen_unique_name("sca_lsf_signal"))
{
	reference_node = false;
	sca_ac_analysis::sca_implementation::sca_ac_register_add_eq_arc(this, &node_number, &reference_node);
}

sca_signal::sca_signal(const char* name_) :
	sca_conservative_signal(name_)
{
	reference_node = false;
	sca_ac_analysis::sca_implementation::sca_ac_register_add_eq_arc(this, &node_number, &reference_node);
}

long sca_signal::get_node_number() const
{
	return sca_core::sca_implementation::sca_conservative_signal::get_node_number();
}


const char* sca_signal::kind() const
{
	return "sca_lsf::sca_signal";
}

//Disabled not used
sca_signal::sca_signal(const sca_lsf::sca_signal&)
{
}

///////////////

bool sca_signal::trace_init(
		sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.type = "-";
	data.unit = "-";

	unsigned long nmodules = get_number_of_connected_modules();
	sca_core::sca_module** modules = get_connected_module_list();

	for (unsigned long i = 0; i < nmodules; ++i)
	{
		lsf_module = dynamic_cast<sca_lsf::sca_module*> (modules[i]);
		if (lsf_module != NULL)
		{
			lsf_module->get_sync_domain()->add_solver_trace(data);
			return true;
		}
	}
	return false;
}

//is called after cluster was calculated
void sca_signal::trace(long id,
		sca_util::sca_implementation::sca_trace_buffer& trace_buffer)
{
	sca_core::sca_time ctime = lsf_module->get_time();

	trace_buffer.store_time_stamp(id, ctime, lsf_module->x(node_number));
}

}
