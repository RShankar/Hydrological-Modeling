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

  sca_tdf_port_impl.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.08.2009

   SVN Version       :  $Revision: 1177 $
   SVN last checkin  :  $Date: 2011-05-01 23:35:57 +0200 (Sun, 01 May 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_port_impl.h 1177 2011-05-01 21:35:57Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_TDF_PORT_IMPL_H_
#define SCA_TDF_PORT_IMPL_H_

namespace sca_ac_analysis
{
bool sca_ac_is_running();
}

namespace sca_core
{
namespace sca_implementation
{
	const sca_core::sca_time& NOT_VALID_SCA_TIME();
}
}


namespace sca_tdf
{

namespace sca_implementation
{

/**
   Port base class for static dataflow modeling
   provides interfaces for port read/write,
   the datflow scheduling configuration (sample rates,
   sample time, time offset) and  getting currently  setted
   rates and times
*/
template<class SC_IF, class T>
class sca_tdf_port_impl : public ::sca_core::sca_port<SC_IF>
{

 protected:

  typedef ::sca_core::sca_port<SC_IF> base_type;

  //declaration due permissive problem (no arguments which are depending on
  //a template parameter)
  unsigned long get_if_id() const { return base_type::get_if_id(); }

  sca_tdf_port_impl();
  explicit sca_tdf_port_impl(const char* name_);

  /** Port read for first sample */
  const T& read() const;

  /** Port read for an arbitrary sample (sample must be < rate) */
  const T& read(unsigned long sample) const;

  /** Port write to the first sample */
  void write(T value);


  /** Port write for an an arbitrary sample (sample must be < rate) */
  void write(T value,unsigned long sample);

  /** Port write in the initialize phase */
  void initialize(const T& value, unsigned long sample_id);

  /** gets reference to sample sample (for implementing [] operator for
   * tdf out
   */
  T& get_ref_for_write(unsigned long sample) const;

  /** Sets sample rate of the port */
  void set_rate(long rate);

  /** Sets number of sample delay of the port */
  void set_delay(long samples);

  /** Sets time distance between two samples */
  void set_T(sca_core::sca_time t_period) ;
  void set_T(double dtime, sc_core::sc_time_unit unit) ;


  /** sets absolute time of first sample (must be < than T */
  void set_t0(sca_core::sca_time _t0);
  void set_t0(double dtime, sc_core::sc_time_unit unit);

  /** Gets the absolute number of samples (from simulation start) samples
      of the current call are not included */
  unsigned long get_sample_cnt() const { return sig->get_sample_cnt(get_if_id()); }
  /** Gets number of sample delay of the port */
  unsigned long get_delay() const
  {
	  if(sig==NULL) return delay;
	  return sig->get_delay(get_if_id());
  }
  /** Gets the setted sampling rate of the port */
  unsigned long get_rate() const
  {
	  if(sig==NULL) return rate;
	  return sig->get_rate(get_if_id());
  }
  /** Gets the calculated sample time distance -> if the time was setted it must
      be equal to the calculated one - if not the scheduler will throw an
      exception */
  sca_core::sca_time get_T() const
  {
	  if(sig==NULL)
	  {
		  std::ostringstream str;
		  str << "can't get_timestep before elaboration has been finished ";
		  str << "for port: " << this->name();
		  SC_REPORT_ERROR("SytemC-AMS",str.str().c_str());
		  return sca_core::sca_implementation::NOT_VALID_SCA_TIME();
	  }

	  sca_core::sca_time rett=sig->get_T(get_if_id());

	  if(rett==sca_core::sca_implementation::NOT_VALID_SCA_TIME())
	  {
		  std::ostringstream str;
		  str << "can't get_timestep before elaboration has been finished ";
		  str << "for port: " << this->name();
		  SC_REPORT_ERROR("SytemC-AMS",str.str().c_str());
	  }

	  return rett;
  }
  /** Gets the calculated time offset (time of the first sample) -> the setted one
      will be equal to the calculated one -> if not possible (causality) the
      scheduler will throw an exception */
  sca_core::sca_time get_t0() const
  {
	  if(sig==NULL) return t0;
	  return sig->get_t0(get_if_id());
  }
  /** Gets the absolute time (from simulation start) of the first sample of the
      current call */
  sca_core::sca_time get_time(unsigned long sample = 0) const
  {
	  sca_core::sca_time rett;
	  if(sig==NULL) rett=sca_core::sca_implementation::NOT_VALID_SCA_TIME();
	  else          rett=sig->get_time(get_if_id(),sample);

	  if(rett==sca_core::sca_implementation::NOT_VALID_SCA_TIME())
	  {
		  std::ostringstream str;
		  str << "can't get_time of port before elaboration has been finished ";
		  str << "for port: " << this->name();
		  SC_REPORT_ERROR("SytemC-AMS",str.str().c_str());
	  }

	  return rett;
  }

  virtual const char* kind() const  { return "sca_tdf_port_impl"; }


 protected:

  void end_of_port_elaboration();
  void start_of_simulation();


 private:

  unsigned long rate;
  unsigned long delay;
  sca_core::sca_time Tp;
  sca_core::sca_time t0;

  bool if_bound;

  const bool* allow_processing_access_flag;
  bool allow_processing_access_default;

  void construct();

  sca_tdf::sca_implementation::sca_tdf_signal_impl<T>* sig;

  void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data);

  sca_core::sca_module* pmod; //parent module

};

//////////////////////////////////////////////////////


template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::
	set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.set_type_info<T>();
}



template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_rate(long rate_)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_rate of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.4.6, 4.1.1.5.6)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(rate_==0)
	{
		std::ostringstream str;
		str << "Attribute rate must be greater 0 for port: ";
		str <<  sc_core::sc_port_base::get_parent()->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

  if(rate_<0)
  {
    std::ostringstream str;
    str << "TDF-Port #" << sca_core::sca_implementation::sca_port_base::get_port_number()
        << " of module: " << sc_core::sc_port_base::get_parent()->name() << std::endl
        << "a very large value is set by sca_sdf_port::set_rate() may a negative value is assigned."
        << " cannot allocate required memory"
        << std::endl;
    SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
  }

  this->rate = rate_;
  if(if_bound)   sig->set_rate(get_if_id(),rate_);
}

