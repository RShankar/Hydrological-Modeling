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

 sca_simcontext.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 13.05.2009

 SVN Version       :  $Revision: 1143 $
 SVN last checkin  :  $Date: 2011-02-05 22:48:02 +0100 (Sat, 05 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_simcontext.h 1143 2011-02-05 21:48:02Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_SIMCONTEXT_H_
#define SCA_SIMCONTEXT_H_

namespace sca_util
{
namespace sca_implementation
{
class sca_trace_file_base;
}
}


namespace sca_core
{
class sca_module;

namespace sca_implementation
{
class sca_solver_manager;
class sca_view_manager;
class sca_object_manager;
}
}

namespace sca_ac_analysis
{
namespace sca_implementation
{
class sca_ac_domain_db;
}
}

#include <vector>

namespace sca_core
{
namespace sca_implementation
{

class systemc_ams_initializer;

/**
 Glogal class for managing simulation instances
 */
class sca_simcontext
{
public:

	//constructor
	sca_simcontext();

	//destructor
	~sca_simcontext();

	//module data base
	sca_core::sca_implementation::sca_object_manager* get_sca_object_manager();

	//get data bases for different layers
	sca_core::sca_implementation::sca_solver_manager* get_sca_solver_manager();
	sca_core::sca_implementation::sca_view_manager* get_sca_view_manager();

	std::vector<sca_util::sca_implementation::sca_trace_file_base*>* get_trace_list();
	bool& all_traces_initialized();


	sca_core::sca_module* get_current_sca_module();

	bool initialized();

	//methods to get simulation kind (e.g. for supressing synchronization
	//during ac-domain simulation
	bool time_domain_simulation();
	void set_no_time_domain_simulation();
	void set_time_domain_simulation();
	bool construction_finished();

	sca_ac_analysis::sca_implementation::sca_ac_domain_db* ac_db;

	unsigned long& get_information_mask();

private:

	//reference to main object manager
	sca_core::sca_implementation::sca_object_manager* m_sca_object_manager;

	sca_core::sca_implementation::sca_solver_manager* m_sca_solver_manager;
	sca_core::sca_implementation::sca_view_manager* m_sca_view_manager;

	//refernece to main SystemC simcontext
	::sc_core::sc_simcontext *sc_kernel_simcontext;

	void sca_pln(); //prints copyright and revision

	bool time_domain_simulation_flag;

	bool traces_initialized;

	unsigned long information_mask;

	systemc_ams_initializer* scams_init;

};

//global function
sca_core::sca_implementation::sca_simcontext* sca_get_curr_simcontext();

} //namespace implementation
} //namespace sca_core

#endif /* SCA_SIMCONTEXT_H_ */
