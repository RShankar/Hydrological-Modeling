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

 sca_tdf_sc_out.h - Converter port tdf -> SystemC

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 1186 $
 SVN last checkin  :  $Date: 2011-05-05 17:37:00 +0200 (Thu, 05 May 2011) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_sc_out.h 1186 2011-05-05 15:37:00Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.1.1.8.
 The class sca_tdf::sca_de::sca_out shall define a specialized port class
 for the TDF MoC. It provides functions for defining or getting attribute
 values (e.g. sampling rate or timestep), for initialization and for writing
 output values. The port shall be bound to a channel derived from an interface
 proper of class sc_core::sc_signal_inout_if or to a port of
 class sc_core::sc_out of the corresponding type. The port shall perform
 the synchronization between the TDF MoC and the SystemC kernel.
 A port of class sca_tdf::sca_de::sca_out can be only a member of the class
 sca_tdf::sca_module, otherwise it shall be an error.
 */

/*****************************************************************************/

#ifndef SCA_TDF_SC_OUT_H_
#define SCA_TDF_SC_OUT_H_

namespace sca_tdf
{

namespace sca_de
{

template<class T>
//class sca_out : public implementation-derived-from sca_core::sca_port&lt; sc_core::sc_signal_inout_if >
class sca_out: public sca_core::sca_port< ::sc_core::sc_signal_inout_if<T> >,
		       public sca_tdf::sca_implementation::sca_port_attributes
{
public:
	sca_out();
	explicit sca_out(const char* name_);

	void set_delay(unsigned long nsamples);
	void set_rate(unsigned long rate);
	void set_timestep(const sca_core::sca_time& tstep);
	void set_timestep(double tstep, ::sc_core::sc_time_unit unit);
	void set_timeoffset(const sca_core::sca_time& toffset);
	void set_timeoffset(double toffset, ::sc_core::sc_time_unit unit);

	unsigned long get_delay() const;
	unsigned long get_rate() const;
	sca_core::sca_time get_time(unsigned long sample_id = 0) const;
	sca_core::sca_time get_timestep() const;
	sca_core::sca_time get_timeoffset() const;

	virtual const char* kind() const;

	void initialize(const T& value, unsigned long sample_id = 0);
	void write(const T& value, unsigned long sample_id = 0);
	void write(sca_core::sca_assign_from_proxy<sca_tdf::sca_de::sca_out<T> >&);

	sca_tdf::sca_de::sca_out<T>& operator=(const T&);
	sca_tdf::sca_de::sca_out<T>& operator=(const sca_tdf::sca_in<T>&);
	sca_tdf::sca_de::sca_out<T>& operator=(const sca_tdf::sca_de::sca_in<T>&);

	sca_tdf::sca_de::sca_out<T>& operator=(sca_core::sca_assign_from_proxy<
			sca_tdf::sca_de::sca_out<T> >&);



	sca_core::sca_assign_to_proxy<sca_tdf::sca_de::sca_out<T>,T>&
	                                       operator[](unsigned long sample_id);


//begin implementation specific

	//on this way we forbid port-to-port binding to a converter port
	void bind(sc_core::sc_port<sc_core::sc_signal_inout_if<T>,1,sc_core::SC_ONE_OR_MORE_BOUND >&);
	void operator()(sc_core::sc_port<sc_core::sc_signal_inout_if<T>,1,sc_core::SC_ONE_OR_MORE_BOUND >&);

	void bind(sc_core::sc_signal_inout_if<T>&);
	void operator()(sc_core::sc_signal_inout_if<T>&);

//end implementation specific

private:
	// Disabled
	sca_out(const sca_tdf::sca_de::sca_out<T>&);

	sca_core::sca_assign_to_proxy<sca_tdf::sca_de::sca_out<T>,T> bracket_proxy;

	//on this way we forbid port-to-port binding to a converter port
	void bind(sca_tdf::sca_de::sca_out<T>&);
	void operator()(sca_tdf::sca_de::sca_out<T>&);

	//begin implementation specific

	sca_core::sca_implementation::sca_sync_value_handle<T> value_handle;
	void write_sc_signal();

	void construct();

	typedef sca_core::sca_port< ::sc_core::sc_signal_inout_if<T> > base_type;

	void end_of_port_elaboration();
	void start_of_simulation();

	const bool* allow_processing_access_flag;
	bool allow_processing_access_default;

	sca_core::sca_module* pmod;

	//end implementation specific
};

//begin implementation specific

//TODO use own method called after solver creation (tdf-view eq-setup)
template<class T>
inline void sca_out<T>::start_of_simulation()
{
	//register connection to SystemC and limit first SC time step to timeoffset
	register_sca_schedule(get_timeoffset(), value_handle);

	if(pmod)
	{
		  allow_processing_access_flag=pmod->get_allow_processing_access_flag();
		  if(allow_processing_access_flag==NULL) allow_processing_access_flag=&allow_processing_access_default;
	}

	if(this->get_timeoffset() >=
			sca_tdf::sca_implementation::sca_port_attributes::get_timestep())
	{
		std::ostringstream str;
		str << "timeoffset of port: " << this->name() << " : ";
	    str << this->get_timeoffset() << " must be smaller than timestep: ";
	    str << sca_tdf::sca_implementation::sca_port_attributes::get_timestep();
	    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
}

template<class T>
inline void sca_out<T>::construct()
{
	base_type::port_type = base_type::SCA_SC_OUT_PORT;
	value_handle.set_method(
			this,
			static_cast<sca_core::sca_implementation::sca_sync_value_handle_base::value_method> (&sca_out<
					T>::write_sc_signal));

	pmod=dynamic_cast<sca_core::sca_module*>(this->get_parent_object());

	if(pmod==NULL)
	{
		std::ostringstream str;
		str << "The sca_tdf::sca_de::sca_out port " << this->name()
			<< " must be instantiated in the context of an sca_tdf::sca_module";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	allow_processing_access_default=false;
	allow_processing_access_flag=NULL;
}

template<class T>
inline sca_out<T>::sca_out() :
	sca_out<T>::base_type(::sc_core::sc_gen_unique_name("sca_tdf_sc_out"))
{
	construct();
}

template<class T>
inline sca_out<T>::sca_out(const char* name_) :
	sca_out<T>::base_type(name_)
{
	construct();
}

template<class T>
inline void sca_out<T>::end_of_port_elaboration()
{
	base_type::m_sca_if = NULL;
}

template<class T>
inline void sca_out<T>::write_sc_signal()
{
	(*this)->write(value_handle.read());
}

template<class T>
inline void sca_out<T>::set_delay(unsigned long nsamples)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_delay of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.7.5)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(long(nsamples)<0)
	{
		std::ostringstream str;
		str << "Attribute delay is set to a very large value: " << nsamples;
		str << " may a negative number is assigned";
		str << " cannot allocate required memory";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_tdf::sca_implementation::sca_port_attributes::set_delay(nsamples);
}

template<class T>
inline void sca_out<T>::set_rate(unsigned long rate_)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_rate of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.7.6)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(rate_==0)
	{
		std::ostringstream str;
		str << "Attribute rate must be greater 0 for port: ";
		str << this->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(long(rate_)<0)
	{
		std::ostringstream str;
		str << "Attribute rate is set to a very large value: " << rate_;
		str << " for port: " << this->name();
		str << " may a negative number is assigned";
		str << " cannot allocate required memory";
	    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_tdf::sca_implementation::sca_port_attributes::set_rate(rate_);
}

template<class T>
inline void sca_out<T>::set_timestep(const sca_core::sca_time& tstep)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timestep of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.7.7)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_tdf::sca_implementation::sca_port_attributes::set_timestep(tstep);
}

template<class T>
inline void sca_out<T>::set_timestep(double tstep, ::sc_core::sc_time_unit unit)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timestep of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.7.7)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_out<T>::set_timestep(sca_core::sca_time(tstep, unit));
}

