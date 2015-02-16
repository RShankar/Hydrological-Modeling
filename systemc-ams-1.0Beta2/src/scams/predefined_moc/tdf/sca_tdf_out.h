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

 sca_tdf_out.h - tdf outport

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_out.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.1.1.6.
 The class sca_tdf::sca_out shall define a port class for the TDF MoC.
 It provides functions for defining or getting attribute values (e.g.
 sampling rate or timestep), for initialization and for writing output samples.
 The port shall be bound to a primitive channel of class sca_tdf::sca_signal
 of the corresponding type.
 */

/*****************************************************************************/

#ifndef SCA_TDF_OUT_H_
#define SCA_TDF_OUT_H_

namespace sca_tdf {

template<class T>
//class sca_out : public implementation-derived-from sca_core::sca_port<sca_tdf::sca_signal_out_if >
class sca_out: public sca_tdf::sca_implementation::sca_tdf_port_impl<
		sca_tdf::sca_signal_if<T>, T> {
public:
	sca_out();
	explicit sca_out(const char* name_);

	void set_delay(unsigned long nsamples);
	void set_rate(unsigned long rate);
	void set_timestep(const sca_core::sca_time& tstep);
	void set_timestep(double tstep, ::sc_core::sc_time_unit unit);

	unsigned long get_delay() const;
	unsigned long get_rate() const;
	sca_core::sca_time get_time(unsigned long sample_id = 0) const;
	sca_core::sca_time get_timestep() const;

	virtual const char* kind() const;

	void initialize(const T& value, unsigned long sample_id = 0);

	void write(const T& value, unsigned long sample_id = 0);
	void write(sca_core::sca_assign_from_proxy<sca_tdf::sca_out<T> >&);

	sca_tdf::sca_out<T>& operator=(const T&);
	sca_tdf::sca_out<T>& operator=(const sca_tdf::sca_in<T>&);
	sca_tdf::sca_out<T>& operator=(const sca_tdf::sca_de::sca_in<T>&);

	sca_tdf::sca_out<T>& operator=(sca_core::sca_assign_from_proxy<
			sca_tdf::sca_out<T> >&);

	sca_core::sca_assign_to_proxy<sca_tdf::sca_out<T>,T >& operator[](unsigned long sample_id);

	void bind(sca_tdf::sca_out<T>&);
	void operator()(sca_tdf::sca_out<T>&);

	void bind(sca_tdf::sca_signal<T>&);
	void operator()(sca_tdf::sca_signal<T>&);

private:
	// Disabled
	sca_out(const sca_tdf::sca_out<T>&);

	sca_core::sca_assign_to_proxy<sca_tdf::sca_out<T>,T > proxy;

	// Disabled
	void bind(sca_tdf::sca_in<T>&);
	void operator()(sca_tdf::sca_in<T>&);

	typedef sca_tdf::sca_implementation::sca_tdf_port_impl<
			sca_tdf::sca_signal_if<T>, T> base_type;
};

//begin implementation specific

template<class T>
inline void sca_out<T>::bind(sca_tdf::sca_in<T>&)
{
	::sc_core::sc_report_handler::report(::sc_core::SC_ERROR, "SystemC-AMS",
			"Binding of sca_tdf::sca_in to "
			"sca_tdf::sca_out not permitted", __FILE__, __LINE__);
	return;
}

template<class T>
inline void sca_out<T>::operator()(sca_tdf::sca_in<T>& p) {
	sca_out<T>::bind(p);
}


template<class T>
inline void sca_out<T>::bind(sca_tdf::sca_out<T>& p)
{
	base_type::bind(p);
	return;
}

template<class T>
inline void sca_out<T>::operator()(sca_tdf::sca_out<T>& p)
{
	sca_out<T>::bind(p);
}

template<class T>
inline void sca_out<T>::bind(sca_tdf::sca_signal<T>& sig)
{
	base_type::bind(sig);
	return;
}

template<class T>
inline void sca_out<T>::operator()(sca_tdf::sca_signal<T>& sig)
{
	sca_out<T>::bind(sig);
}





template<class T>
inline sca_out<T>::sca_out() :
	sca_tdf::sca_implementation::sca_tdf_port_impl<sca_tdf::sca_signal_if<T>, T>(sc_core::sc_gen_unique_name("sca_tdf_out")) {
	base_type::port_type = base_type::SCA_OUT_PORT;
}

//////

template<class T>
inline sca_out<T>::sca_out(const char* name_) :
	sca_tdf::sca_implementation::sca_tdf_port_impl<sca_tdf::sca_signal_if<T>, T>(
			name_) {
	base_type::port_type = base_type::SCA_OUT_PORT;
}

///////

template<class T>
inline void sca_out<T>::set_delay(unsigned long nsamples) {
	base_type::set_delay(nsamples);
}

/////

template<class T>
inline void sca_out<T>::set_rate(unsigned long rate) {
	base_type::set_rate(rate);
}

//////////

template<class T>
inline void sca_out<T>::set_timestep(const sca_core::sca_time& tstep) {
	base_type::set_T(tstep);
}

//////

template<class T>
inline void sca_out<T>::set_timestep(double tstep, ::sc_core::sc_time_unit unit) {
	base_type::set_T(tstep, unit);
}

////

template<class T>
inline unsigned long sca_out<T>::get_delay() const {
	return base_type::get_delay();
}

/////

template<class T>
inline unsigned long sca_out<T>::get_rate() const {
	return base_type::get_rate();
}

//////

template<class T>
inline sca_core::sca_time sca_out<T>::get_time(unsigned long sample_id) const {
	return base_type::get_time(long(sample_id));
}

////////

template<class T>
inline sca_core::sca_time sca_out<T>::get_timestep() const {
	return base_type::get_T();
}

////////

template<class T>
inline const char* sca_out<T>::kind() const {
	return "sca_tdf::sca_out";
}

//////////

template<class T>
inline void sca_out<T>::initialize(const T& value, unsigned long sample_id) {
	base_type::initialize(value, sample_id);
}

//////

template<class T>
inline void sca_out<T>::write(const T& value, unsigned long sample_id) {
	base_type::write(value, sample_id);
}

/////

template<class T>
inline sca_tdf::sca_out<T>& sca_out<T>::operator=(const T& value) {
	base_type::write(value);
	return *this;
}

//////////////

template<class T>
inline sca_tdf::sca_out<T>& sca_out<T>::operator=(
		const sca_tdf::sca_in<T>& port) {
	base_type::write(port.read());
	return *this;
}

/////////

template<class T>
inline sca_tdf::sca_out<T>& sca_out<T>::operator=(
		const sca_tdf::sca_de::sca_in<T>& port) {
	return base_type::operator=(port);
}

////////

template<class T>
inline void sca_out<T>::write(sca_core::sca_assign_from_proxy<sca_tdf::sca_out<T> >& proxy)
{
	proxy.assign_to(*this);
}

template<class T>
inline sca_tdf::sca_out<T>& sca_out<T>::operator=(
		sca_core::sca_assign_from_proxy<sca_tdf::sca_out<T> >& proxy)
{
	write(proxy);
	return *this;
}

//////

template<class T>
inline sca_core::sca_assign_to_proxy<sca_tdf::sca_out<T>,T >& sca_out<T>::operator[](unsigned long sample_id) {
	proxy.index=sample_id;
	proxy.obj=this;
	return proxy;
}

template<class T>
inline sca_out<T>::sca_out(const sca_tdf::sca_out<T>&) {

}

//end implementation specific


} // namespace sca_tdf



#endif /* SCA_TDF_OUT_H_ */
