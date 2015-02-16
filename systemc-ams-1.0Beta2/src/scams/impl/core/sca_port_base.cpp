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

 sca_port_base.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 15.05.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_port_base.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/impl/core/sca_port_base.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/synchronization/sca_sync_value_handle.h"
#include "scams/impl/core/sca_solver_base.h"

#include <sstream>

using namespace sc_core;
using namespace std;

namespace sca_core
{
namespace sca_implementation
{

sca_port_base::sca_port_base()
{
	port_elaborated = false;

	port_type = SCA_NOT_DEFINED_PORT;

	m_if = NULL;
	m_sca_if = NULL;

	port_if_id = 0;
	port_number = 0;

	//ignore hierarchical ports
	//will be already registered in base sc_module
	//ports outside modules should yield an error in the base classes
	parent_module = sca_get_curr_simcontext()->get_current_sca_module();
	if (parent_module)
		port_number = parent_module->register_port(this);
}

/////////////////////////////////////////

//destructor
sca_port_base::~sca_port_base()
{
}

/////////////////////////////////////////


sca_module* sca_port_base::get_parent_module() const
{
	return parent_module;
}

sca_interface* sca_port_base::create_synchronization_channel()
{
	return NULL;
}

/////////////////////////////////////////

const char* sca_port_base::sca_name()
{
	sc_object* obj = dynamic_cast<sc_object*> (this);
	if (obj == NULL)
		return "NO valid object";
	else
		return obj->name();
}

/////////////////////////////////////////

const char* sca_port_base::sca_basename()
{
	sc_object* obj = dynamic_cast<sc_object*> (this);
	if (obj == NULL)
		return "NO valid object";
	else
		return obj->basename();
}

/////////////////////////////////////////

void sca_port_base::end_of_port_elaboration()
{
}

void sca_port_base::register_sc_value_trace(const ::sc_core::sc_event& ev,
			sca_core::sca_implementation::sca_sync_value_handle_base& handle)
{
	parent_module->get_sync_domain()->register_sc_value_trace(ev,handle);
}


void sca_port_base::register_sca_schedule(::sc_core::sc_time next_time,
			sca_core::sca_implementation::sca_sync_value_handle_base& handle)
{
	parent_module->get_sync_domain()->register_sca_schedule(next_time,handle);
}


void sca_port_base::get_sc_value_on_time(::sc_core::sc_time time,
		                    sca_sync_value_handle_base& handle)
{
	parent_module->get_sync_domain()->get_sc_value_on_time(time,handle);
}



//writes sc value from handle at a given time
void sca_port_base::write_sc_value_on_time( ::sc_core::sc_time current_time,
		                       ::sc_core::sc_time next_time,
		                      sca_sync_value_handle_base& handle)
{
	parent_module->get_sync_domain()->write_sc_value(current_time,next_time,handle);
}


sc_core::sc_time sca_port_base::get_parent_module_time() const
{
	return get_parent_module()->sca_get_time();
}

void sca_port_base::elaborate_port()
{
	m_if = sc_get_interface();

	if (m_if == NULL)
	{
		ostringstream str;
		str << "Error module: " << parent_module->name() << " port: "
				<< port_number << " not bound" << endl;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	m_sca_if = dynamic_cast<sca_interface*> (m_if);
	/*
	 if(m_sca_if==NULL)
	 {
	 m_sca_if = create_synchronization_channel();
	 sca_prim_channel* m_sca_ch = dynamic_cast<sca_prim_channel*>(m_sca_if);
	 if(m_sca_ch==NULL)
	 {
	 ostringstream str;
	 str << "Embedded port synchronization interface has no ";
	 str << "sca-channel : " << parent_module->name();
	 str << " port: " << port_number << endl;
	 SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	 }
	 m_sca_ch->register_sc_interface(m_if);
	 }


	 if(m_sca_if==NULL)  //port to SystemC kernel
	 {
	 ostringstream str;
	 str << "sca_port #" << port_number << " in " <<  parent_module->name() << ": ";

	 if( (port_type == SCA_SC_INOUT_PORT) ||
	 (port_type == SCA_SC_OUT_PORT)   ||
	 (port_type == SCA_SC_IN_PORT)	)
	 {
	 str << "needs a synchronization interface." << endl;
	 }
	 else
	 {
	 str << "must be connected with sca_interface." << endl;
	 }
	 SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	 }
	 */
	if (port_type == SCA_NOT_DEFINED_PORT)
	{
		ostringstream str;
		str << "Cannot determine type of sca_port #" << port_number
				<< " in sca_module: " << parent_module->name()
				<< " --> port_type is not set." << endl;
		SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());
	}

	if (m_sca_if != NULL)
	{
		sca_prim_channel* m_sca_ch = dynamic_cast<sca_prim_channel*> (m_sca_if);

		if (m_sca_ch == NULL)
		{
			ostringstream str;
			str << "Interface bound to sca_port #" << port_number << " in "
					<< parent_module->name()
					<< ": is not implemented by a sca_channel." << endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		//  sets id for port identificattion
		set_if_id(m_sca_ch->get_number_of_connected_ports());
		m_sca_ch->register_port(this);
	}

	port_elaborated = true;

	end_of_port_elaboration();
}

///////////////////////////////////////////////////////////////////////////////
// implementation of traceable object interface

//virtual method may overloaded
void sca_port_base::set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.set_type_info<double>(); //default double
}


 bool sca_port_base::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
 {
	 set_type_info(data);

 //trace init is called after end_of_elaboration -> thus port binding has
 //been always finished
 if(m_sca_if==NULL)
 {
 m_sca_if      = dynamic_cast<sca_interface*>(sc_get_interface());
 }

 sca_prim_channel* m_sca_ch = dynamic_cast<sca_prim_channel*>(sca_get_interface());
 if(m_sca_ch==NULL) return false;
 else               return m_sca_ch->trace_init(data);
 }

 void sca_port_base::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
 {
 sca_prim_channel* m_sca_ch = dynamic_cast<sca_prim_channel*>(sca_get_interface());
 if(m_sca_ch==NULL) return;
 else               return m_sca_ch->trace(id,buffer);
 }


}//namespace sca_implementation
}//namespace sca_core
