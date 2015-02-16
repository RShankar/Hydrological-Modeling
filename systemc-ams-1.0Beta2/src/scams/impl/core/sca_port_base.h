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

  sca_port_base.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 15.05.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_port_base.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_PORT_BASE_H_
#define SCA_PORT_BASE_H_



namespace sca_core
{

class sca_interface;

namespace sca_implementation
{

class sca_sync_value_handle_base;

/**
	Abstract base class for sca_ports. This class is required
	to handle ports type and interface independent.
*/
class sca_port_base : public sca_util::sca_traceable_object
{

public:

  /**
  	Returns the sca_module which contains this port (the port is
  	a member of that module).
  */
  sca_module* get_parent_module() const;

  //only if accessed via abstract base class
  //otherwise a port is derived from sc_object
  const char* sca_name();
  const char* sca_basename();

  //constructors
  sca_port_base();
  sca_port_base(const char* nm);

  /**
  	Returns a pointer to the first bound interface -
  	must be always a sca_interface - if a sc_interface is
  	bound, a pointer to the in-port sca_channel will be
  	returned.
  */
  virtual const sca_core::sca_interface* sca_get_interface() const {return m_sca_if;}
  virtual       sca_core::sca_interface* sca_get_interface()       {return m_sca_if;}
  virtual       sc_core::sc_interface*  sc_get_interface()       = 0;
  virtual const sc_core::sc_interface*  sc_get_interface() const = 0;

  //destructor
  virtual ~sca_port_base();

  /**
  	The enumaration defines the possible port categories.
  	These categories are used for analyzing the communication
  	graphs.
  */
  enum port_typeT
  {
	SCA_NOT_DEFINED_PORT = 0,

	SCA_IN_PORT = 1,
	SCA_OUT_PORT = 2,
	SCA_INOUT_PORT = 3,
	SCA_CONSERVATIVE_PORT = 4,

	SCA_SC_IN_PORT = 9,
	SCA_SC_OUT_PORT = 10,
	SCA_SC_INOUT_PORT = 11
  };

  /** This method returns the port's assigned type. */
  port_typeT  get_port_type() const { return  port_type; }

  /** This method tries to determine the bound sca_interfaces.
      If a sc_interface is bound then a sca - synchronization - channel
      will be created (using the virtual method
      @ref #create_synchronization_channel() ). Therefore, that
      creation method has to be overloaded by a sca_port specialization.
      Else an exception will occur.
  */
  void elaborate_port();

  /** This virtual method will be called after the port has elaborated
      and thus all interfaces/channel are bound. Thus the bound
      interfaces can be accessed inside this method (e.g. to propagate
      port-attributes to the channel).
  */
  virtual void end_of_port_elaboration();

  /** Returns port-number within the parent module. */
  unsigned long get_port_number() const { return port_number; }

  void set_if_id(unsigned long id) { port_if_id = id;   }
  unsigned long get_if_id() const  { return port_if_id; }

protected:

  port_typeT port_type;

  unsigned long port_number;

  // identification of the port to the bound interface
  unsigned long port_if_id;

  //returns time of parent module
  sc_core::sc_time get_parent_module_time() const;


  //method for converter ports -> registers event on which a value from a
  //sc-signal has to be read to the synchronization layer
  void register_sc_value_trace(const ::sc_core::sc_event& ev,
  			sca_core::sca_implementation::sca_sync_value_handle_base& handle);


  //method for converter ports -> registers handle for writing values to a
   //sc-signal
   void register_sca_schedule(::sc_core::sc_time,
   			sca_core::sca_implementation::sca_sync_value_handle_base& handle);

  //reads sc value from handle at given time
  void get_sc_value_on_time(::sc_core::sc_time time,
		                    sca_sync_value_handle_base& handle);

  //writes sc value from handle at a given time
  void write_sc_value_on_time( ::sc_core::sc_time current_time,
		                       ::sc_core::sc_time next_time,
		                      sca_sync_value_handle_base& handle);

  /**
  	Method for synchronization - channel creation for in-port
  	synchronization - in the case that an sc_interface is bound
  	see @ref #elaborate_port().  For in-port synchronization the
  	method must be overloaded by port specializations.
  */
  virtual sca_core::sca_interface* create_synchronization_channel();


  sca_module* parent_module;

  //bound interfaces
  ::sc_core::sc_interface  *m_if;
  //provided interfaces
  sca_interface *m_sca_if;

  bool port_elaborated;

  //implementation of sca_traceable_object interface
 virtual bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
 virtual void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);

 virtual void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data);

};


} // namespace sca_implementation
} // namespace sca_core


#endif /* SCA_PORT_BASE_H_ */
