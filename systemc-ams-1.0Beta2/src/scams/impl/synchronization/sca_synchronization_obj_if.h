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

 sca_synchronization_obj_if.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 14.05.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_synchronization_obj_if.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_SYNCHRONIZATION_OBJ_IF_H_
#define SCA_SYNCHRONIZATION_OBJ_IF_H_


#include <systemc-ams>
#include "scams/impl/synchronization/sca_sync_value_handle.h"
#include "scams/impl/util/tracing/sca_trace_object_data.h"

namespace sca_core
{

class sca_prim_channel;

namespace sca_implementation
{

class sca_port_base;
class sca_sync_alg_objT;
class sca_solver_base;
class sca_synchronization_obj_if;
class sca_sync_value_handle_base;

typedef void (::sc_core::sc_object::*sc_object_method)();

/**
 Interface class for the synchronization layer. The interface will
 be implemented by @ref #sca_solver_base, due a solver is a
 synchronization object.
 Additional methods for configurating the synchronization
 and to change this configuration during simulation are
 provided. The methods will be implemented by the synchronization.
 */
class sca_synchronization_obj_if
{
public:

	virtual ~sca_synchronization_obj_if()
	{
	}

	enum directionT
	{
		NOT_DEFINED_DIRECTION,

		TO_ANALOG, //outport to other analog obj
		FROM_ANALOG, //inport from other analog obj

		TO_SYSTEMC, //outport to SystemC kernel
		FROM_SYSTEMC
	//inport from SystemC kernel
	};



	/**
	 * Adds trace which is activated (the trace method of the object is called)
	 * after each cluster calculation by the synchronization layer
	 * (traces usually signals between solvers)
	 */
	virtual void add_cluster_trace(sca_util::sca_implementation::sca_trace_object_data& tr_obj) = 0;

	/**
	 * Adds trace which is activated (the trace method of the object is called)
	 * by the specific solver (traces usually signals/values inside a solver)
	 */
	virtual void add_solver_trace(sca_util::sca_implementation::sca_trace_object_data& tr_obj) = 0;



	/**
	 struct which collects the properties/attributtes
	 of a synchronization port
	 */
	struct sca_sync_port
	{

		//reference to port which represents the sync-port
		sca_port_base* port;

		//reference to number of data items which has to be
		//proceeded per call
		unsigned long* rate;

		//reference to number of data items which are
		//included/written during initialization
		unsigned long* delay;

		//reference to set time distance of the data items
		::sc_core::sc_time* T_set;

		//reference to the set absolute time of the first data item
		::sc_core::sc_time* t0_set;

		//reference to the connected channel
		sca_core::sca_prim_channel* channel;

		//direction of the connection (port)
		directionT dir;

		//calculated  time distance of data items by the sync-algorithm
		::sc_core::sc_time T_calculated;

		//calculated time distance of data items by the sync-algorithm
		::sc_core::sc_time t0_calculated;

		unsigned long sync_port_number;
		sca_synchronization_obj_if* parent_obj;

	};


	/** registers method which is called after the event ev
	 * will be used to trace an channel from SystemC-kernel
	 * a handle will be returned
	 * abstract interface method implemented in sca_solver_base
	 */
	virtual void register_sc_value_trace(const ::sc_core::sc_event& ev,
			sca_sync_value_handle_base& handle)=0;

	/** if SystemC has not reached time the sca process of the current cluster
	 * suspended, after resume the corresponding value
	 * (stored by the trace process) is returned
	 * abstract interface method implemented in sca_solver_base
	 */
	virtual void get_sc_value_on_time(::sc_core::sc_time time,
			sca_sync_value_handle_base& handle) = 0;

	//type dependent user method (used in concrete synchronization channel)
	template<class T>
	const T& get_sc_value_on_time(::sc_core::sc_time time,
			sca_sync_value_handle<T>& handle)
	{
		get_sc_value_on_time(time,
				static_cast<sca_sync_value_handle_base&> (handle));
		return handle.read();
	}

	/** if no value in the chain (stored by the trace process) the sca - cluster
	 * process is suspended, after resume the value is returnde
	 * abstract interface method implemented in sca_solver_base
	 */
	virtual void get_sc_value_by_order(sca_sync_value_handle_base& handle) = 0;

	//type dependent user method (used in concrete synchronization channel)
	template<class T>
	const T& get_sc_value_by_order(sca_sync_value_handle<T>& handle)
	{
		get_sc_value_by_order(static_cast<sca_sync_value_handle_base&> (handle));
		return handle.read();
	}

	/** registers synchronization from a sca domain to a sc domain -
	 * limits the sc time to the first sca schedule time
	 */
	virtual void register_sca_schedule(::sc_core::sc_time next_time,
			sca_sync_value_handle_base& handle) =0;

	/** writes with write_method a value to a channel at time ctime or may be
	 * timeless - the SystemC-kernel will suspend (and thus resume the sca -
	 * cluster) not later then next_time - during this activation the mehtod
	 * must be called to remove the old and schedule a new next_time -
	 * otherwise it should give an exception, due we deadlocked
	 * for timeless (ordered) synchronization next_time can be set to a large
	 * value or SC_INVALID_TIME - in this case the sc_channel suspends if a
	 * value required (we have to do nothing therefore)
	 * abstract interface method
	 */
	virtual void write_sc_value(::sc_core::sc_time ctime, ::sc_core::sc_time next_time,
			sca_sync_value_handle_base& handle) = 0;

