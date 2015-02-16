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

  sca_synchronization_alg.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 25.08.2009

   SVN Version       :  $Revision: 1187 $
   SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_synchronization_alg.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_SYNCHRONIZATION_ALG_H_
#define SCA_SYNCHRONIZATION_ALG_H_

#include <systemc>
#include "scams/impl/synchronization/sca_synchronization_obj_if.h"


#include<vector>
#include<list>


namespace sca_core
{
namespace sca_implementation
{


/** forward class definition */
class sca_synchronization_layer_process;

/**
  Implements synchronization algorithm. Currently
  a static dataflow scheduler is implemented for
  synchronization.
*/
class sca_synchronization_alg
{
public:
 class sca_sync_objT
 {
  public:
    sca_synchronization_obj_if* sync_if;

    //sca_sync_objT* driver;
    std::vector<sca_sync_objT*> inports;

    unsigned long nin;
    std::vector<unsigned long> in_rates;
    std::vector<unsigned long> sample_inports;

    unsigned long nout;
    std::vector<unsigned long> out_rates;
    //outports with references to the connected inports
    std::vector<std::vector<unsigned long*> > next_inports;

    sca_synchronization_obj_if::object_sync_data osync_data;
    //long long call_counter;
    //long calls_per_period;

    inline void call_init_method()
    {
    	sc_core::sc_object* obj;
    	sca_core::sca_implementation::sc_object_method meth;

    	osync_data.call_counter = -1;

    	sync_if->get_initialization_method(obj,meth);

    	sca_core::sca_module* cmod=dynamic_cast<sca_core::sca_module*>(obj);
    	if(cmod!=NULL) cmod->allow_initialize_access_flag=true;

    	if(obj!=NULL)  (obj->*(meth))();

    	if(cmod!=NULL) cmod->allow_initialize_access_flag=false;
    }


    void init(int ninports, int noutports)
    {
    	inports.resize(ninports,0);
    	in_rates.resize(ninports,0);
    	sample_inports.resize(ninports,0);
    	nin = ninports;

    	out_rates.resize(noutports,0);
    	next_inports.resize(noutports);
    	nout = noutports;
    }

    inline bool schedule_if_possible()
    {
    	//cout  << "try schedule: " << sync_if->get_name();
    	//for(unsigned long i=0;i<nin;i++)
    	//                           cout << "   " <<  sample_inports[i] << "/"<< in_rates[i];
    	//cout << endl;

    	//check schedubility
    	for(unsigned long i=0;i<nin;i++)
    										if(sample_inports[i] < in_rates[i]) return false;

    	//cout << endl << "------------------------------------------------------" << endl;

    	//schedule inports
    	for(unsigned long i=0;i<nin;i++) sample_inports[i]-=in_rates[i];

    	//schedule outports (write to all connected inports)
    	for(unsigned long i=0;i<nout;i++)
    		for(unsigned long j=0;j<next_inports[i].size();j++)
    								*(next_inports[i][j])+=out_rates[i];
        return true;
    }

    //is used by sca_synchronization_alg during analysis (scheduling list set up)
    long multiple;

    bool dead_cluster;
    long cluster_id;

    sca_sync_objT();

 };


 //memory for synchronization objects
 //to save allocation and initialization time
 sca_sync_objT* sync_obj_mem;

 /** class which stores informations regarding scheduling time
 */

 class schedule_element
 {
 public:

	 ::sc_core::sc_object*          schedule_obj;
  	sc_object_method    proc_method;


  	unsigned long call;

  	sc_dt::int64* call_counter;
  	bool*      allow_processing_access;

  	sca_sync_objT* obj;

  	//pre-odering criteria
   	inline  bool less_time (schedule_element* ele1)
 	{
 			return (this->scaled_schedule_time < ele1->scaled_schedule_time);
 	}

	 inline bool schedule_if_possible()
	 {
	 	return(obj->schedule_if_possible());
	 }

	 inline void run()
	 {
	 	(*call_counter)++;
	 	(*allow_processing_access)=true;
	 	(schedule_obj->*(proc_method))();
	 	(*allow_processing_access)=false;
	 }


  	unsigned long scaled_schedule_time;
 };



/**
  Class for encapsulating synchronization cluster data
*/
class sca_cluster_objT
{
	 std::vector<sca_sync_objT*>  obj_list;

public:

     typedef std::vector<sca_sync_objT*>::iterator iterator;

     bool dead_cluster;

     sca_synchronization_layer_process* csync_mod;

     //data for statistics
     sc_dt::int64 calls_per_period_max;
     std::string  mod_name_period_max;
     sc_dt::int64 calls_per_period_min;
     std::string  mod_name_period_min;

     void push_back(sca_sync_objT* obj) { obj_list.push_back(obj); }

     const iterator end()    { return obj_list.end();   }
     const iterator begin() { return obj_list.begin(); }
     long size() { return obj_list.size(); }

     sca_sync_objT*& operator [] (unsigned long n) { return obj_list[n]; }

     sca_synchronization_obj_if::cluster_sync_data csync_data;

