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

 sca_module.h - base class for all sca_modules

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 03.03.2009

 SVN Version       :  $Revision: 1031 $
 SVN last checkin  :  $Date: 2010-06-04 18:26:43 +0200 (Fri, 04 Jun 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_module.h 1031 2010-06-04 16:26:43Z karsten $

 *****************************************************************************/

/*
 LRM clause 3.2.1
 The class sca_core::sca_module shall define the base class to
 derive primitive modules for the predefined models of computation.
 */

/*****************************************************************************/

#ifndef SCA_MODULE_H_
#define SCA_MODULE_H_

//// begin implementation specific declarations //////////////

namespace sca_core
{

class sca_prim_channel;

namespace sca_implementation
{
class sca_port_base;
class sca_solver_base;
class sca_view_manager;
class sca_object_manager;
class sca_view_base;
class sca_conservative_module;
class sca_conservative_view;

}
}

namespace sca_tdf
{
class sca_module;

namespace sca_implementation
{
  class sca_tdf_signal_impl_base;
  class sca_tdf_view;
  class sca_trace_variable_base;
}
}

namespace sca_eln
{
class sca_module;
class sca_node;

namespace sca_implementation
{
class sca_eln_view;
}
}

namespace sca_lsf
{
class sca_module;
class sca_signal;

namespace sca_implementation
{
class sca_lsf_view;
}
}



namespace sca_ac_analysis
{
namespace sca_implementation
{
class sca_ac_domain_solver;
}
}


//// end implementation specific declarations //////////////


//begin LRM copy
namespace sca_core
{

class sca_module: public ::sc_core::sc_module
{
public:
	virtual const char* kind() const;

	virtual void set_timestep(const sca_core::sca_time&);
	virtual void set_timestep(double, sc_core::sc_time_unit);

	virtual sca_core::sca_time get_timestep() const;

protected:

	sca_module();
	virtual ~sca_module();

	//////////// begin implementation specific section ////////////////

private:

	friend class sca_core::sca_implementation::sca_port_base;
	friend class sca_core::sca_implementation::sca_solver_base;
	friend class sca_core::sca_implementation::sca_view_manager;
	friend class sca_core::sca_implementation::sca_object_manager;
	friend class sca_tdf::sca_implementation::sca_tdf_signal_impl_base;
	friend class sca_core::sca_prim_channel;
	friend class sca_tdf::sca_module;
	friend class sca_eln::sca_module;
	friend class sca_lsf::sca_module;
	friend class sca_core::sca_implementation::sca_conservative_module;
	friend class sca_core::sca_implementation::sca_conservative_view;
	friend class sca_tdf::sca_implementation::sca_tdf_view;
	friend class sca_eln::sca_implementation::sca_eln_view;
	friend class sca_lsf::sca_implementation::sca_lsf_view;
	friend class sca_tdf::sca_implementation::sca_trace_variable_base;
	friend class sca_eln::sca_node;
	friend class sca_lsf::sca_signal;
	friend class sca_ac_analysis::sca_implementation::sca_ac_domain_solver;

	typedef std::vector<sca_core::sca_implementation::sca_port_base*> sca_port_base_listT;
	typedef sca_port_base_listT::iterator sca_port_base_list_iteratorT;

	sca_core::sca_implementation::sca_solver_base* sync_domain;

	unsigned long register_port(sca_core::sca_implementation::sca_port_base* port);

	//!!!! required due private sc_module portlist !!!!!!
	sca_port_base_listT port_list;

	sca_port_base_listT& get_port_list();

	long get_view_id();

public:
	/** Returns flag for allowing access to ports for processing phase
		 */
	const bool* get_allow_processing_access_flag();

	/** Returns flag for allowing access for intializing e.g. ports
		 */
	bool get_allow_initialize_access_flag();

	/** Returns flag for allowing access for setting attributes
		 */
	bool get_allow_attributes_access_flag();

	bool allow_initialize_access_flag;
	bool allow_attributes_access_flag;

	/** Returns an interface to the synchronization cluster to which
	 * the module is assigned - a sca_module is assigned to exactly
	 * one cluster (others should make no sense ???)
	 */
	sca_core::sca_implementation::sca_solver_base* get_sync_domain();


	sca_core::sca_time sca_get_time() const;

	/**
	 * Requests maximum absolute next time, returns number of calls
	 * before request can be accepted and in mtime the time which was
	 * accepted - if there are other requests the module is may be
	 * called earlier
	 */
	long sca_next_max_time(sc_core::sc_time mtime);

	/** Requests next maximum time step, returns number of calls before
	 * the new time step can be accepted - if there are other requests, the
	 * resulting time step is may be smaller, the time step will be hold until
	 * other requests occur
	 */
	long sca_next_max_time_step(sc_core::sc_time mtime);

	/** Registers an event which triggers next cluster activation and
	 * returns the earliest time when the event can be accepted -
	 * events before will be ignored
	 */
	sc_core::sc_time sca_synchronize_on_event(const sc_core::sc_event& ev);

	/**
	 * Synchronizes cluster activation with an event on a converter port
	 * the module is called latest the event occurs (may it is called
	 * earlier due cluster_period or next_start_time); the return time
	 * is the earliest time when the event can be accepted (due to
	 * multi-rate systems)
	 */
	sc_core::sc_time sca_synchronize_on_event(
			const sca_core::sca_implementation::sca_port_base& iport);

	//the view manger is allowed to access the assigned view

	sca_core::sca_implementation::sca_view_base* view_interface;
	long view_id;

	//elaborates the sca_module
	//permit overloading to add elaboration actions - overloaded method must call this base method
	virtual void elaborate();


	sca_core::sca_time timestep_set;
	bool               timestep_is_set;
	bool               timestep_elaborated;

	/////////// end implementation specific section //////////////////////


};

//#define SCA_CTOR(name)       implementation-defined name( sc_core::sc_module_name )
#define SCA_CTOR(name)       name( sc_core::sc_module_name )

} // namespace sca_core


//end LRM copy

#endif /* SCA_MODULE_H_ */