	//type dependent user method (used in concrete synchronization channel)
	template<class T>
	void write_sc_value(::sc_core::sc_time ctime, ::sc_core::sc_time next_time,
			sca_sync_value_handle<T>& handle, T& value)
	{
		handle.write_tmp(value);
		write_sc_value(ctime, next_time, handle);
	}

	/** returns current cluster start time -> of the current period
	 * calls of set_max_cluster_start_time of the current period have no
	 * influence
	 */
	virtual ::sc_core::sc_time get_cluster_start_time() = 0;

	/** returns the current  period of the object of the current cluster call
	 * this time corresponds to cluster_period/(obj. calls per period)
	 * a change in the current cluster call (using set_max_period) has not yet
	 * influence
	 */
	virtual ::sc_core::sc_time get_current_period() = 0;

	/** sets for the next cluster start the start maximal time - if the method
	 *  called from different objects the minimum time will be choosen - the
	 * method returns the number of object calls before the time will be
	 * accepted (in a multirate system the number of object calls in the current
	 * period - in single rate system this number will be always 0)ntime is
	 * setted to the time used if the return value >0
	 * (ntime = ret_val * cluster_period/calls_per_period)
	 */
	virtual long set_max_start_time(::sc_core::sc_time& ntime) = 0;

	/** sets the maximum time period which will be valid up to the next cluster
	 * start (the time is the cluster_period/(obj calls per cluster) ), if the
	 * time is setted from othe objects of the clusters also the time resulting
	 * from the minimum cluster_period will be used
	 * the method returns the number of object calls in the current period
	 * before the time will be accepted  (in a single rate system this number
	 * will be allways 0)
	 */
	virtual long set_max_period(::sc_core::sc_time& nperiod) = 0;

	/** registers an event for activation of the next cluster start
	 * the method returns the time before the event can be
	 * accepted (events before will be ignored) - for single rate always <
	 * cluster period and if no t0 specified it should be equal to the
	 * current time
	 * after restarting the cluster the events will be removed from the list
	 * -> thus the events has to be re-registered every cluster period
	 */
	virtual ::sc_core::sc_time register_activation_event(const ::sc_core::sc_event& ev)=0;

	/**
	 Returns number of synchronization ports to other synchronization objects
	 */
	virtual unsigned long get_number_of_sync_ports(directionT direction)=0;

	/**
	 Returns the number synchronization ports to/from an other
	 synchronization object, or systemc discrete event kernel
	 (for the first channel number=0) if number>= number of channels a null
	 pointer is returned.
	 */
	virtual sca_sync_port* get_sync_port(directionT direction,
			unsigned long number)=0;

	/**
	 Returns initialization method for synchronization object.
	 */
	virtual void get_initialization_method(::sc_core::sc_object*&, sc_object_method&)=0;

	/**
	 Returns processing method for synchronization object.
	 */
	virtual void get_processing_method(::sc_core::sc_object*&, sc_object_method&)=0;

	/**
	 Returns post processing method for synchronization object.
	 */
	virtual void get_post_method(::sc_core::sc_object*&, sc_object_method&)=0;

	/**
	 Returns reference to number of solver calls per cluster period
	 */
	virtual const long& get_calls_per_period()=0;

	/**
	 Returns reference to cluster period time
	 */
	virtual const ::sc_core::sc_time& get_cluster_period() =0;

	/**
	 Returns reference to call counter
	 */
	virtual const sc_dt::int64*& get_call_counter_ref()=0;

	/** terminates synchronization objects (calls post methods)
	 */
	virtual void terminate()=0;

	/**
	 Resets all states to allow re-analysing of the graphs
	 */
	virtual void reset()=0;

	/**
	 Returns name of synchronization object -> for debugging and
	 error messages
	 */
	std::string get_name() const
	{
		return sync_obj_name;
	}

	virtual std::string get_name_associated_names(int max_num=-1)
	{
		return get_name();
	}

	struct cluster_sync_data
	{
		::sc_core::sc_time cluster_period;
		::sc_core::sc_time cluster_start_time;
		::sc_core::sc_time next_cluster_period;
		::sc_core::sc_time next_cluster_start_time;

		bool request_new_start_time;
		bool request_new_period;

		::sc_core::sc_event cluster_resume_event;
		::sc_core::sc_time cluster_resume_event_time;

		std::vector<const ::sc_core::sc_event*> reactivity_events;

		std::vector<sca_util::sca_implementation::sca_trace_object_data*> traces;
	};

	struct object_sync_data
	{
		sc_dt::int64 call_counter;
		long calls_per_period;
		bool allow_processing_access_flag;
	};

	sca_core::sca_time timestep_set;
	sca_core::sca_time timestep_calculated;
	sca_core::sca_module* module_timestep_set;
	bool               timestep_is_set;
	sca_core::sca_module* module_min_timestep_set;
	sca_core::sca_time    min_timestep_set;

protected:

	std::string sync_obj_name;

	friend class sca_synchronization_alg;

	//is used by sca_synchronization_alg during analysis (scheduling list set up)
	bool dead_cluster;
	long cluster_id;

	bool not_clustered()
	{
		return (cluster_id < 0);
	}

private:

	//position in cluster list
	unsigned long sync_id;

	//position in overall list
	unsigned long id;

	//mechanism to transfer datas from the sync-cluster to the solver obj
	virtual void set_sync_data_references(cluster_sync_data& crefs,
			object_sync_data& orefs) = 0;

};

//typedef sca_synchronization_obj_if::sca_sync_port sca_sync_port;
class sca_sync_port : public sca_synchronization_obj_if::sca_sync_port{};


} //namaespace sca_implementation
} //namespace sca_core


#endif /* SCA_SYNCHRONIZATION_OBJ_IF_H_ */
