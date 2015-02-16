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

 sca_eln_l.h - electrical linear net inductor

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 1101 $
 SVN last checkin  :  $Date: 2011-01-02 17:42:36 +0100 (Sun, 02 Jan 2011) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_l.h 1101 2011-01-02 16:42:36Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.7.
 * The class sca_eln::sca_l shall implement a primitive module for the ELN MoC
 * that represents an inductor. The primitive shall contribute the following
 * equation to the equation system:
 *
 *        v(p,n)(t) = value * d(i(p,n)(t) + phi0/value) / dt
 *
 * where value is the inductor value in henry, phi0 is the initial magnetic flux
 * in weber, v(p,n)(t)is the voltage across the inductor between terminals p and
 * n, and i(p,n)(t) is the current through the inductor flowing from terminal p
 * to terminal n.
 */

/*****************************************************************************/

#ifndef SCA_ELN_L_H_
#define SCA_ELN_L_H_

namespace sca_eln
{

//  class sca_l : public implementation-derived-from sca_core::sca_module,
//                protected sca_util::sca_traceable_object
class sca_l: public sca_eln::sca_module,
		     public sca_util::sca_traceable_object
{
public:
	sca_eln::sca_terminal p;
	sca_eln::sca_terminal n;

	sca_core::sca_parameter<double> value;
	sca_core::sca_parameter<double> phi0;

	virtual const char* kind() const;

	explicit sca_l(sc_core::sc_module_name, double value_ = 1.0, double phi0_ =
			0.0);
	//begin implementation specific

private:
		 virtual void matrix_stamps();

		 long nadd;
		 bool dc_init;

		 void post_solve();

		 bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
		 void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);
		 sca_util::sca_complex calculate_ac_result(sca_util::sca_complex* res_vec);

	//end implementation specific

};

} // namespace sca_eln

#endif /* SCA_ELN_L_H_ */
