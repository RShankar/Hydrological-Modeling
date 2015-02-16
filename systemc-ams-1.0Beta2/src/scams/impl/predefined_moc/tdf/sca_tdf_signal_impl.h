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

 sca_tdf_signal_impl.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 06.08.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_signal_impl.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_TDF_SIGNAL_IMPL_H_
#define SCA_TDF_SIGNAL_IMPL_H_


namespace sca_tdf
{

namespace sca_implementation
{

/**
 Static dataflow signal. This channel can be used
 to connect SCA_SDF_MODULE's via sca_sdf_in/out ports
 (@ref #class sca_sdf_port<T>). The channel implements
 the @ref #class sca_sdf_interface<T> for communication
 and synchronization.
 */
template<class T>
class sca_tdf_signal_impl: public sca_tdf::sca_implementation::sca_tdf_signal_impl_base
{
public:

	/** Port read for first sample */
	const T& read(unsigned long port) const;

	/** Port read for an arbitrary sample (sample must be < rate) */
	const T& read(unsigned long port, unsigned long sample) const;

	/** Port write to the first sample */
	void write(unsigned long port, T value);

	/** Port write for an an arbitrary sample (sample must be < rate) */
	void write(unsigned long port, T value, unsigned long sample);

	/** returns reference to a buffer element to write - for
	 * implementing the [] operator of an outport (sample must be < rate) */
	T& get_ref_for_write(unsigned long port, unsigned long sample) const;

	const char* kind() const;

	sca_tdf_signal_impl();
	sca_tdf_signal_impl(const char* name_);

	virtual ~sca_tdf_signal_impl();

private:

	T* buffer;
	std::vector<T*> in_delay_buffer;

	void create_buffer(unsigned long size);
    void store_trace_buffer_time_stamp(sca_util::sca_implementation::sca_trace_buffer& trace_buffer,unsigned long id,sca_core::sca_time ctime, unsigned long bpos);

	void in_delay_buffer_resize(unsigned long n);
	void in_delay_buffer_create(unsigned long b_nr, unsigned long size);

	void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data);


};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


template<class T>
void sca_tdf_signal_impl<T>::set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.set_type_info<T>();
}


template<class T>
inline void sca_tdf_signal_impl<T>::create_buffer(unsigned long size)
{
	buffer = new T[size];
	for (unsigned long i = 0; i < size; ++i)
		buffer[i] = T();
}


template<class T>
inline void sca_tdf_signal_impl<T>::store_trace_buffer_time_stamp(
		sca_util::sca_implementation::sca_trace_buffer& trace_buffer,unsigned long id,sca_core::sca_time ctime, unsigned long bpos)
{
	trace_buffer.store_time_stamp(id,ctime,buffer[bpos]);
}



template<class T>
inline void sca_tdf_signal_impl<T>::in_delay_buffer_resize(unsigned long n)
{
	in_delay_buffer.resize(n, NULL);
}

template<class T>
inline void sca_tdf_signal_impl<T>::in_delay_buffer_create(unsigned long b_nr,
		unsigned long size)
{
	in_delay_buffer[b_nr] = new T[size];
}

/** Port read for first sample */
template<class T>
inline const T& sca_tdf_signal_impl<T>::read(unsigned long port) const
{
	sc_dt::int64 read_pos = (sc_dt::int64) (*(call_counter_refs[port]) * rates[port]
			+ buffer_offsets[port]);

	if (read_pos < 0)
	{
		return (in_delay_buffer[port])[read_pos - buffer_offsets[port]];
	}
	else
	{
		//				cout << endl << "Read from pos.: " << (unsigned long long)(*(call_counter_refs[port]) *
		//			                  rates[port] + buffer_offsets[port]+sample) % buffer_size
		//			                  << " Value: " << buffer[(unsigned long long)(*(call_counter_refs[port]) *
		//			                  rates[port] + buffer_offsets[port]+sample) % buffer_size]
		//			                  << "  rate: " << rates[port] << " off: " << buffer_offsets[port] << endl;

		return (buffer[((sc_dt::uint64) read_pos) % buffer_size]);

	}
}

