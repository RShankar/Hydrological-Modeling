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

 sca_tdf_trace_variable.h - variable for tracing internal tdf states

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_trace_variable.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.1.1.9.
 The class sca_tdf::sca_trace_variable shall implement a variable, which can
 be traced in a trace file of class sca_util::sca_trace_file.
 */

/*****************************************************************************/

#ifndef SCA_TDF_TRACE_VARIABLE_H_
#define SCA_TDF_TRACE_VARIABLE_H_

namespace sca_tdf
{

//template<class T>
//class sca_trace_variable: protected sca_util::sca_traceable_object,
//public ::sc_core::sc_object
template<class T>
class sca_trace_variable: public sca_tdf::sca_implementation::sca_trace_variable_base
{
public:
	sca_trace_variable();
	explicit sca_trace_variable(const char* name_);

	virtual const char* kind() const;

	void set_rate(unsigned long rate);
	void set_timeoffset(const sca_core::sca_time& toffset);
	void set_timeoffset(double toffset, ::sc_core::sc_time_unit unit);

	void write(const T& value, unsigned long sample_id = 0);
	sca_tdf::sca_trace_variable<T>& operator=(const T& value);
	sca_tdf::sca_trace_variable<T>& operator=(const sca_tdf::sca_in<T>& port);
	sca_tdf::sca_trace_variable<T>& operator=(
			const sca_tdf::sca_de::sca_in<T>& port);

	sca_core::sca_assign_to_proxy<sca_tdf::sca_trace_variable<T>,T>&
	                                      operator[](unsigned long sample_id);


//Begin implementation specific

private:

	std::vector<T> buffer;
	T last_value;

    void resize_buffer(long n);
    void store_to_last(long n);
    void trace_last(long id,sca_core::sca_time& ct,sca_util::sca_implementation::sca_trace_buffer& tr_buffer);
    void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data);

	sca_core::sca_assign_to_proxy<sca_tdf::sca_trace_variable<T>,T> bracket_proxy;

//End implementation specific

};

//Begin implementation specific

template<class T>
inline void sca_trace_variable<T>::set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.set_type_info<T>();
}


template<class T>
inline void sca_trace_variable<T>::resize_buffer(long n)
{
	buffer.resize(n);
	buffer_size=n;
}


template<class T>
inline void sca_trace_variable<T>::store_to_last(long n)
{
	last_value = buffer[n];
}


template<class T>
inline void sca_trace_variable<T>::trace_last(
		long id,sca_core::sca_time& ct,sca_util::sca_implementation::sca_trace_buffer& tr_buffer)
{
	tr_buffer.store_time_stamp(id, ct, last_value);
}

template<class T>
inline sca_trace_variable<T>::sca_trace_variable():
	sca_tdf::sca_implementation::sca_trace_variable_base(
			sc_core::sc_gen_unique_name("sca_tdf_trace_variable"))
{
}

template<class T>
inline sca_trace_variable<T>::sca_trace_variable(const char* name_):
	sca_tdf::sca_implementation::sca_trace_variable_base(name_)
{
}

template<class T>
inline const char* sca_trace_variable<T>::kind() const
{
	return "sca_tdf::sca_trace_variable";
}

template<class T>
inline void sca_trace_variable<T>::set_rate(unsigned long rate_)
{
	rate=rate_;
}

template<class T>
inline void sca_trace_variable<T>::set_timeoffset(const sca_core::sca_time& toffset)
{
	timeoffset=toffset;
}

template<class T>
inline void sca_trace_variable<T>::set_timeoffset(double toffset, ::sc_core::sc_time_unit unit)
{
	timeoffset=sca_core::sca_time(toffset,unit);
}



template<class T>
inline sca_core::sca_assign_to_proxy<sca_tdf::sca_trace_variable<T>,T>&
                     sca_trace_variable<T>::operator[](unsigned long sample_id)
{
	  bracket_proxy.index=sample_id;
	  bracket_proxy.obj=this;

	  return bracket_proxy;
}


template<class T>
inline void sca_trace_variable<T>::write(const T& value, unsigned long sample_id)
{
	if (!initialized)
		if (!initialize()) return;

	if (sample_id>=rate)
	{
		std::ostringstream str;
		str << "sample_id must be smaller than rate in sca_tdf::sca_trace_variable: "
		    << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}


	long index=((*call_counter_ref)%calls_per_period)*rate + sample_id;

	write_flags[index] = true;
	buffer[index]=value;
}


template<class T>
inline sca_tdf::sca_trace_variable<T>& sca_trace_variable<T>::operator=(const T& value)
{
	write(value);
	return *this;
}

template<class T>
inline sca_tdf::sca_trace_variable<T>& sca_trace_variable<T>::operator=(const sca_tdf::sca_in<T>& port)
{
	write(port.read());
	return *this;
}


template<class T>
inline sca_tdf::sca_trace_variable<T>& sca_trace_variable<T>::operator=(
		const sca_tdf::sca_de::sca_in<T>& port)
{
	write(const_cast<sca_tdf::sca_de::sca_in<T>*>(&port)->read());
	return *this;
}

//End implementation specific


} // namespace sca_tdf

#endif /* SCA_TDF_TRACE_VARIABLE_H_ */
