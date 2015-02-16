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

 sca_eln_node.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 10.11.2009

 SVN Version       :  $Revision: 977 $
 SVN last checkin  :  $Date: 2010-03-04 20:23:45 +0100 (Thu, 04 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_node.cpp 977 2010-03-04 19:23:45Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/predefined_moc/eln/sca_eln_node.h"
#include "scams/impl/core/sca_solver_base.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"

namespace sca_eln
{

sca_node::sca_node() :
	sca_conservative_signal(sc_core::sc_gen_unique_name("sca_eln_node"))
{
	reference_node = false;
	sca_ac_analysis::sca_implementation::sca_ac_register_add_eq_arc(this, &node_number, &reference_node);
}

sca_node::sca_node(const char* name_) :
	sca_conservative_signal(name_)
{
	reference_node = false;
	sca_ac_analysis::sca_implementation::sca_ac_register_add_eq_arc(this, &node_number, &reference_node);
}

const char* sca_node::kind() const
{
	return "sca_eln::sca_node";
}

long sca_node::get_node_number() const
{
	return sca_core::sca_implementation::sca_conservative_signal::get_node_number();
}

//Disabled not used
sca_node::sca_node(const sca_eln::sca_node&)
{
}

///////////////

bool sca_node::trace_init(
		sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.type = "Voltage";
	data.unit = "V";

	unsigned long nmodules = get_number_of_connected_modules();
	sca_core::sca_module** modules = get_connected_module_list();

	for (unsigned long i = 0; i < nmodules; ++i)
	{
		eln_module = dynamic_cast<sca_eln::sca_module*> (modules[i]);
		if (eln_module != NULL)
		{
			eln_module->get_sync_domain()->add_solver_trace(data);
			return true;
		}
	}

	return false;
}

//is called after cluster was calculated
void sca_node::trace(long id,
		sca_util::sca_implementation::sca_trace_buffer& trace_buffer)
{
	sca_core::sca_time ctime = eln_module->get_time();

	if (reference_node) //cumbersome reference node tracing
	{
		double gnd = 0.0;
		trace_buffer.store_time_stamp(id, ctime, gnd);
	}
	else
	{
		trace_buffer.store_time_stamp(id, ctime, eln_module->x(node_number));
	}
}

}
