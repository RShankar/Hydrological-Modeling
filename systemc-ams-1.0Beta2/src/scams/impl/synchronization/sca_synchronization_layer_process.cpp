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

 sca_synchronization_layer_process.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 26.08.2009

 SVN Version       :  $Revision: 1104 $
 SVN last checkin  :  $Date: 2011-01-09 11:45:25 +0100 (Sun, 09 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_synchronization_layer_process.cpp 1104 2011-01-09 10:45:25Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include"systemc-ams"

#include "scams/impl/synchronization/sca_synchronization_layer_process.h"
#include "scams/impl/util/tracing/sca_trace_file_base.h"
#include "scams/impl/core/sca_simcontext.h"
#include <algorithm>

namespace sca_core
{
namespace sca_implementation
{



sca_synchronization_layer_process::sca_synchronization_layer_process(
		sca_synchronization_alg::sca_cluster_objT* ccluster)
{
	cluster = ccluster;
	cluster_period = &(cluster->csync_data.cluster_period);
	cluster_resume_event = &(cluster->csync_data.cluster_resume_event);
	cluster_start_time = &(cluster->csync_data.cluster_start_time);
	next_cluster_start_time = &(cluster->csync_data.next_cluster_start_time);
	next_cluster_period = &(cluster->csync_data.next_cluster_period);
	cluster_traces = &(cluster->csync_data.traces);
	reactivity_events = &(cluster->csync_data.reactivity_events);

	request_new_period = &(cluster->csync_data.request_new_period);
	request_new_start_time = &(cluster->csync_data.request_new_start_time);

	sc_core::sc_spawn_options opt;
	opt.set_sensitivity(cluster_resume_event);
	opt.set_sensitivity(&time_out_event);

	sc_core::sc_spawn(
			sc_bind(&sca_synchronization_layer_process::cluster_process,this),
			sc_core::sc_gen_unique_name("cluster_process"),&opt);
}

void sca_synchronization_layer_process::cluster_process()
{
	std::vector<sca_synchronization_alg::schedule_element*>::iterator
			scheduling_begin, scheduling_end;

	scheduling_begin = cluster->scheduling_list.begin();
	scheduling_end = cluster->scheduling_list.end();

	bool
			& all_traces_initialized =
					sca_core::sca_implementation::sca_get_curr_simcontext()->all_traces_initialized();


	for_each(
			cluster->begin(),
			cluster->end(),
			std::mem_fun(&sca_synchronization_alg::sca_sync_objT::call_init_method)
	         );

	(*cluster_start_time) = (*next_cluster_start_time);

	//prevent delta cycle at time zero
	if((*cluster_start_time) - ::sc_core::sc_time_stamp() != sc_core::SC_ZERO_TIME)
	{
	   sc_core::wait((*cluster_start_time) - sc_core::sc_time_stamp());
	}

#ifdef SCA_IMPLEMENTATION_DEBUG
	std::cout << ::sc_core::sc_time_stamp() << "---------- TDF Cluster started ------------"
	<< " current period: " << (*cluster_period) << std::endl;
#endif

	cp_backup = (*cluster_period);

	while (true)
	{
		if (!all_traces_initialized)
		{
			all_traces_initialized = true;

			std::vector<sca_util::sca_implementation::sca_trace_file_base*>* traces;

			traces=sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();

			for_each(traces->begin(),traces->end(),
			              std::mem_fun(&sca_util::sca_implementation::sca_trace_file_base::initialize));
		}

		//after this time cluster calculation must be finished
		time_out_event.notify(*cluster_period);

		//cout << "Next time out event: " << sc_time_stamp()+(*cluster_period) << endl;

		for_each(scheduling_begin,scheduling_end,
		              std::mem_fun(&sca_synchronization_alg::schedule_element::run));


		for_each(cluster_traces->begin(),cluster_traces->end(),
		              std::mem_fun(&sca_util::sca_implementation::sca_trace_object_data::trace));

		//claculate next start time -> earliest time point wins

		sc_core::sc_time ctime = sc_core::sc_time_stamp();
		sc_core::sc_time next_start_time = last_start_time + cp_backup;

		if (*request_new_period)
		{
			cp_backup = (*next_cluster_period);
			next_start_time = last_start_time + cp_backup;
			(*request_new_period) = false;
		}

		if (*request_new_start_time)
		{
			if (next_start_time > (*next_cluster_start_time))
			{
				next_start_time = (*next_cluster_start_time);
			}
			(*request_new_start_time) = false;
		}

		time_out_event.cancel(); //cluster calculation ready

		if (reactivity_events->size() != 0)
		{
#ifdef SCA_IMPLEMENTATION_DEBUG
			std::cout << ::sc_core::sc_time_stamp() << " reactivity_events: "
			<< reactivity_events->size() << std::endl;
#endif

			//create event list - first event next_start_time event
			sc_core::sc_event time_out_ev;
			time_out_ev.notify(next_start_time - ctime);
			sc_core::sc_event_or_list* ev_list = &(time_out_ev
					| *(*reactivity_events)[0]);
			for (unsigned int i = 1; i < reactivity_events->size(); i++)
				ev_list = &((*ev_list) | *(*reactivity_events)[i]);
			sc_core::wait(*ev_list);

			reactivity_events->resize(0); //delete event list
		}
		else
		{
			sc_core::wait(next_start_time - ctime);
		}

		ctime = sc_core::sc_time_stamp();
		(*cluster_period) = ctime - (*cluster_start_time);
		(*cluster_start_time) = ctime;
		last_start_time = ctime;

#ifdef SCA_IMPLEMENTATION_DEBUG
		std::cout << "----- current start time: " << ctime << " cluster period: "
		<< (*cluster_period) << "  -------- " << std::endl;
#endif
	}
}


sca_synchronization_layer_process::~sca_synchronization_layer_process()
{
	for (sca_synchronization_alg::sca_cluster_objT::iterator it =
			cluster->begin(); it != cluster->end(); it++)
		(*it)->sync_if->terminate();
}

}
}

