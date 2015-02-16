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

  sca_tdf_trace_variable_base.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 12.12.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_trace_variable_base.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/predefined_moc/tdf/sca_tdf_trace_variable_base.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_tdf
{
namespace sca_implementation
{

sca_trace_variable_base::sca_trace_variable_base(const char* nm) :
	sc_core::sc_object(nm)
{
	rate = 1;
	timeoffset = sc_core::SC_ZERO_TIME;

	parent_module = dynamic_cast<sca_tdf::sca_module*>(
			sca_core::sca_implementation::sca_get_curr_simcontext()->get_current_sca_module());

	if (!parent_module)
	{
		std::ostringstream str;
		str << "A sca_tdf::sca_trace_variable"
				<< " can be used only in the context of a "
				<< "sca_tdf::sca_module (SCA_TDF_MODULE) for vatiable: "<< name()<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}
	initialized=false;
}

bool sca_trace_variable_base::initialize()
{
	if (parent_module->get_sync_domain()==NULL)
	{
		std::ostringstream str;
		str << "A sca_tdf::sca_trace_variable"
				<< " can be used only in the context of a "
				<< "sca_tdf::sca_module (SCA_TDF_MODULE) for vatiable: "<< name()<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		return false;
	}

	calls_per_period = parent_module->get_sync_domain()->get_calls_per_period();
	call_counter_ref = parent_module->get_sync_domain()->get_call_counter_ref();

	resize_buffer(rate*calls_per_period);
	write_flags.resize(rate*calls_per_period, false);

	initialized=true;

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool sca_trace_variable_base::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.type="-";
	data.unit="-";
	//trace will be activated after every complete cluster calculation
	//by the synchronization layer

	set_type_info(data);


	if(initialize())
	{
		parent_module->get_sync_domain()->add_cluster_trace(data);
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////

void sca_trace_variable_base::trace(long id, sca_util::sca_implementation::sca_trace_buffer& tr_buffer)
{
	sca_core::sca_time ctime, sample_period_loc;

	sample_period_loc = parent_module->get_sync_domain()->get_current_period();
	ctime=parent_module->get_sync_domain()->get_cluster_start_time() + timeoffset;

	for (int i=0; i<buffer_size; i++)
	{
		if (write_flags[i])
		{
			store_to_last(i);;
			write_flags[i]=false;
		}
		trace_last(id, ctime, tr_buffer);
		ctime+=sample_period_loc/rate;
	}

}

////////////////////////////////////////////////////////////////////////////


sca_trace_variable_base::~sca_trace_variable_base()
{
}


} //namespace sca_implementation
} //namespace sca_tdf
