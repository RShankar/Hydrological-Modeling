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

 sca_tdf_signal_impl_base.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 06.08.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_signal_impl_base.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_TDF_SIGNAL_IMPL_BASE_H_
#define SCA_TDF_SIGNAL_IMPL_BASE_H_


namespace sca_tdf
{

namespace sca_implementation
{

/**
 Non template functionality for sca_tdf::sca_signal
 */
class sca_tdf_signal_impl_base: public sca_core::sca_prim_channel
{
public:

	/** Sets sample rate of the port */
	void set_rate(unsigned long port, unsigned long rate);

	/** Sets number of sample delay of the port */
	void set_delay(unsigned long port, unsigned long samples);

	/** Sets time distance between two samples */
	void set_T(unsigned long port, sca_core::sca_time t_period);

	/** sets absolute time of first sample (must be < than T */
	void set_t0(unsigned long port, sca_core::sca_time t0);

	/** Gets the absolute number of samples (from simulation start) samples
	 of the current call are not included */
	unsigned long get_sample_cnt(unsigned long port);

	/** Gets the setted sampling rate of the port */
	unsigned long& get_rate(unsigned long port);

	/** Gets the setted sampling delay of the port */
	unsigned long& get_delay(unsigned long port);

	/** Gets the calculated sample time distance -> if the time was setted it
	 must be equal to the calculated one - if not the scheduler will through
	 an exception */
	sca_core::sca_time& get_T(unsigned long port);

	/** Gets the calculated time offset (time of the first sample) -> the setted one
	 will be equal to the calculated one -> if not possible (causality) the
	 scheduler will through an exception */
	sca_core::sca_time& get_t0(unsigned long port);

	/** Gets the absolute time (from simulation start) of the first sample of the
	 current call */
	//	sc_time get_time(unsigned long port, unsigned long sample=0)
	//       {
	//           long call_cnt_call =(*(call_counter_refs[port]))%calls_per_period[port];
	//           sc_time ct=get_synchronization_if()->get_cluster_start_time() +
	//                      ((sample+call_cnt_call*get_rate(port))*get_T(port));
	//           return ct;
	//       }


	/** Gets the absolute time (from simulation start) of the first sample of the
	 current call */
	sca_core::sca_time get_time(unsigned long port, unsigned long sample = 0);

	//prepare channel datastructures
	virtual void end_of_elaboration();

	long get_driver_port_id();

	const char* kind() const;

	std::vector<unsigned long> rates;
	std::vector<unsigned long> delays;
	std::vector<sca_core::sca_time> Ts_setted;
	std::vector<sca_core::sca_time> t0s_setted;
	std::vector<sca_core::sca_time> Ts_calculated;
	std::vector<sca_core::sca_time> t0s_calculated;

	void construct();

	sca_tdf_signal_impl_base();
	sca_tdf_signal_impl_base(const char* name_);

	virtual ~sca_tdf_signal_impl_base();

protected:

	virtual bool trace_init(
			sca_util::sca_implementation::sca_trace_object_data& data);

	//is called after cluster was calculated
	virtual void trace(long id,
			sca_util::sca_implementation::sca_trace_buffer& trace_buffer);

	unsigned long buffer_size;
	long* buffer_offsets;

	long* calls_per_period;
	const sc_dt::int64** call_counter_refs;
	long driver_port;
	const sca_core::sca_time* cluster_period;
	bool* is_inport; //flag array for signing inports->check whether write allowed

private:

	virtual void in_delay_buffer_resize(unsigned long n)=0;
	virtual void in_delay_buffer_create(unsigned long b_nr,unsigned long size)=0;
	virtual void create_buffer(unsigned long size)=0;
    virtual void store_trace_buffer_time_stamp(sca_util::sca_implementation::sca_trace_buffer& trace_buffer,unsigned long id,sca_core::sca_time ctime, unsigned long bpos)=0;

    virtual void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)=0;
};

} //namespace sca_implementation
} //namespace sca_tdf



#endif /* SCA_TDF_SIGNAL_IMPL_BASE_H_ */