/** Port read for an arbitrary sample (sample must be < rate) */
template<class T>
inline const T& sca_tdf_signal_impl<T>::read(unsigned long port,
		unsigned long sample) const
{
	if (sample >= rates[port])
	{
		std::ostringstream str;
		str << "Access to sample >= rate not allowed for port: "
				<< get_connected_port_list()[port]->sca_name() << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	sc_dt::int64 read_pos = (sc_dt::int64) (*(call_counter_refs[port]) * rates[port]
			+ buffer_offsets[port] + sample);

	if (read_pos < 0)
	{
		//if we have an access before the first activation (e.g. for AC simulation)
		//or if Joerg is doing strange things
		if((read_pos - buffer_offsets[port])<0) return buffer[0];
		return (in_delay_buffer[port])[read_pos - buffer_offsets[port]];
	}
	else
	{
		//				cout << endl << "Read from pos.: " << (unsigned long long)(*(call_counter_refs[port]) *
		//			                  rates[port] + buffer_offsets[port]+sample) % buffer_size
		//			                  << " Value: " << buffer[(unsigned long long)(*(call_counter_refs[port]) *
		//			                  rates[port] + buffer_offsets[port]+sample) % buffer_size]
		//			                  << "  rate: " << rates[port] << " off: " << buffer_offsets[port] << " sam: " << sample<< endl;

		return (buffer[((sc_dt::uint64) read_pos) % buffer_size]);

	}

}

/** Port write to the first sample */
template<class T>
inline void sca_tdf_signal_impl<T>::write(unsigned long port, T value)
{
	//		cout << "Write at pos.: " <<  (unsigned long long) ( *(call_counter_refs[port]) * rates[port]
	//		                + buffer_offsets[port] ) % buffer_size << " value: " << value << endl;

	sc_dt::int64 call_cnt = *(call_counter_refs[port]);
	if (call_cnt < 0)
	{
		if (delays[port] < 1)
		{
			std::ostringstream str;
			str
					<< " Port must have a delay if it initialized (written) during init: ";
			str << "for port: " << get_connected_port_list()[port]->sca_name();
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}
		if (in_delay_buffer[port] == NULL)
			buffer[0] = value;
		else
			(in_delay_buffer[port])[0] = value;
		return;
	}

	if (is_inport[port])
	{
		std::ostringstream str;
		str << std::endl << "Write access is not allowed to inport: "
				<< get_connected_port_list()[port]->sca_name() << std::endl << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	buffer[(sc_dt::uint64) (call_cnt * rates[port] + buffer_offsets[port])
			% buffer_size] = value;
}



/** returns reference to a buffer element to write - for
 * implementing the [] operator of an outport (sample must be < rate) */
template<class T>
inline T& sca_tdf_signal_impl<T>::get_ref_for_write(unsigned long port,
		unsigned long sample) const
{
	sc_dt::int64 call_cnt = *(call_counter_refs[port]);
	if (call_cnt < 0)
	{
		if (sample > delays[port])
		{
			std::ostringstream str;
			str << " Sample number(" << sample << ")"
					<< " for port initialize must be smaller "
					<< "than the specified delay" << " value (" << delays[port]
					<< ") for port: ";
			str << get_connected_port_list()[port]->sca_name();
			str << std::endl << std::endl;
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}

		if (in_delay_buffer[port] == NULL)
		{
			return buffer[sample];
		}
		else
		{
			return (in_delay_buffer[port])[sample];
		}
	}

	if (is_inport[port])
	{
		std::ostringstream str;
		str << std::endl << "Write access is not allowed to inport: "
				<< get_connected_port_list()[port]->sca_name() << std::endl << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	if (sample >= rates[port])
	{
		std::ostringstream str;
		str << "access to sample >= rate not allowed for port: "
				<< get_connected_port_list()[port]->sca_name() << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}


	return buffer[(sc_dt::uint64) (call_cnt * rates[port] + buffer_offsets[port]
			+ sample) % buffer_size];
}




/** Port write for an an arbitrary sample (sample must be < rate) */
template<class T>
inline void sca_tdf_signal_impl<T>::write(unsigned long port, T value,
		unsigned long sample)
{
	get_ref_for_write(port,sample)=value;

}


template<class T>
inline const char* sca_tdf_signal_impl<T>::kind() const
{
	return "sca_sdf_signal";
}

template<class T>
inline sca_tdf_signal_impl<T>::sca_tdf_signal_impl()
{
	buffer = NULL;
}


template<class T>
inline sca_tdf_signal_impl<T>::sca_tdf_signal_impl(const char* name_):
	sca_tdf::sca_implementation::sca_tdf_signal_impl_base(name_)
{
	buffer = NULL;
}


template<class T>
inline sca_tdf_signal_impl<T>::~sca_tdf_signal_impl()
{

	delete[] buffer;

	for (typename std::vector<T*>::iterator it = in_delay_buffer.begin(); it
			!= in_delay_buffer.end(); ++it)
	{
		delete[] *it;
	}
}

} //namespace sca_implentation
} //namespace sca_tdf


#endif /* SCA_TDF_SIGNAL_IMPL_H_ */