template<class SC_IF, class T>
inline sca_tdf_port_impl<SC_IF,T>::sca_tdf_port_impl() : base_type()
{
  construct();

#ifdef SCA_IMPLEMENTATION_DEBUG
  std::cout << "port " << sc_object::basename() << " initialized" << std::endl;
#endif
}

template<class SC_IF, class T>
inline sca_tdf_port_impl<SC_IF,T>::sca_tdf_port_impl(const char* name_) : base_type(name_)
{
  construct();

#ifdef SCA_IMPLEMENTATION_DEBUG
  std::cout << "port " << name_ << " initialized" << std::endl;
#endif
}

template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::construct()
{
  rate   = 1;
  delay = 0;
  if_bound = false;
  sig=NULL;

  Tp = sca_core::sca_implementation::NOT_VALID_SCA_TIME();
  t0 = sca_core::sca_implementation::NOT_VALID_SCA_TIME();

  allow_processing_access_default=false;
  allow_processing_access_flag=&allow_processing_access_default;

  pmod=dynamic_cast<sca_core::sca_module*>(this->get_parent_object());
}

template<class SC_IF, class T>
inline const T& sca_tdf_port_impl<SC_IF,T>::read() const
{
	if(!(*allow_processing_access_flag))
	{
		if(!sca_ac_analysis::sca_ac_is_running())
		{
			std::ostringstream str;
			str << "Port: " << this->name() << " can't be read due it is not accessed "
		        << " from the context of the processing method of the parent sca_tdf::sca_module"
		        << " (see LRM clause 4.1.1.4.16)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

  return sig->read(get_if_id());
}

template<class SC_IF, class T>
inline const T& sca_tdf_port_impl<SC_IF,T>::read(unsigned long sample) const
{
	if(!(*allow_processing_access_flag))
	{
		if(!sca_ac_analysis::sca_ac_is_running())
		{
			std::ostringstream str;
			str << "Port: " << this->name() << " can't be read due it is not accessed "
			    << " from the context of the processing method of the parent sca_tdf::sca_module"
			    << " (see LRM clause 4.1.1.4.16)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	return sig->read(get_if_id(),sample);

}

template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::write(T value)
{
	if(!(*allow_processing_access_flag))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be written due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.5.16)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sig->write(get_if_id(),value);

}

template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::write(T value,unsigned long sample)
{
	if(!(*allow_processing_access_flag))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be written due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.5.16)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sig->write(get_if_id(),value,sample);

}


template<class SC_IF, class T>
void sca_tdf_port_impl<SC_IF,T>::initialize(const T& value, unsigned long sample_id)
{
	if(pmod && !(pmod->get_allow_initialize_access_flag()))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be initialized due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.5.13, 4.1.1.5.13)";
	    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	sig->write(get_if_id(),value,sample_id);
}

template<class SC_IF, class T>
inline T& sca_tdf_port_impl<SC_IF,T>::get_ref_for_write(unsigned long sample) const
{
	if(!(*allow_processing_access_flag))
	{
		std::ostringstream str;
	    str << "Port: " << this->name() << " can't be written due it is not accessed "
		    << " from the context of the processing method of the parent sca_tdf::sca_module"
		    << " (see LRM clause 4.1.1.5.16)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	return sig->get_ref_for_write(get_if_id(),sample);

}


template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_delay(long samples)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_delay of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.4.5, 4.1.1.5.5)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}


  if(samples<0)
  {
    std::ostringstream str;
    str << "TDF-Port #" << sca_core::sca_implementation::sca_port_base::get_port_number()
        << " of module: " << sc_core::sc_port_base::get_parent()->name() << std::endl
        << "Delays set by sca_sdf_port::set_delay() is a very large value"
        << " may be a negative value was assigned"
        << " cannot allocate required memory";
    SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
  }

  delay = samples;
  if(if_bound) sig->set_delay(get_if_id(),delay);
}

