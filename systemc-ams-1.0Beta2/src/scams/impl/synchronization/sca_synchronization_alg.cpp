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

 sca_synchronization_alg.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 26.08.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_synchronization_alg.cpp 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/impl/synchronization/sca_synchronization_alg.h"
#include "scams/impl/solver/linear/sca_linear_solver.h"
#include "scams/impl/synchronization/sca_synchronization_layer_process.h"
#include "scams/impl/util/tracing/sca_trace_file_base.h"
#include "scams/impl/core/sca_simcontext.h"

#include<algorithm>
#include<climits>

using namespace sca_util;
using namespace sca_util::sca_implementation;

namespace sca_core
{
namespace sca_implementation
{

sca_synchronization_alg::sca_sync_objT::sca_sync_objT()
{
	cluster_id = -1;
}

sca_synchronization_alg::sca_synchronization_alg()
{
	schedule_list_length = 0;
	scaled_time_lcm = 1;
	scheduling_list_warning_printed = false;

	sync_obj_mem = NULL;
}

sca_synchronization_alg::~sca_synchronization_alg()
{
	for (std::vector<sca_cluster_objT*>::iterator cit = clusters.begin(); cit
			!= clusters.end(); ++cit)
	{
		delete (*cit)->csync_mod;
		(*cit)->csync_mod=NULL;
		delete *cit;
		(*cit)=NULL;
	}

	delete[] sync_obj_mem;
	sync_obj_mem=NULL;
}

/** Initializes datastructures and calculates the scheduling list */
void sca_synchronization_alg::initialize(std::vector<
		sca_synchronization_obj_if*>& solvers)
{
	sync_obj_mem = new sca_sync_objT[solvers.size()];

	int id_cnt = 0;
	for (std::vector<sca_synchronization_obj_if*>::iterator sit =
			solvers.begin(); sit != solvers.end(); ++sit, ++id_cnt)
	{
		sync_objs.push_back(*sit);
		(*sit) -> id = id_cnt;
		(*sit) -> dead_cluster = false;

		//initialize objects for analyzer to speed up (re-) analyzing process
		int nin = (*sit)-> get_number_of_sync_ports(
				sca_synchronization_obj_if::FROM_ANALOG);
		int nout = (*sit)-> get_number_of_sync_ports(
				sca_synchronization_obj_if::TO_ANALOG);
		sync_obj_mem[id_cnt].init(nin, nout);

	}

	check_closed_graph();
	cluster();
#ifdef SCA_IMPLEMENTATION_DEBUG

	std::cout << "\t\tClustering finished: " << clusters.size()
	<< " cluster found" << std::endl;
#endif

	analyse_sample_rates();

#ifdef SCA_IMPLEMENTATION_DEBUG

	for(unsigned long i=0;i<clusters.size();i++)
	{
		if(clusters[i]->dead_cluster)
		std::cout << "!!!!!!!!!!! Cluster dead !!!!!!!!!!!!!!!" << std::endl;
		clusters[i]->print();
		std::cout << std::endl
		<< "------------------------------------------------------" << endl;
	}
#endif

	generate_scheduling_list();

#ifdef SCA_IMPLEMENTATION_DEBUG

	for(unsigned long i=0;i<clusters.size();i++)
	{
		clusters[i]->print_schedule_list();
		std::cout << std::endl
		<< "------------------------------------------------------" << std::endl;
	}
#endif

	std::ostringstream sta_str;
	sta_str << std::endl;
	sta_str << "\t" << clusters.size() << " dataflow clusters instantiated"
			<< std::endl;

	unsigned long& info_mask(
	    sca_core::sca_implementation::sca_get_curr_simcontext()->
	    get_information_mask());


	for (unsigned long i = 0; i < clusters.size(); i++)
	{

		if (!clusters[i]->dead_cluster)
		{
		  if(info_mask & sca_util::sca_info::sca_tdf_solver.mask)
		  {
			 unsigned long to_systemc=0;
			 unsigned long from_systemc=0;

			 for(sca_cluster_objT::iterator it=  clusters[i]->begin();
			 it!= clusters[i]->end();   it++)
			 {
			 to_systemc+=(*it)->sync_if->get_number_of_sync_ports(sca_synchronization_obj_if::TO_SYSTEMC);
			 from_systemc+=(*it)->sync_if->get_number_of_sync_ports(sca_synchronization_obj_if::FROM_SYSTEMC);
			 }

			 sta_str << "\t  cluster " << i << ":" << std::endl;
			 sta_str << "\t\t" << clusters[i]->size() << " dataflow modules/solver, ";
			 sta_str << "contains e.g. module: " << (*(clusters[i]->begin()))->sync_if->get_name_associated_names(5) << std::endl;
			 sta_str << "\t\t" << clusters[i]->schedule_list_length << " elements in schedule list,  " << std::endl;
			 sta_str << "\t\t" << clusters[i]->T_cluster<< " cluster period, " << std::endl;
			 sta_str << "\t\t"<< "ratio to lowest:  "  << clusters[i]->calls_per_period_max;
			 sta_str <<"              e.g. module: "<< clusters[i]->mod_name_period_max << std::endl;
			 sta_str << "\t\tratio to highest: "<< clusters[i]->calls_per_period_min << " sample time ";
			 sta_str <<" e.g. module: " << clusters[i]->mod_name_period_min << std::endl;
			 sta_str << "\t\t" << to_systemc << " connections to SystemC de, ";
			 sta_str << from_systemc << " connections from SystemC de";
			 sta_str << std::endl;
	           }

			clusters[i]->csync_mod = new sca_synchronization_layer_process(clusters[i]);
		}
	}


	if(info_mask & sca_util::sca_info::sca_tdf_solver.mask)
	{
	  SC_REPORT_INFO("SystemC-AMS",sta_str.str().c_str());
	}

}

///////////////////////////////////////////////////////////////////////////////

//Paranoid check which avoids some checking in the
//following steps
void sca_synchronization_alg::check_closed_graph()
{
	sca_synchronization_obj_if* sync_obj;
	sca_prim_channel* scaif;
	unsigned long nch;

	//for all sync-objects
	for (std::vector<sca_synchronization_obj_if*>::iterator sit =
			sync_objs.begin(); sit != sync_objs.end(); sit++)
	{

		sca_synchronization_obj_if::directionT dir =
				sca_synchronization_obj_if::FROM_ANALOG;
		do //for both directions
		{
			//check whether input - channels have one driver only
			nch = (*sit)->get_number_of_sync_ports(dir);

			for (unsigned long i = 0; i < nch; i++) //for all channels of one direction
			{
				//get channel
				scaif = dynamic_cast<sca_prim_channel*> ((*sit)->get_sync_port(
						dir, i)->channel);

				////// check that the connected channel has one driver only //////
				unsigned long ndriver =
						scaif->get_connected_solvers(true).size();
				if ((ndriver > 1) | (ndriver < 1))
				{
					std::ostringstream str;
					if (ndriver > 1)
						str
								<< "sca_tdf::sca_signal has more than one driver "
								<< "the following modules are connected to the signal: "
								<< std::endl;
					else
						str << "sca_tdf::sca_signal has no driver "
								<< "the following modules are connected to the channel: "
								<< std::endl;

					unsigned long nmod =
							scaif->get_number_of_connected_modules();
					for (unsigned long j = 0; j < nmod; j++)
					{
						str << "\t"
								<< scaif->get_connected_module_list()[j]->name()
								<< std::endl;
					}
					str << std::endl;
					SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
				}

				////// check that the connected channel has at least one sink //////
				unsigned long nsinks =
						scaif->get_connected_solvers(false).size();
				//        if(nsinks<1)
				//        {
				//          std::ostringstream str;
				//          str << "sca_synchronization channel has no sink "
				//              << "the following modules are connected to the channel: " << std::endl;
				//
				//          unsigned long nmod=scaif->get_number_of_connected_modules();
				//          for(unsigned long j=0;j<nmod;j++)
				//          {
				//            str << "\t"<< scaif->get_connected_module_list()[j]->name()<< std::endl;
				//          }
				//          str << std::endl;
				//          SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
				//        }

				//check driver solver first - only one exists
				sync_obj = scaif->get_connected_solvers(true)[0];
				sca_sync_obj_listT::iterator it;
				long k = -1;
				while (k < (long) nsinks)
				{
					//then check all sink solver
					if (k >= 0)
						sync_obj = scaif->get_connected_solvers(false)[k];
					k++;

					// check that the solver connected via the channel
					//  is also a member of the list
					it = find(sync_objs.begin(), sync_objs.end(), sync_obj);
					if (it == sync_objs.end())
					{
						std::ostringstream str;
						str << "Connected sca_synchronization object is not "
								<< "in the synchronization object list: "
								<< std::endl;
						SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
					}

					// check that the connected solver is only included one time
					it = find(++it, sync_objs.end(), sync_obj);
					if (it != sync_objs.end())
					{
						std::ostringstream str;
						str
								<< "Error: connected sca_synchronization object is more than "
								<< "one time in the synchronization object list: "
								<< std::endl;
						SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
					}
				}
			} //end for all channels


			if (dir == sca_synchronization_obj_if::TO_ANALOG)
				dir = sca_synchronization_obj_if::FROM_ANALOG;
			else
				dir = sca_synchronization_obj_if::TO_ANALOG;

		} while (dir != sca_synchronization_obj_if::FROM_ANALOG);
	} //next synchronization object
}

///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::cluster()
{
	//reset all variables to allow re-clustering

	// if compiled with g++ 3.x you can use this instead the for loop
	//for_each(sync_objs.begin(),sync_objs.end(),
	//              mem_fun(&sca_synchronization_obj_if::reset));

	for (sca_sync_obj_listT::iterator it = sync_objs.begin(); it
			!= sync_objs.end(); it++)
		(*it)->reset();

	sca_sync_obj_listT::iterator sync_obj;

	//if you use g++ 3.x you can enable this lines instead of the following
	//for -loop
	//sync_obj = find_if(sync_objs.begin(),sync_objs.end(),
	//                 mem_fun(&sca_synchronization_obj_if::not_clustered));

	for (sync_obj = sync_objs.begin(); sync_obj != sync_objs.end(); sync_obj++)
		if ((*sync_obj)->not_clustered())
			break;

    int n = 0;

	while (sync_obj != sync_objs.end())
	{
		//create new cluster
		long cluster_id;
		sca_cluster_objT* cluster_tmp = new sca_cluster_objT;
		cluster_id = clusters.size();
       
        //std::cout<<n<<"]cluster_id ="<<cluster_id <<"\n";
        //n++;

		clusters.push_back(cluster_tmp);

		cluster_tmp->dead_cluster = false;

		sca_sync_objT* current_obj = &(sync_obj_mem[(*sync_obj)->id]);
		current_obj->sync_if = (*sync_obj);

		//store cluster id and add obj to cluster
		current_obj->cluster_id = cluster_id;
		(*sync_obj)->cluster_id = cluster_id;
		(*sync_obj)->sync_id = 0;

		cluster_tmp->push_back(current_obj);
		if ((*sync_obj)->dead_cluster)
		{
			cluster_tmp->dead_cluster = true;
#ifdef SCA_IMPLEMENTATION_DEBUG

			std::cout << "----- dead module : " << (*sync_obj)->get_name_associated_names(5) << std::endl;
#endif

		}

		//rekursive method which moves all connected objs
		//to the cluster
        
        //std::cout<<"C ="<<cluster_tmp<<" "<<cluster_id<<""<<current_obj<<std::endl;

        move_connected_objs(cluster_tmp, cluster_id, current_obj);

		//if you use g++ 3.x you can enable the following lines instead of the
		//for - loop
		sync_obj = find_if(sync_objs.begin(), sync_objs.end(), std::mem_fun(
				&sca_synchronization_obj_if::not_clustered));
		//    for( sync_obj =  sync_objs.begin();
		//         sync_obj != sync_objs.end();
		//         sync_obj++ ) if( (*sync_obj)->not_clustered() ) break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// if not yet done the sync_obj will be moved from the global to the cluster
// list -> than recursive call for the moved object
sca_synchronization_alg::sca_sync_objT*
sca_synchronization_alg::move_obj_if_not_done(
		sca_synchronization_obj_if* sync_obj, sca_cluster_objT* cluster_,
		long cluster_id)
{
    
    //std::cout<<"\tmove_obj_if_not_done "<<sync_obj<<" "<<cluster_<<cluster_id<<std::endl;

    if (sync_obj->cluster_id < 0) //not yet founded
	{
		sca_sync_objT* next_obj = &(sync_obj_mem[sync_obj->id]);
		next_obj->sync_if = sync_obj;

		//store cluster id and add obj to cluster

		next_obj->cluster_id = cluster_id;
		sync_obj->cluster_id = cluster_id;
		sync_obj->sync_id = clusters[cluster_id]->size();

		cluster_->push_back(next_obj);
		if (sync_obj->dead_cluster)
			cluster_->dead_cluster = true;
        
        //std::cout<<"cluster_id ="<<cluster_id <<"\n";
		//rekursive method which moves all connected objs
		//to the cluster
        
        //std::cout<<"\tnext_obj = "<<next_obj<<std::endl;

        //std::cout<<"\tcalling move_connected_objs"<<next_obj<<std::endl;

		move_connected_objs(cluster_, cluster_id, next_obj);
       // std::cout<<"next"<<"\n";
		return next_obj;
	}
	else if (sync_obj->dead_cluster)
	{
#ifdef SCA_IMPLEMENTATION_DEBUG
		std::cout << "----- dead module : " << sync_obj->get_name_associated_names(5) << std::endl;
#endif

		clusters[sync_obj->cluster_id]->dead_cluster = true;
	}

	return (*cluster_)[sync_obj->sync_id];
}

///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::move_connected_objs(sca_cluster_objT* cluster_,
		long cluster_id, sca_sync_objT* current_obj)
{
    //std::cout<<cntP<<")move_connected_objs "<<cluster_<<" "<< cluster_id<<" "<<current_obj<<std::endl;

    //std::cout<<"move_connected_objs::cluster_id="<<cluster_id<<std::endl;

	//go first in the dirction of all outports
	unsigned long nop;
	nop = current_obj->sync_if->get_number_of_sync_ports(
			sca_synchronization_obj_if::TO_ANALOG);

	for (unsigned long opnr = 0; opnr < nop; opnr++)
	{
		//get current outport
		sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port
				*cport;
		cport = current_obj->sync_if->get_sync_port(
				sca_synchronization_obj_if::TO_ANALOG, opnr);

		//get sampling rate of current port
		long port_rate = *(cport -> rate);

        //std::cout<<"port_rate= "<<port_rate<<std::endl;

		//initialize for scheduling analysis
		current_obj->out_rates[opnr] = port_rate;

		//get channel connected to current port
		//dynamic cast checked during add_port
		sca_prim_channel* scaif =
				dynamic_cast<sca_prim_channel*> (cport -> channel);

		//an outport drives one or more inports but no other outport
		//get number of connected inports
		unsigned long ncip = scaif -> get_connected_sync_ports(false).size();
		current_obj->next_inports[opnr].resize(ncip);
		for (unsigned long cipnr = 0; cipnr < ncip; cipnr++)
		{
			//get next connected inport
			sca_sync_port *ciport =
					scaif->get_connected_sync_ports(false)[cipnr];

			//get solver which is of the sync_port
			sca_synchronization_obj_if* sync_obj = ciport -> parent_obj;

			if (port_rate != 0)
			{
				sca_sync_objT* con_obj;
				//std::cout<<"->move_obj_if_not_done("<<sync_obj<<" "<<cluster_<<" "<<cluster_id<<")"<<std::endl;
                con_obj = move_obj_if_not_done(sync_obj, cluster_, cluster_id);

				//initialize datastructure for scheduling analysis
				//connect references to connected inports
				current_obj->next_inports[opnr][cipnr]
						= &(con_obj->sample_inports[ciport->sync_port_number]);
				//initialize sample counter with number of delays
				*(current_obj->next_inports[opnr][cipnr]) += *(cport -> delay);
			}
			else
			{
#ifdef SCA_IMPLEMENTATION_DEBUG
				std::cout << "----- dead module : " << sync_obj->get_name_associated_names(5) << std::endl;
#endif

				//dead clusters not yet supported
				std::ostringstream str;
				str << std::endl;
				str << "Rate=0 not allowed for port: "
						<< cport->port->sca_name() << std::endl << std::endl;
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());

				sync_obj->dead_cluster = true;
				if (sync_obj->cluster_id >= 0)
					clusters[sync_obj->cluster_id]->dead_cluster = true;
			}
		} //for(cipnr=0;cipnr<ncip;cipnr++) -- for all connected inports
	} //for(unsigned long opnr=0;opnr<nop;opnr++) -- for all outports


	//now do the same for all inports
	unsigned long nip;
	nip = current_obj->sync_if->get_number_of_sync_ports(
			sca_synchronization_obj_if::FROM_ANALOG);

	for (unsigned long ipnr = 0; ipnr < nip; ipnr++)
	{
		//get driving outport
		sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port
				*cport;
		cport = current_obj->sync_if->get_sync_port(
				sca_synchronization_obj_if::FROM_ANALOG, ipnr);

		//get sampling rate of current port
		long port_rate = *(cport -> rate);

		current_obj->in_rates[ipnr] = port_rate;
		current_obj->sample_inports[ipnr] += *(cport -> delay);

		//get channel connected to current port
		sca_prim_channel* scaif =
				dynamic_cast<sca_prim_channel*> (cport -> channel);

		//an inport is driven by one outport only

		//get connected outport
		sca_sync_port *coport = scaif->get_connected_sync_ports(true)[0];

		//get solver of the sync outport
		sca_synchronization_obj_if* sync_obj = coport -> parent_obj;

		if (port_rate != 0  )
		{
            if( sync_obj->cluster_id < 0 || sync_obj->dead_cluster)
            {
                //std::cout<<"sync_obj->cluster_id="<<sync_obj->cluster_id<<std::endl;
			    move_obj_if_not_done(sync_obj, cluster_, cluster_id);
            }
		}
		else
		{
			sync_obj->dead_cluster = true;
			if (sync_obj->cluster_id >= 0)
				clusters[sync_obj->cluster_id]->dead_cluster = true;

			std::ostringstream str;
			str << std::endl;
			str << "rate=0 not allowed for port: " << cport->port->sca_name()
					<< std::endl << std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

	} //for(unsigned long ipnr=0;ipnr<nip;ipnr++) -- for all inports
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline sc_dt::uint64 sca_synchronization_alg::analyse_sample_rates_gcd(
		sc_dt::uint64 x, sc_dt::uint64 y)
{
	if (y == 0)
		return x;
	else
		return analyse_sample_rates_gcd(y, x % y);
}

// lowest common multiple
inline unsigned long sca_synchronization_alg::analyse_sample_rates_lcm(
		unsigned long n1, unsigned long n2)
{
	if ((n1 == 0) || (n2 == 0))
		return 0;
	return ((sc_dt::uint64) n1 * (sc_dt::uint64) n2)
			/ analyse_sample_rates_gcd(n1, n2);
}

///////////////////////////////////////////////////////////////////////////////

inline bool sca_synchronization_alg::analyse_sample_rates_first_obj(
		unsigned long& nin, unsigned long& nout, long& multiplier,
		sca_synchronization_alg::sca_sync_objT*& obj,
		sca_cluster_objT::iterator& sit,
		std::vector<sca_cluster_objT*>::iterator& cit)
{
	sca_prim_channel* ch;
	sca_synchronization_obj_if::sca_sync_port* sport;

	long port_rate;
	bool p_found = false;

	//start with multiplier 1
	multiplier = 1;

	//assign object calls -> try 1 for the first object
	obj->osync_data.calls_per_period = 1;
	obj->multiple = multiplier;

	check_sample_time_consistency(obj, NULL, multiplier);

	//start on first inport
	for (unsigned long i = 0; i < nin; i++)
	{
		sport = obj->sync_if-> get_sync_port(
				sca_synchronization_obj_if::FROM_ANALOG, i);
		port_rate = *(sport->rate);

		if (port_rate > 0)
		{
			p_found = true;
			ch = sport->channel;

			ch->max_samples = *(sport->rate);
			ch->multiple = multiplier;
			break;
		}
	}

	for (unsigned long i = 0; (i < nout) & !p_found; i++)
	{
		sport = obj->sync_if-> get_sync_port(
				sca_synchronization_obj_if::TO_ANALOG, i);
		port_rate = *(sport->rate);

		if (port_rate > 0)
		{
			p_found = true;
			ch = sport->channel;

			ch->max_samples = *(sport->rate);
			ch->multiple = multiplier;
			break;
		}
	}

	if (!p_found)
	{
		//cluster consists of one object only -> no channel to another object
		sit++;
		if (sit != (*cit)->end())
		{
			// Error should not be possible -> Error in cluster() ??
			std::ostringstream str;
			str << "Internal Error in: "
					<< "sca_synchronization_alg::analyse_sample_rates() "
					<< __FILE__ << " line: " << __LINE__ << std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
		sit--;

		//such an solver will be called once per cluster period
		obj->osync_data.calls_per_period = 1;
		//obj->sync_if->get_calls_per_period() = 1;

		//in this case we have may be synchronization ports with a time assigned

		//check timing of synchronization ports
		unsigned long nin_sc = obj->sync_if-> get_number_of_sync_ports(
				sca_synchronization_obj_if::FROM_SYSTEMC);
		unsigned long nout_sc = obj->sync_if-> get_number_of_sync_ports(
				sca_synchronization_obj_if::TO_SYSTEMC);

		for (unsigned long i = 0; i < (nin_sc + nout_sc); i++)
		{
			//first for all inports
			if (i < nin_sc)
				sport = obj->sync_if->get_sync_port(
						sca_synchronization_obj_if::FROM_SYSTEMC, i);
			else
				sport = obj->sync_if->get_sync_port(
						sca_synchronization_obj_if::TO_SYSTEMC, i - nin_sc);

			check_sample_time_consistency(obj, sport, 1);
		}
		return true; //cluster ready - no more objects
	}

	return false; //there must be more objects
}

///////////////////////////////////////////////////////////////////////////////

inline void sca_synchronization_alg::analyse_sample_rates_calc_obj_calls(
		unsigned long& nin, unsigned long& nout, long& multiplier,
		sca_synchronization_alg::sca_sync_objT*& obj)
{
	sca_prim_channel* ch;
	sca_synchronization_obj_if::sca_sync_port* sport;

	//search for sync_port channel which is yet calculated
	bool ch_found = false;

	sc_dt::int64 ch_max_sample;
	long port_rate;
	sc_dt::int64 n_sample;

	obj->osync_data.calls_per_period = 0;

	//search on all inports and outports
	for (unsigned long i = 0; i < (nin + nout); i++)
	{
		//search first on all inports
		if (i < nin)
			sport = obj->sync_if-> get_sync_port(
					sca_synchronization_obj_if::FROM_ANALOG, i);
		else
			sport = obj->sync_if-> get_sync_port(
					sca_synchronization_obj_if::TO_ANALOG, i - nin);

		port_rate = *(sport->rate);

		// ignore ports with rate zero
		if (port_rate == 0)
			continue;

		ch = sport->channel;
		//ch->multiple > 0; multiplier > 0

		//we must be careful to prevent integer oferflows
		//the current multiplier is >= the ch->multiplier and is always
		//a multiple of the ch->multiplier -> thus we can divide first
		//and than multiply -> we get no temporarly oferflow
		ch_max_sample = (sc_dt::int64) (ch->max_samples) * (sc_dt::int64) (multiplier
				/ ch->multiple);

		//if we have nevertheless an overflow we cannot handle this
		//we will need at least one buffer of several Giga - this should
		//exceed usually the memory limits
		if (ch_max_sample >= LONG_MAX)
		{
			std::ostringstream str;
			str << " Bad conditioned synchronization cluster integer "
					<< "limit reached for buffer size of: " << ch->name()
					<< std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		if (ch_max_sample > 0)
		{
			ch_found = true;

			//lowest common multiple
			n_sample = analyse_sample_rates_lcm(ch_max_sample, port_rate);
			if (n_sample < 0)
			{
				std::ostringstream str;
				str
						<< " Error bad conditioned synchronization cluster integer "
						<< "limit reached for buffer size of: " << ch->name()
						<< std::endl;
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}

			if (n_sample >= LONG_MAX)
			{
				std::ostringstream str;
				str << " Bad conditioned synchronization cluster integer "
						<< "limit reached for buffer size of: " << ch->name()
						<< std::endl;
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}

			multiplier = (n_sample / ch_max_sample) * multiplier;

			ch -> max_samples = n_sample;
			ch -> multiple = multiplier;

			obj->osync_data.calls_per_period = n_sample / port_rate;
			obj->multiple = multiplier;

			break; //obj-call defined can stop
		}
	} //end for all ports

	if (!ch_found)
	{
		// Error should not be possible
		std::ostringstream str;
		str << "Internal Error in: " << __FILE__ << " line: " << __LINE__
				<< " assumption for sync-algorithm seems not to be fullfilled"
				<< " in cluster: " << obj->cluster_id << std::endl;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////

inline void sca_synchronization_alg::check_sample_time_consistency(
		sca_synchronization_alg::sca_sync_objT*& obj,
		sca_synchronization_obj_if::sca_sync_port* sport, long multiplier)
{
	long n_sample;
	sca_core::sca_time c_timestep;


	if(sport!=NULL)
	{
		n_sample=(*(sport->rate)) * obj->osync_data.calls_per_period;
		//search for assigned T (sample time)
		c_timestep = *(sport->T_set);
	}
	else
	{
		if(obj->sync_if->timestep_is_set)
		{
			c_timestep=obj->sync_if->timestep_set;
			n_sample=obj->osync_data.calls_per_period;
		}
		else
		{
			return;
		}
	}

	if (c_timestep != NOT_VALID_SCA_TIME())
	{

		if (current_cluster->T_last_obj == NULL)
		{
			current_cluster->T_last_n_sample = n_sample;
			current_cluster->T_cluster = c_timestep * n_sample;
			current_cluster->T_multiplier = multiplier;
			current_cluster->T_last_obj = obj;
			current_cluster->T_last_port = sport;
			current_cluster->T_last=c_timestep;

			return;
		}


		//check consistency


			sca_core::sca_time T_expect = c_timestep;
			sca_core::sca_time T_ref = current_cluster->T_last;

			sc_dt::uint64 ti_exp = T_expect.value();
			sc_dt::uint64 ti_set = current_cluster->T_last.value();

			//scaling last n_sample value to current multiplier
			long last_n_sample = current_cluster->T_last_n_sample * multiplier
					/ current_cluster->T_multiplier;

			sc_dt::int64 ti_dt;

			//we use the largest assigned sample rate
			//for error calculation we scale always to the smaller T
			//thus the maximum error is +/- 1
			//( ti_exp > ti_set ) -> larger buffer, smaller T
			if (last_n_sample > n_sample)
			{
				//scaling to smaller old T
				ti_dt = ti_set - ti_exp / (last_n_sample / n_sample);

				//we store larger T fur cluster
				current_cluster->T_last_n_sample = n_sample;
				current_cluster->T_cluster = T_expect * n_sample;
				current_cluster->T_multiplier = multiplier;
				current_cluster->T_last_obj = obj;
				current_cluster->T_last_port = sport;
				current_cluster->T_last=c_timestep;
			}
			else
			{
				//scaling to smaller current T
				ti_dt = ti_set / (n_sample / last_n_sample) - ti_exp;
			}

			//we allow 2 resolution time units errors
			//(one for double conversion; second for divider)
			if ((ti_dt > 2) || (ti_dt < -2)) //llabs / abs is platform dependent
			{
				sca_core::sca_time T_cur;
				T_cur = current_cluster->T_cluster * (multiplier
						/ current_cluster->T_multiplier);

				long nk = (last_n_sample > n_sample) ? last_n_sample / n_sample
						: 1;
				sca_core::sca_time T_expect;
				if (ti_dt < 0)
					T_expect = c_timestep - sc_core::sc_get_time_resolution()
							* double(-ti_dt * nk);
				else
					T_expect = c_timestep + sc_core::sc_get_time_resolution()
							* double(ti_dt * nk);

				long nkr = (last_n_sample < n_sample) ? n_sample
						/ last_n_sample : 1;
				sca_core::sca_time T_ref_expect;
				if (ti_dt < 0)
					T_ref_expect = T_ref
							+ sc_core::sc_get_time_resolution()
									* double(-ti_dt * nkr);
				else
					T_ref_expect = T_ref
							- sc_core::sc_get_time_resolution()
									* double(ti_dt * nkr);

				std::ostringstream str;
				str << std::endl << std::endl;
				str << "Inconsistency in timestep assignment between" << std::endl;
				if(sport==NULL)
				{
					str << " module: " << obj->sync_if->module_timestep_set->name();
				}
				else
				{
				    str << "\t port: " << sport->port->sca_name();
				}
				str << "\t timestep: " << c_timestep << " (" << 1.0 / c_timestep.to_seconds()
				    << " Hz)" << "\t\t   expect: " << T_expect
				    << " (" << 1.0 / (T_expect.to_seconds()) << " Hz)" << std::endl
					<< "\t and ";
				if(current_cluster->T_last_port==NULL)
				{
					str << " module: "
					    << current_cluster->T_last_obj->sync_if->module_timestep_set->name();
				}
				else
				{
				  str << current_cluster->T_last_port->port->sca_name();
				}
				str << "\t T: " << T_ref << " (" << 1.0/ T_ref.to_seconds() << " Hz)"
				    << "\t\tor expect: " << T_ref_expect << " ("
				    << 1.0/ T_ref_expect.to_seconds() << " Hz)" << std::endl
					<< std::endl << "\t rate1/rate2= ";
				if (n_sample > last_n_sample)
				{
					str << n_sample / last_n_sample << "/1";
				}
				else
				{
					str << "1/" << last_n_sample / n_sample;
				}
				str << "\t dT: ";
				if (ti_dt < 0)
					str << "-" << sc_core::sc_get_time_resolution()
							* double(-ti_dt) << std::endl;
				else
					str << sc_core::sc_get_time_resolution() * double(ti_dt)
							<< std::endl;
				str << std::endl;

				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}
	}
}

inline void sca_synchronization_alg::analyse_sample_rates_assign_max_samples(
		unsigned long& nin, unsigned long& nout, long& multiplier,
		sca_synchronization_alg::sca_sync_objT*& obj)
{
	sca_prim_channel* ch;
	sca_synchronization_obj_if::sca_sync_port* sport;

	sc_dt::int64 ch_max_sample;
	long port_rate;
	sc_dt::int64 n_sample;

	check_sample_time_consistency(obj, NULL, multiplier);

	// assign max_samples to all connected channels
	// if yet assigned check consistency
	for (unsigned long i = 0; i < (nin + nout); i++)
	{
		//search first on all inports
		if (i < nin)
			sport = obj-> sync_if->get_sync_port(
					sca_synchronization_obj_if::FROM_ANALOG, i);
		else
			sport = obj-> sync_if->get_sync_port(
					sca_synchronization_obj_if::TO_ANALOG, i - nin);

		port_rate = *(sport->rate);

		ch = sport->channel;
		//multiplier > 0 ; ch->multiple > 0
		ch_max_sample = (sc_dt::int64) ch->max_samples * (sc_dt::int64) (multiplier
				/ ch->multiple);
		n_sample = port_rate * (sc_dt::int64) (obj->osync_data.calls_per_period);

		if ((ch_max_sample >= LONG_MAX) || (n_sample >= LONG_MAX))
		{
			std::ostringstream str;
			str << " Bad conditioned synchronization cluster integer "
					<< "limit reached for buffer size of: " << ch->name()
					<< std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		check_sample_time_consistency(obj, sport, multiplier);

		if (ch_max_sample > 0) //check consistency
		{
			//Due the assumption for the algorithm this condition must be
			//fullfilled, because all assigned
			//max_samples resulting from previously analyzed
			//modules/ports (in other words a change of those max_sample results
			//in a change of all yet assigned max_samples/obj_calls -> due it is
			//at least the second (the first was assigned above) analyzed
			//port/channel of the object consistency can't be achieved)
			if (ch_max_sample != n_sample)
			{
				std::ostringstream str;
				str << "Sample rates are inconsistent in : "
						<< obj -> sync_if->get_name_associated_names(5) << " at ";
				if (i < nin)
					str << "inport " << i << std::endl;
				else
					str << "outport " << i - nin << std::endl;

				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}
		}
		else //assign max_sample
		{
			ch -> max_samples = n_sample;
			ch -> multiple = multiplier;
		}
	} //for all ports

	//check timing of synchronization ports
	unsigned long nin_sc = obj->sync_if-> get_number_of_sync_ports(
			sca_synchronization_obj_if::FROM_SYSTEMC);
	unsigned long nout_sc = obj->sync_if-> get_number_of_sync_ports(
			sca_synchronization_obj_if::TO_SYSTEMC);

	for (unsigned long i = 0; i < (nin_sc + nout_sc); i++)
	{
		//first for all inports
		if (i < nin_sc)
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::FROM_SYSTEMC, i);
		else
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::TO_SYSTEMC, i - nin_sc);

		check_sample_time_consistency(obj, sport, multiplier);
	}
}

///////////////////////////////////////////////////////////////////////////////

inline void sca_synchronization_alg::analyse_sample_rates_calc_def_rates(
		long& multiplier, sca_synchronization_alg::sca_sync_objT *& obj)
{
	//obj->multiple can't be zero
	sc_dt::int64 calls_per_period;
	calls_per_period = (sc_dt::int64) (obj->osync_data.calls_per_period)
			* (sc_dt::int64) (multiplier / obj->multiple);

	obj->osync_data.calls_per_period = calls_per_period;
	//obj->sync_if->get_calls_per_period() = calls_per_period;

	//assign calculated module timestep
	obj->sync_if->timestep_calculated=current_cluster->csync_data.cluster_period/calls_per_period;

	//collect some infos which will speed-up the schedule-list build
	schedule_list_length += calls_per_period;
	scaled_time_lcm = analyse_sample_rates_lcm(scaled_time_lcm,
			calls_per_period);

	//this is heuristic - we warn due it can took very long for such lists
	//and may it comes anywhere an bad_alloc exception
	if (schedule_list_length > 10000000)
	{
		if (!scheduling_list_warning_printed)
		{
			std::ostringstream str;
			str << "The cluster containing module: "
					<< obj -> sync_if->get_name_associated_names(5)
					<< " results in an extremely long scheduling list - thus the memory"
					<< " space requirement will be very high - may the cluster is bad conditioned"
					<< " or it has very large sample rate ratios";
			SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());
			scheduling_list_warning_printed = true;
		}

		if (schedule_list_length >= LONG_MAX)
		{
			SC_REPORT_ERROR("SystemC-AMS","Maximum scheduling list length exceeded");
		}
	}

	//number of channels to the object (inports)
	unsigned long nin = obj->nin;
	//number of channels from the object (outports)
	unsigned long nout = obj->nout;

	sca_synchronization_obj_if::sca_sync_port* sport;

	//for all inports and outports
	for (unsigned long i = 0; i < (nin + nout); i++)
	{
		//first for all inports
		if (i < nin)
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::FROM_ANALOG, i);
		else
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::TO_ANALOG, i - nin);

		//delays not considered -> considered in concrete buffer implementation
		sc_dt::int64 max_sample = (sc_dt::int64) (sport->channel->max_samples)
				* (multiplier / sport->channel->multiple);

		if (max_sample >= LONG_MAX)
		{
			std::ostringstream str;
			str << " Bad conditioned synchronization cluster integer "
					<< "limit reached for buffer size of: "
					<< sport->channel->name() << std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		//set calculated T of port
		sport->T_calculated = current_cluster->csync_data.cluster_period
				/ max_sample;

		*(sport->T_set) = sport->T_calculated;

		if(sport->T_calculated==sc_core::SC_ZERO_TIME)
		{
			std::ostringstream str;
			str << "Timestep of port: " << sport->port->sca_name()
			    << " becomes zero (may resolution time is to high ) "
				<< " current cluster period: " << current_cluster->csync_data.cluster_period
				<< " divider: " << max_sample;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			return;
		}

		if (*(sport->t0_set) >= sport->T_calculated)
		{
			std::ostringstream str;
			str << "timeoffset: " << *(sport->t0_set)
					<< " is greater than timestep: " << sport->T_calculated
					<< " at port: " << sport->port->sca_name();
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			return;
		}
		else
		{
			sport->t0_calculated = *(sport->t0_set);
		}

		//ch->multiple should not be 0
		sport->channel->max_samples = max_sample;
		sport->channel->multiple = multiplier;

	}

	//asign timing to synchronization ports
	unsigned long nin_sc = obj->sync_if-> get_number_of_sync_ports(
			sca_synchronization_obj_if::FROM_SYSTEMC);
	unsigned long nout_sc = obj->sync_if-> get_number_of_sync_ports(
			sca_synchronization_obj_if::TO_SYSTEMC);

	for (unsigned long i = 0; i < (nin_sc + nout_sc); i++)
	{
		//first for all inports
		if (i < nin_sc)
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::FROM_SYSTEMC, i);
		else
			sport = obj->sync_if->get_sync_port(
					sca_synchronization_obj_if::TO_SYSTEMC, i - nin_sc);

		//set calculated T of port
		sport->T_calculated = current_cluster->csync_data.cluster_period
				/ (obj->osync_data.calls_per_period * *(sport->rate));


		sport->t0_calculated = *(sport->t0_set);


		if(sport->T_calculated==sc_core::SC_ZERO_TIME)
		{
			std::ostringstream str;
			str << "Timestep of port: " << sport->port->sca_name()
			    << " becomes zero (may resolution time is to high ) "
				<< " current cluster period: " << current_cluster->csync_data.cluster_period
				<< " divider: " << (obj->osync_data.calls_per_period * *(sport->rate));
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			return;
		}



		if (sport->t0_calculated > sport->T_calculated)
		{
			std::ostringstream str;
			str << "timeoffset: " << sport->t0_calculated
					<< " is greater than timestep: " << sport->T_calculated
					<< " at port: " << sport->port->sca_name()
					<< " (LRM clause 4.1.1.7.8)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			return;
		}

		//write calculated values back
		*(sport->T_set) = sport->T_calculated;
	}
}

///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::analyse_sample_rates()
{
	//for all clusters
	std::vector<sca_cluster_objT*>::iterator cit;
	for (cit = clusters.begin(); cit != clusters.end(); ++cit)
	{
		current_cluster = (*cit);

		scaled_time_lcm = 1;
		schedule_list_length = 0;

		//ignore dead clusters
		if ((*cit)->dead_cluster)
			continue;

		//from the first object all others shuold be reachable
		sca_cluster_objT::iterator sit = (*cit)->begin();

		if (sit == (*cit)->end()) //cluster empty
		{
			// Error should not be possible -> Error in cluster() ??
			std::ostringstream str;
			str << "Internal Error in: "
					<< "sca_synchronization_alg::analyse_sample_rates() "
					<< __FILE__ << " line: " << __LINE__ << std::endl;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		//Assumption (is fullfilled by sca_synchronization_alg::cluster() ) :
		//         a cluster object can be reached (via in- or outports) from
		//         previosly analyzed cluster object ->
		//         at least one port (except the first module) has a port with a
		//         yet analyzed signal -> no dataflow analyzing will be required

		//current multiplier for sampling rates -> trick to prevent re-calculation
		//of yet calculated rates after a sample rate change with remainder
		//(in such a case the LCM must be calculated and all previously calculated
		//rates/obj-calls must be multiplied)
		//Now for every rate/obj-call the additional the current value of the
		//multiplier is stored, on this way the re-calculation has to performed
		//only once after sample rate analyzis finishing
		long multiplier;

		//go through all objects of the current cluster in the order of the list
		// (the order they were push back)
		bool first_obj = true;
		for (sit = (*cit)->begin(); sit != (*cit)->end(); sit++) //go through all objects
		{
			sca_synchronization_alg::sca_sync_objT* obj;

			obj = (*sit);

			//number of channels to the object (inports)
			unsigned long nin = obj->nin;

			//number of channels from the object (outports)
			unsigned long nout = obj->nout;

			if (first_obj) //for the first object no analyzed channel is connected
			{
				first_obj = analyse_sample_rates_first_obj(nin, nout,
						multiplier, obj, sit, cit);
				//only one object in cluster (obj without in and/or outs)
				if (first_obj)
					continue;
			}

			//calculate object calls per period by searching a yet calculated channel
			//and the LCM of the port rate and the assigned max_samples of the channel
			analyse_sample_rates_calc_obj_calls(nin, nout, multiplier, obj);

			//assign to all connected channels the maximum number of samples
			// (port_rate * obj-calls per period) if not yet done otherwise
			// check for consistency
			analyse_sample_rates_assign_max_samples(nin, nout, multiplier, obj);

		} //go through all objects

		schedule_list_length = 0;
		scaled_time_lcm = 1;

		if (current_cluster->T_multiplier == 0)
		{
			std::ostringstream str;
			str
					<< "Error at least one sample period must be assigned per cluster "
					<< std::endl
					<< " the following modules are included in the current cluster"
					<< " (max. 20 printed):" << std::endl;

			long cnt = 0;
			for (sca_cluster_objT::iterator it = current_cluster->begin(); it
					!= current_cluster->end(); it++, cnt++)
			{
				sca_core::sca_implementation::sca_linear_solver* solv;
				solv=dynamic_cast<sca_core::sca_implementation::sca_linear_solver*>((*it)->sync_if);

				str << "\t" << (*it)->sync_if->get_name_associated_names(10) << std::endl;

				if (cnt >= 20)
					break;
			}

			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		//calculate cluster_period
		current_cluster->csync_data.cluster_period = current_cluster->T_cluster
				* (multiplier / current_cluster->T_multiplier);
		current_cluster->T_cluster = current_cluster->csync_data.cluster_period;
		current_cluster->T_multiplier = multiplier;

#ifdef SCA_IMPLEMENTATION_DEBUG

		std::cout << "Cluster period: " << current_cluster->csync_data.cluster_period << std::endl;
#endif

		//now calculate definitive max_samples / calls_per_period using the multiplier
		for (sit = (*cit)->begin(); sit != (*cit)->end(); sit++) //go through all objects
		{
			sca_synchronization_alg::sca_sync_objT* obj = (*sit);
			analyse_sample_rates_calc_def_rates(multiplier, obj);

			//for statistics
			if (obj->osync_data.calls_per_period
					> current_cluster->calls_per_period_max)
			{
				current_cluster->calls_per_period_max
						= obj->osync_data.calls_per_period;
				current_cluster->mod_name_period_max = obj->sync_if->get_name();
			}
			if (obj->osync_data.calls_per_period
					< current_cluster->calls_per_period_min)
			{
				current_cluster->calls_per_period_min
						= obj->osync_data.calls_per_period;
				current_cluster->mod_name_period_min = obj->sync_if->get_name();
			}

			//sdatas.call_counter           = &(obj->sync_if->get_call_counter());
			//sdatas.calls_per_period       = &(obj->sync_if->get_calls_per_period());

			obj->sync_if->set_sync_data_references(current_cluster->csync_data,
					obj->osync_data);

		}

		(*cit)->schedule_list_length = schedule_list_length;
		(*cit)->scaled_time_lcm = scaled_time_lcm;

	} //for all clusters
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::analyse_timing()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void sca_synchronization_alg::generate_scheduling_list_expand_list(
		sca_synchronization_alg::sca_cluster_objT& cluster)
{
	try
	{
		//allocate memory for schedule elemnts
		cluster.scheduling_elements
				= new schedule_element[cluster.schedule_list_length];
		//create scheduling list
		cluster.scheduling_list.resize(cluster.schedule_list_length);
	} catch (...)
	{
		std::ostringstream str;
		str
				<< "Can't allocate memory for scheduling list - the scheduling list"
				<< " of the cluster containing the module: "
				<< (*cluster.begin())-> sync_if->get_name_associated_names(5) << " is to long "
				<< std::endl
				<< "the cluster is may bad conditioned or has very high sample "
				<< "rate ratios which may have no greatest common divider";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	unsigned long cnt = 0;
	std::vector<schedule_element*>::iterator slit =
			cluster.scheduling_list.begin();
	for (sca_cluster_objT::iterator it = cluster.begin(); it != cluster.end(); it++)
	{
		sca_sync_objT* obj;
		schedule_element* ele;

		::sc_core::sc_object* sched_obj;
		sca_core::sca_implementation::sc_object_method proc_method;
		obj = (*it);
		obj->sync_if->get_processing_method(sched_obj, proc_method);

		unsigned long scaled_time_step;
		unsigned long scaled_cur_time;
		unsigned long calls_per_period;

		calls_per_period = obj->osync_data.calls_per_period;
		scaled_time_step = cluster.scaled_time_lcm / calls_per_period;
		scaled_cur_time = 0;

		//initialize scheduling list
		for (unsigned long i = 0; i < calls_per_period; i++)
		{
			ele = &(cluster.scheduling_elements[cnt]);
			ele->obj = obj;
			ele->schedule_obj = sched_obj;
			ele->proc_method = proc_method;
			ele->scaled_schedule_time = scaled_cur_time;
			ele->call = i + 1;
			ele->call_counter = &(obj->osync_data.call_counter);
			ele->allow_processing_access = &(obj->osync_data.allow_processing_access_flag);
			*(ele->allow_processing_access) = false;
			(*slit) = ele;
			slit++;
			cnt++;
			scaled_cur_time += scaled_time_step;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

//if you use g++ 3.x you donot need this procedure instead you can use
//the simpler sort - function
static bool less_time(sca_synchronization_alg::schedule_element* el1,
		sca_synchronization_alg::schedule_element* el2)
{
	return el1->less_time(el2);
}

inline void sca_synchronization_alg::generate_scheduling_list_schedule(
		sca_synchronization_alg::sca_cluster_objT& cluster)
{
	std::vector<schedule_element*>& schedule_vector(cluster.scheduling_list);
	std::list<schedule_element*>& tmp_list(cluster.scheduling_list_tmp);

	//pre-odering using time

	//if you use g++ 3.x you can enable this line instead the following and
	//the procedure less_time
	//sort(schedule_vector.begin(),schedule_vector.end(),
	//	   mem_fun(&schedule_element::less_time));

	sort(schedule_vector.begin(), schedule_vector.end(), less_time);

	//copy to list to allow faster remove
	tmp_list.resize(schedule_vector.size());
	copy(schedule_vector.begin(), schedule_vector.end(), tmp_list.begin());

	std::vector<schedule_element*>::iterator vslit = schedule_vector.begin();
	std::list<schedule_element*>::iterator lslit;
	while (true)
	{
		//find first scheduable object
		//if you use g++ 3.x you can enable the following lines instead the
		//for -loop
		//lslit=find_if(tmp_list.begin(),tmp_list.end(),
		//                  mem_fun(&schedule_element::schedule_if_possible));

		for (lslit = tmp_list.begin(); lslit != tmp_list.end(); lslit++)
			if ((*lslit)->schedule_if_possible())
				break;

		//if no further object scheduable all elements scheduled or error
		if (lslit == tmp_list.end())
			break;

		(*vslit++) = (*lslit); //copy object back to vector
		tmp_list.erase(lslit); //remove object from list
	}

	if (tmp_list.size() != 0)
	{
		std::ostringstream str;
		str << "System not scheduable -";
		if (tmp_list.size() == schedule_vector.size())
		{
			str << " no element scheduable" << std::endl;
		}
		else
		{
			vslit--;
			str << " last scheduable element : " << std::endl;
			str << "                  " << (*vslit)->schedule_obj->name()
					<< std::endl;

			str << std::endl << "     Current list: " << std::endl;
			for (unsigned int i = 0; i < schedule_vector.size()
					- tmp_list.size(); i++)
			{
				str << "                                "
						<< schedule_vector[i]->schedule_obj->name()
						<< std::endl;
			}
		}
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
}

void sca_synchronization_alg::generate_scheduling_list()
{
	for (unsigned long i = 0; i < clusters.size(); i++)
	{
		current_cluster = clusters[i];

		generate_scheduling_list_expand_list(*current_cluster);
		generate_scheduling_list_schedule(*current_cluster);
	}
}

///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::sca_cluster_objT::print()
{
	for (iterator i = begin(); i != end(); i++)
	{
		int nin = (*i)->sync_if-> get_number_of_sync_ports(
				sca_synchronization_obj_if::FROM_ANALOG);
		int nout = (*i)->sync_if-> get_number_of_sync_ports(
				sca_synchronization_obj_if::TO_ANALOG);

		std::cout << "\t" << (*i)->sync_if->get_name_associated_names();

		std::cout << "\t" << (*i)->sync_if->get_calls_per_period();

		std::cout << " \t nin: " << nin << " nout: " << nout << "\t";

		sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port
				*cport;
		for (long cnt = 0; cnt < nin; cnt++)
		{
			cport = (*i)->sync_if->get_sync_port(
					sca_synchronization_obj_if::FROM_ANALOG, cnt);
			std::cout << "  " << *(cport->rate) << "," << *(cport->delay);
		}
		std::cout << "   | |   ";
		for (long cnt = 0; cnt < nout; cnt++)
		{
			cport = (*i)->sync_if->get_sync_port(
					sca_synchronization_obj_if::TO_ANALOG, cnt);
			std::cout << "  " << *(cport->rate) << "," << *(cport->delay);
		}
		std::cout << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////

void sca_synchronization_alg::sca_cluster_objT::print_schedule_list()
{
	int k = 1;
	for (std::vector<schedule_element*>::iterator sit = scheduling_list.begin(); sit
			!= scheduling_list.end(); sit++, k++)
	{
		std::cout << k << "\t";
		std::cout << (*sit)->obj->sync_if->get_name_associated_names() << "\t";
		std::cout << (*sit)->scaled_schedule_time << "\t";
		std::cout << (*sit)->call << " of "
				<< (*sit)->obj->osync_data.calls_per_period << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////


}
}
