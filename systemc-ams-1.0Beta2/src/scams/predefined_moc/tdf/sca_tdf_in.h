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

 sca_tdf_in.h - tdf inport

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_in.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.1.1.5.
 The class sca_tdf::sca_in shall define a port class for the TDF MoC.
 It provides functions for defining or getting attribute values (e.g.
 sampling rate or time step), for initialization and for reading input
 samples. The port shall be bound to a primitive channel of class
 sca_tdf::sca_signal of the corresponding type.

 */

/*****************************************************************************/

#ifndef SCA_TDF_IN_H_
#define SCA_TDF_IN_H_

namespace sca_tdf
{

template<class T>
//class sca_in : public implementation-derived-from sca_core::sca_port<sca_tdf::sca_signal_in_if >
class sca_in: public sca_tdf::sca_implementation::sca_tdf_port_impl<
		sca_tdf::sca_signal_if<T>, T>
{
public:
	sca_in();
	explicit sca_in(const char* name_);

	void set_delay(unsigned long nsamples);
	void set_rate(unsigned long rate);
	void set_timestep(const sca_core::sca_time& tstep);
	void set_timestep(double tstep, sc_core::sc_time_unit unit);

	unsigned long get_delay() const;
	unsigned long get_rate() const;
	sca_core::sca_time get_time(unsigned long sample_id = 0) const;
	sca_core::sca_time get_timestep() const;

	virtual const char* kind() const;

	void initialize(const T& value, unsigned long sample_id = 0);
	const T& read(unsigned long sample_id = 0) const;
	operator const T&() const;
	const T& operator[](unsigned long sample_id) const;

private:
	// Disabled
	sca_in(const sca_tdf::sca_in<T>&);

	typedef sca_tdf::sca_implementation::sca_tdf_port_impl<
	sca_tdf::sca_signal_if<T>, T> base_type;
};


/////////////////////////////////////////////
/////////////////////////////////////////////

template<class T>
inline sca_in<T>::sca_in() :
			sca_tdf::sca_implementation::sca_tdf_port_impl<
					sca_tdf::sca_signal_if<T>, T>(sc_core::sc_gen_unique_name("sca_tdf_in"))
{
	 base_type::port_type = base_type::SCA_IN_PORT;
}

///

template<class T>
inline sca_in<T>::sca_in(const char* name_) :
			sca_tdf::sca_implementation::sca_tdf_port_impl<
					sca_tdf::sca_signal_if<T>, T>(name_)
{
	 base_type::port_type = base_type::SCA_IN_PORT;
}

////

template<class T>
inline void sca_in<T>::set_delay(unsigned long nsamples)
{
	base_type::set_delay((long)nsamples);
}

////

template<class T>
inline void sca_in<T>::set_rate(unsigned long rate)
{
	base_type::set_rate(long(rate));
}

////

template<class T>
inline void sca_in<T>::set_timestep(const sca_core::sca_time& tstep)
{
	base_type::set_T(tstep);
}

////

template<class T>
inline void sca_in<T>::set_timestep(double tstep, sc_core::sc_time_unit unit)
{
	base_type::set_T(tstep,unit);
}

/////

template<class T>
inline unsigned long sca_in<T>::get_delay() const
{
	return base_type::get_delay();
}

/////

template<class T>
inline unsigned long sca_in<T>::get_rate() const
{
	return base_type::get_rate();
}


///////

template<class T>
inline sca_core::sca_time sca_in<T>::get_time(unsigned long sample_id) const
{
	return base_type::get_time(long(sample_id));
}


/////////

template<class T>
inline sca_core::sca_time sca_in<T>::get_timestep() const
{
	return base_type::get_T();
}

//////

template<class T>
inline const char* sca_in<T>::kind() const
{
	return "sca_tdf::sca_in";
}

/////

template<class T>
inline void sca_in<T>::initialize(const T& value, unsigned long sample_id)
{
   base_type::initialize(value,sample_id);
}


/////

template<class T>
inline const T& sca_in<T>::read(unsigned long sample_id) const
{
	return base_type::read(long(sample_id));
}

////

template<class T>
inline sca_in<T>::operator const T&() const
{
	return base_type::read();
}
template<class T>
inline const T& sca_in<T>::operator[](unsigned long sample_id) const
{
	return base_type::read(long(sample_id));
}


} // namespace sca_tdf


#endif /* SCA_TDF_IN_H_ */