template<class T>
inline void sca_out<T>::set_timeoffset(const sca_core::sca_time& toffset)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timeoffset of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.7.8)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_tdf::sca_implementation::sca_port_attributes::set_timeoffset(toffset);
}

template<class T>
inline void sca_out<T>::set_timeoffset(double toffset,
		::sc_core::sc_time_unit unit)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timeoffset of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.6.8, 4.1.1.7.8)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sca_out<T>::set_timeoffset(sca_core::sca_time(toffset, unit));
}

template<class T>
inline unsigned long sca_out<T>::get_delay() const
{
	return sca_tdf::sca_implementation::sca_port_attributes::get_delay();
}

template<class T>
inline unsigned long sca_out<T>::get_rate() const
{
	return sca_tdf::sca_implementation::sca_port_attributes::get_rate();
}

template<class T>
inline sca_core::sca_time sca_out<T>::get_time(unsigned long sample_id) const
{
	//allowed in initialize and processing only
	if((allow_processing_access_flag==NULL) || !(*allow_processing_access_flag))
	{
		if(!pmod || !(pmod->get_allow_initialize_access_flag()))
		{
			std::ostringstream str;
		    str << "get_time for port: " << this->name()
			    << " can only be accessed from the context of the initialize or"
		        << " processing callback";
		    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

	}

	return this->get_parent_module_time() + sample_id
			* sca_out<T>::get_timestep() + sca_out<T>::get_timeoffset();
}

template<class T>
inline sca_core::sca_time sca_out<T>::get_timestep() const
{
	//allowed in initialize and processing only
	if((allow_processing_access_flag==NULL) || !(*allow_processing_access_flag))
	{
		if(!pmod || !(pmod->get_allow_initialize_access_flag()))
		{
			std::ostringstream str;
		    str << "get_timestep for port: " << this->name()
			    << " can only be accessed from the context of the initialize or"
		        << " processing callback";
		    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

	}

	return sca_tdf::sca_implementation::sca_port_attributes::get_timestep();
}

template<class T>
inline sca_core::sca_time sca_out<T>::get_timeoffset() const
{
	return sca_tdf::sca_implementation::sca_port_attributes::get_timeoffset();
}

template<class T>
inline const char* sca_out<T>::kind() const
{
	return "sca_tdf::sca_de::sca_out";
}

template<class T>
inline void sca_out<T>::initialize(const T& value, unsigned long sample_id)
{
	if(!pmod || !(pmod->get_allow_initialize_access_flag()))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be initialized due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.5.13, 4.1.1.5.13, 4.1.1.6.15, 4.1.1.7.15)";
	    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if (sample_id >= sca_out<T>::get_delay())
	{
		std::ostringstream str;
		str << "Sample id (" << sample_id << ") is greater than delay ("
				<< sca_out<T>::get_delay() << " while initializing port: "
				<< sca_out<T>::name();
		::sc_core::sc_report_handler::report(::sc_core::SC_ERROR,
				"SystemC-AMS", str.str().c_str(), __FILE__, __LINE__);
	}


	::sc_core::sc_time sched_time;
	sched_time = sample_id * get_timestep() + get_timeoffset();

	value_handle.write_tmp(value);
	write_sc_value_on_time(sched_time, sched_time + get_timestep(), value_handle);

}

template<class T>
inline void sca_out<T>::write(const T& value, unsigned long sample_id)
{

	if((allow_processing_access_flag==NULL) || !(*allow_processing_access_flag))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be written due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.7.16)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}


	if (sample_id >= sca_out<T>::get_rate())
	{
		std::ostringstream str;
		str << "Sample id (" << sample_id
				<< ") is greater or equal than rate (" << sca_out<T>::get_rate()
				<< " while writing to port: " << sca_out<T>::name();

		::sc_core::sc_report_handler::report(::sc_core::SC_ERROR,
				"SystemC-AMS", str.str().c_str(), __FILE__, __LINE__);
	}

	::sc_core::sc_time sched_time;
	sched_time = get_time(sample_id) + get_delay() * get_timestep();

	value_handle.write_tmp(value);
	write_sc_value_on_time(sched_time, sched_time + get_timestep(), value_handle);

}