     //cluster synchronization data
//     sc_time   cluster_period;
//     sc_time   cluster_start_time;
//     sc_time   next_cluster_period;
//     sc_time   next_cluster_start_time;
//     sc_event  cluster_resume_event;
//     sc_time   cluster_resume_event_time;


     unsigned long schedule_list_length;
     unsigned long scaled_time_lcm;


     sca_core::sca_time        T_cluster;         //period of cluster
     unsigned long  T_multiplier;      //multiplier of T_cluster assignment
     sca_sync_objT* T_last_obj;        //reference to obj which assigned T_cluster
     long           T_last_n_sample;   //number of sample
      //reference to port which assigned T_cluster
     sca_synchronization_obj_if::sca_sync_port* T_last_port;
     //time of last analyzed object / port
     sca_core::sca_time T_last;

     //pointer to memory of schedule elements - to speed up
     //sorting by copy pointers instead of complete classes
     schedule_element* scheduling_elements;

     std::vector<schedule_element*> scheduling_list;

     //fast remove possible
     std::list<schedule_element*> scheduling_list_tmp;

     //for debugging
     void print();
     void print_schedule_list();

     sca_cluster_objT()
     {
        dead_cluster = false;
        csync_mod    = NULL;

        csync_data.cluster_period = NOT_VALID_SCA_TIME();
        csync_data.cluster_resume_event_time = NOT_VALID_SCA_TIME();

        csync_data.next_cluster_period       = ::sc_core::SC_ZERO_TIME;
        csync_data.next_cluster_start_time   = ::sc_core::SC_ZERO_TIME;

        csync_data.request_new_start_time    = false;
        csync_data.request_new_period        = false;

        schedule_list_length = 0;
        scaled_time_lcm      = 0;

        T_last_obj  = NULL;
        T_last_port = NULL;

        T_multiplier=0;
        T_cluster=::sc_core::SC_ZERO_TIME;

        calls_per_period_max=0;
        calls_per_period_min=0x7fffffff;

     }

};

 public:

	sca_synchronization_alg();
	~sca_synchronization_alg();

  /** Initializes datastructures and calculates the scheduling list */
  void initialize(std::vector<sca_synchronization_obj_if*>& solvers);

 private:
  
  typedef std::vector<sca_synchronization_obj_if*> sca_sync_obj_listT;


  //list for all (unclustered) objects
  sca_sync_obj_listT sync_objs;

  //list for objects which can't be analyzed in the first step, due zero port_rates
  sca_sync_obj_listT remainder_list;

  //list of synchronization clusters
  std::vector<sca_cluster_objT*> clusters;

  //reference to cluster which is currently analyzed
  sca_cluster_objT* current_cluster;

  sc_dt::uint64 schedule_list_length;
  sc_dt::uint64 scaled_time_lcm;
  bool scheduling_list_warning_printed;


  void check_closed_graph();
  void cluster();

  void analyse_sample_rates();
  bool analyse_sample_rates_first_obj(
						unsigned long&                             nin,
						unsigned long&                             nout,
						long&                                      multiplier,
						sca_synchronization_alg::sca_sync_objT*&   obj,
						sca_cluster_objT::iterator&                sit,
						std::vector<sca_cluster_objT*>::iterator&       cit
									);

void analyse_sample_rates_calc_obj_calls (
						unsigned long&                              nin,
						unsigned long&                              nout,
						long&                                       multiplier,
						sca_synchronization_alg::sca_sync_objT*&    obj
										);

void analyse_sample_rates_assign_max_samples (
						unsigned long&                              nin,
						unsigned long&                              nout,
						long&                                       multiplier,
                        sca_synchronization_alg::sca_sync_objT*&    obj
											  );

  void check_sample_time_consistency (
                                sca_synchronization_alg::sca_sync_objT*& obj,
                                sca_synchronization_obj_if::sca_sync_port* sport,
                                long multiplier
                                      );



void analyse_sample_rates_calc_def_rates (
						long&                                       multiplier,
						sca_synchronization_alg::sca_sync_objT*&    obj
									     );

  //greatest common divider (recursive function) called by lccm
sc_dt::uint64
          analyse_sample_rates_gcd(sc_dt::uint64 x, sc_dt::uint64 y);

  //lowest common multiple (LCM)
  unsigned long analyse_sample_rates_lcm(unsigned long n1, unsigned long n2);

  void move_connected_objs(sca_cluster_objT* cluster,long cluster_id,
                           sca_sync_objT*  current_obj );
  sca_sync_objT* move_obj_if_not_done(sca_synchronization_obj_if* sync_obj,
										    sca_cluster_objT*    cluster,
                                            long                 cluster_id );


   void analyse_timing();

   void generate_scheduling_list();

   bool comp_time(schedule_element* e1, schedule_element* e2);
   void generate_scheduling_list_expand_list(
							sca_synchronization_alg::sca_cluster_objT& cluster
												                   );
   void generate_scheduling_list_schedule(
							sca_synchronization_alg::sca_cluster_objT& cluster
																									 );

};




}
}


#endif /* SCA_SYNCHRONIZATION_ALG_H_ */