template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_T(sca_core::sca_time t_period)
{

	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timestep of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.4.7, 4.1.1.5.7)";
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

  Tp = t_period;
  if(if_bound) sig->set_T(get_if_id(),Tp);
}


template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_T(double dtime, sc_core::sc_time_unit unit)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timestep of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.4.7, 4.1.1.5.7)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	set_T(sca_core::sca_time(dtime,unit));
}


template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_t0(sca_core::sca_time _t0)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timeoffset of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.6.8, 4.1.1.7.8)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

  t0 = _t0;
  if(if_bound) sig->set_t0(get_if_id(),t0);
}

template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::set_t0(double dtime, sc_core::sc_time_unit unit)
{
	if(pmod && !(pmod->get_allow_attributes_access_flag()))
	{
		std::ostringstream str;
		str << "Method set_timeoffset of port: " << this->name()
			<< " can only be called in the context of set_attributes of the parent module"
			<< " (see LRM clause 4.1.1.6.8, 4.1.1.7.8)";;
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

  set_t0(sca_core::sca_time(dtime,unit));
}





template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::end_of_port_elaboration()
{
#ifdef SCA_IMPLEMENTATION_DEBUG
  std::cout << "end of port elaboration" << std::endl;
#endif

  sig=dynamic_cast<sca_tdf::sca_implementation::sca_tdf_signal_impl<T>*>(this->sca_get_interface());

  sig->set_rate(get_if_id(),rate);
  sig->set_delay(get_if_id(),delay);

  sig->set_T(get_if_id(),Tp);
  sig->set_t0(get_if_id(),t0);

  if_bound = true;
}


template<class SC_IF, class T>
inline void sca_tdf_port_impl<SC_IF,T>::start_of_simulation()
{
	  if(pmod)
	  {
		  allow_processing_access_flag=pmod->get_allow_processing_access_flag();
		  if(allow_processing_access_flag==NULL) allow_processing_access_flag=&allow_processing_access_default;
	  }

}

} //namespace sca_implementation
} //namespace sca_tdf


#endif /* SCA_TDF_PORT_IMPL_H_ */
