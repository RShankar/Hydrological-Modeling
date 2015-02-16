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

 sca_eln_vsource.h - electrical linear net voltage source

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_vsource.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.16.
 * The class sca_eln::sca_vsource shall implement a primitive module for the
 * ELN MoC that realizes a voltage source. In time-domain simulation, the
 * primitive shall contribute the following equation to the equation system:
 *
 *  v(p,n)(t) = init_value                                             t<delay
 *  v(p,n)(t) = offset + amplitude * sin(2pi*frequency*(t-delay)+phase t>=delay
 *
 * where t is the time, delay is the initial delay in second, init_value is the
 * inital voltage in volt, offset is the offset voltage in volt,amplitude is the
 * source amplitude in volt, frequency is the source frequency in hertz, phase
 * is the source phase in radian, pi is the pi constant,and v(p,n)(t) is the
 * output voltage across terminals p and n. Voltage source parameters shall be
 * set to zero by default.
 * In small-signal frequency-domain simulation, the primitive shall contribute
 * the following equation to the equation system:
 *
 *        v(p,n)(f) = ac_amplitude * (cos(ac_phase) + j*sin(ac_phase))
 *
 * where f is the simulation frequency in hertz, ac_amplitude is the
 * small-signal amplitude in volt, and ac_phase is the small-signal phase in
 * radian.
 * In small-signal frequency-domain noise simulation, the primitive shall
 * contribute the following equation to the equation system:
 *
 *        v(p,n)(f) = ac_noise_amplitude
 *
 * where f is the simulation frequency in hertz, and ac_noise_amplitude is the
 * small-signal noise amplitude in volt.
 */

/*****************************************************************************/

#ifndef SCA_ELN_VSOURCE_H_
#define SCA_ELN_VSOURCE_H_

namespace sca_eln
{

//  class sca_vsource : public implementation-derived-from sca_core::sca_module,
//                      protected sca_util::sca_traceable_object
class sca_vsource: public    sca_eln::sca_module,
		           public sca_util::sca_traceable_object
{
public:
	sca_eln::sca_terminal p;
	sca_eln::sca_terminal n;

	sca_core::sca_parameter<double> init_value;
	sca_core::sca_parameter<double> offset;
	sca_core::sca_parameter<double> amplitude;
	sca_core::sca_parameter<double> frequency;
	sca_core::sca_parameter<double> phase;
	sca_core::sca_parameter<sca_core::sca_time> delay;
	sca_core::sca_parameter<double> ac_amplitude;
	sca_core::sca_parameter<double> ac_phase;
	sca_core::sca_parameter<double> ac_noise_amplitude;

	virtual const char* kind() const;

	explicit sca_vsource(sc_core::sc_module_name, double init_value_ = 0.0,
			double offset_ = 0.0, double amplitude_ = 0.0, double frequency_ =
					0.0, double phase_ = 0.0, sca_core::sca_time delay_ = sc_core::SC_ZERO_TIME, double ac_amplitude_ = 0.0,
			double ac_phase_ = 0.0, double ac_noise_amplitude_ = 0.0);
	//begin implementation specific

private:
		 virtual void matrix_stamps();
		 double v_t();

		 long nadd;

		 bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
		 void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);
		 sca_util::sca_complex calculate_ac_result(sca_util::sca_complex* res_vec);

	//end implementation specific

};

} // namespace sca_eln

#endif /* SCA_ELN_VSOURCE_H_ */