template<class T>
inline sca_tdf::sca_de::sca_out<T>& sca_out<T>::operator=(const T& value)
{
	write(value);
	return *this;
}


template<class T>
inline sca_tdf::sca_de::sca_out<T>& sca_out<T>::operator=(const sca_tdf::sca_in<T>& iport)
{
	write(iport.read());
	return *this;
}


template<class T>
inline sca_tdf::sca_de::sca_out<T>& sca_out<T>::operator=(const sca_tdf::sca_de::sca_in<T>& iport)
{
	write(iport.read());
	return *this;
}


template<class T>
inline void sca_out<T>::write
		(sca_core::sca_assign_from_proxy<sca_tdf::sca_de::sca_out<T> >& proxy)
{
	if(!(*allow_processing_access_flag))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be written due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.7.16)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	proxy.assign_to(*this);
}



template<class T>
inline sca_tdf::sca_de::sca_out<T>& sca_out<T>::operator=(sca_core::sca_assign_from_proxy<
		sca_tdf::sca_de::sca_out<T> >& proxy)
{
	write(proxy);
	return *this;
}



template<class T>
inline void sca_out<T>::bind(sc_core::sc_port<sc_core::sc_signal_inout_if<T>,1,sc_core::SC_ONE_OR_MORE_BOUND >& oport)
{
	base_type::bind(oport);
}

template<class T>
void sca_out<T>::operator()(sc_core::sc_port<sc_core::sc_signal_inout_if<T>,1,sc_core::SC_ONE_OR_MORE_BOUND >& oport)
{
	bind(oport);
}




template<class T>
inline void sca_out<T>::bind(sc_core::sc_signal_inout_if<T>& intf)
{
	base_type::bind(intf);
}

template<class T>
inline void sca_out<T>::operator()(sc_core::sc_signal_inout_if<T>& intf)
{
	bind(intf);
}

//methods private and thus disabled - should not possible to call
template<class T>
inline void sca_out<T>::bind(sca_tdf::sca_de::sca_out<T>&)
{
	SC_REPORT_ERROR("SystemC-AMS","Binding of converter port to converter port not permitted");
}

template<class T>
inline void sca_out<T>::operator()(sca_tdf::sca_de::sca_out<T>& port)
{
	bind(port);
}

template<class T>
inline sca_core::sca_assign_to_proxy<sca_tdf::sca_de::sca_out<T>,T>&
	                            sca_out<T>::operator[](unsigned long sample_id)
{
  bracket_proxy.index=sample_id;
  bracket_proxy.obj=this;

  return bracket_proxy;
}


} // namespace sca_de

//template<class T>
//typedef sca_tdf::sca_de::sca_out<T> sc_out<T> ;

template<class T>
class sc_out: public sca_tdf::sca_de::sca_out<T>
{
public:
	sc_out() :
		sca_tdf::sca_de::sca_out<T>()
	{
	}
	explicit sc_out(const char* nm) :
		sca_tdf::sca_de::sca_out<T>(nm)
	{
	}
};

} // namespace sca_tdf

#endif /* SCA_TDF_SC_OUT_H_ */
