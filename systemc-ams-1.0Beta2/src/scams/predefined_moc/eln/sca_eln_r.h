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

 sca_eln_r.h - electrical linear net resistor

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 1095 $
 SVN last checkin  :  $Date: 2010-12-13 21:45:09 +0100 (Mon, 13 Dec 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_r.h 1095 2010-12-13 20:45:09Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.5.
 * The class sca_eln::sca_r shall implement a primitive module for the ELN MoC
 * that represents a resistor. The primitive shall contribute the following
 * equation to the equation system:
 *
 *        i(p,n)(t) = v(p,n)(t) / value
 *
 * where value is the resistor value in ohm, v(p,n)(t) is the voltage across the
 * resistor between terminals p and n, and i(p,n)(t) is the current through the
 * resistor flowing from terminal p to terminal n.
 */

/*****************************************************************************/

#ifndef SCA_ELN_R_H_
#define SCA_ELN_R_H_

namespace sca_eln
{

//  class sca_r : public implementation-derived-from sca_core::sca_module,
//                protected sca_util::sca_traceable_object
class sca_r: public    sca_eln::sca_module ,
             public sca_util::sca_traceable_object
{
public:
	sca_eln::sca_terminal p;
	sca_eln::sca_terminal n;

	sca_core::sca_parameter<double> value;

	virtual const char* kind() const;

	explicit sca_r(sc_core::sc_module_name, double value_ = 1.0);

	//begin implementation specific

private:
	 virtual void matrix_stamps();

	 bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
	 void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);
	 sca_util::sca_complex calculate_ac_result(sca_util::sca_complex* res_vec);

	 long nadd;



	//end implementation specific
};

} // namespace sca_eln


#endif /* SCA_ELN_R_H_ */
