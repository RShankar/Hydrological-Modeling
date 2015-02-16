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

 sca_eln_tdf_rswitch.h - linear electrical net switch controlled by a tdf signal

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 08.03.2009

 SVN Version       :  $Revision: 1019 $
 SVN last checkin  :  $Date: 2010-06-02 17:26:56 +0200 (Wed, 02 Jun 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_tdf_rswitch.h 1019 2010-06-02 15:26:56Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.21.
 * The class sca_eln::sca_tdf::sca_rswitch shall implement a primitive module
 * for the ELN MoC that represents a switch, which is controlled by a TDF
 * control signal. The primitive shall contribute the following equation to the
 * equation system:
 *
 *        v(p,n)(t) = ron  * i(p,n)(t)  ctrl!=off_state
 *        v(p,n)(t) = roff * i(p,n)(t)  ctrl=off_state
 *
 * where ctrl is the TDF control signal, roff is the resistance of the switch in
 * ohm under the condition that off_state is equal to the TDF control signal,
 * and ron is the resistance of the switch in ohm under the condition that
 * off_state is not equal to the TDF control signal. v(p,n)(t) is the voltage
 * across terminals p and n, and i(p,n)(t) is the current flowing through the
 * primitive from terminal p to terminal n.
 */

/*****************************************************************************/

#ifndef SCA_ELN_TDF_RSWITCH_H_
#define SCA_ELN_TDF_RSWITCH_H_

namespace sca_eln
{

namespace sca_tdf
{

//    class sca_rswitch : public implementation-derived-from sca_core::sca_module,
//                        protected sca_util::sca_traceable_object
class sca_rswitch: public sca_eln::sca_module,
		           public sca_util::sca_traceable_object
{
public:
	sca_eln::sca_terminal p;
	sca_eln::sca_terminal n;

	::sca_tdf::sca_in<bool> ctrl;

	sca_core::sca_parameter<double> ron;
	sca_core::sca_parameter<double> roff;
	sca_core::sca_parameter<bool> off_state;

	virtual const char* kind() const;

	explicit sca_rswitch(sc_core::sc_module_name, double ron_ = 0.0,
			double roff_ = sca_util::SCA_INFINITY, bool off_state_ = false);


	//begin implementation specific
private:

	virtual void matrix_stamps();

	void read_rval();
	void update_rval();

	long nadd;
	double r_val, r_old;
	double ron_eff, roff_eff;
	bool first_call, short_cut;

	 bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
	 void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);
	 sca_util::sca_complex calculate_ac_result(sca_util::sca_complex* res_vec);

	//end implementation specific

};

} // namespace sca_tdf

typedef sca_eln::sca_tdf::sca_rswitch sca_tdf_rswitch;

} // namespace sca_eln


#endif /* SCA_ELN_TDF_RSWITCH_H_ */
