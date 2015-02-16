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

 sca_module.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 15.05.2009

 SVN Version       :  $Revision: 1147 $
 SVN last checkin  :  $Date: 2011-02-06 15:12:47 +0100 (Sun, 06 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_module.cpp 1147 2011-02-06 14:12:47Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>

#include "scams/impl/core/sca_simcontext.h"
#include "scams/core/sca_module.h"
#include "scams/impl/util/tracing/sca_trace_file_base.h"
#include "scams/impl/core/sca_object_manager.h"
#include "scams/impl/core/sca_port_base.h"
#include "scams/impl/core/sca_solver_base.h"
#include "scams/impl/core/sca_globals.h"

//STL usage
#include <algorithm>
#include <sstream>


namespace sca_core
{

using namespace sca_implementation;


sca_module::sca_module()
{
	sync_domain = NULL;
	view_interface = NULL;

	timestep_set=sc_core::SC_ZERO_TIME;
    timestep_is_set=false;
    timestep_elaborated=false;

    allow_initialize_access_flag=false;
    allow_attributes_access_flag=false;

	sca_get_curr_simcontext()->get_sca_object_manager()->insert_module(this);
}

//////////////////////////////////////////////////////////////////

//destructor
sca_module::~sca_module()
{
	sca_get_curr_simcontext()->get_sca_object_manager()->remove_module(this);
}

//////////////////////////////////////////////////////////////////

const char* sca_module::kind() const
{
	return "sca_core::sca_module";
}


void sca_module::set_timestep(const sca_core::sca_time& tstep)
{
	if(timestep_elaborated)
	{
		std::ostringstream str;
		str << "The module timestep must set before end_of_elaboration for module: "
		    << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	timestep_set=tstep;
	timestep_is_set=true;
}

void sca_module::set_timestep(double tstep, sc_core::sc_time_unit unit)
{
	if(timestep_elaborated)
	{
		std::ostringstream str;
		str << "The module timestep must set before end_of_elaboration for module: "
		    << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	timestep_set=sca_core::sca_time(tstep,unit);
	timestep_is_set=true;
}

unsigned long sca_module::register_port(sca_port_base* port)
{
	unsigned long n = port_list.size();

	port_list.push_back(port);
	return n;
}

//////////////////////////////////////////////////////////////////

sca_module::sca_port_base_listT& sca_module::get_port_list()
{
	return port_list;
}

//////////////////////////////////////////////////////////////////

void sca_module::elaborate()
{
	if (view_interface == NULL)
	{
		std::ostringstream str;
		str << "Error: " << name()
				<< " a sca_module must be associated with a concrete view!"
				<< std::endl;
		str << "It is not allowed to instantiate the sca_module base class."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	//elaborate all ports (register bound interfaces)
	for (sca_port_base_list_iteratorT pit = get_port_list().begin(); pit
			!= get_port_list().end(); ++pit)
	{
		(*pit)->elaborate_port();
	}

	const std::vector<sc_core::sc_object*>& cobj(this->get_child_objects());

	for(unsigned long i=0;i<cobj.size();i++)
	{
		if(dynamic_cast<sca_core::sca_module*>(cobj[i]) ||
		   dynamic_cast<sca_core::sca_interface*>(cobj[i]))
		{
			std::ostringstream str;
			str << "A " << this->kind() << " ( " << this->name()
			    << " ) can't have child: " <<  cobj[i]->kind()
		        << "( " << cobj[i]->name() << " )  see LRM clause 4.1";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}
}


/////////////////////////////////////////////////////////////////

sca_solver_base* sca_module::get_sync_domain()
{
	return sync_domain;
}

const bool* sca_module::get_allow_processing_access_flag()
{
	if(get_sync_domain()==NULL) return NULL;

	return get_sync_domain()->get_allow_processing_access_flag_ref();
}


bool sca_module::get_allow_initialize_access_flag()
{
	return allow_initialize_access_flag;
}

bool sca_module::get_allow_attributes_access_flag()
{
	return allow_attributes_access_flag;
}



/////////////////////////////////////////////////////////////////

::sc_core::sc_time sca_module::sca_get_time() const
{
	if (sync_domain == NULL)
		return NOT_VALID_SCA_TIME();
	return sync_domain->get_current_time();
}

/////////////////////////////////////////////////////////////////

::sca_core::sca_time sca_module::get_timestep() const
{
	sca_core::sca_time rett;
	if (sync_domain == NULL) rett=NOT_VALID_SCA_TIME();
	else  rett=sync_domain->get_current_period();

	if(rett==NOT_VALID_SCA_TIME())
	{
		std::ostringstream str;
		str << "it is not allowed to call get_timestep before the elaboration has been finished ";
		str << "in module: " << name();

		SC_REPORT_ERROR("SysemC-AMS",str.str().c_str());
	}

	return rett;
}

/////////////////////////////////////////////////////////////////

long sca_module::sca_next_max_time(::sc_core::sc_time mtime)
{
	if (sync_domain == NULL)
		return -1;
	return sync_domain->set_max_start_time(mtime);
}

/////////////////////////////////////////////////////////////////

long sca_module::sca_next_max_time_step(::sc_core::sc_time ptime)
{
	if (sync_domain == NULL)
		return -1;
	return sync_domain->set_max_period(ptime);
}

/////////////////////////////////////////////////////////////////

::sc_core::sc_time sca_module::sca_synchronize_on_event(
		const ::sc_core::sc_event& ev)
{
	if (sync_domain == NULL)
		return ::sc_core::SC_ZERO_TIME;
	return sync_domain->register_activation_event(ev);
}

::sc_core::sc_time sca_module::sca_synchronize_on_event(
		const sca_port_base& iport)
{
	const ::sc_core::sc_interface* intf = iport.sc_get_interface();
	if ((intf == NULL) || (iport.get_port_type()
			!= sca_port_base::SCA_SC_IN_PORT))
	{
		std::ostringstream str;
		str << "Port " << iport.get_port_number() << " in module: "
				<< iport.get_parent_module()->name() << std::endl
				<< "is no converter inport. Setting reactivity is only possible for "
				<< "SystemC to SystemC-AMS ports (e.g. sca_scsdf_in<>)."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	if (sync_domain == NULL)
		return ::sc_core::SC_ZERO_TIME;
	return sync_domain->register_activation_event(intf->default_event());
}

/////////////////////////////////////////////////////////////////

} //namespace sca_core
