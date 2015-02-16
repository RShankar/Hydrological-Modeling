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

 sca_eln_tdf_r.h - electrical linear net resistor controlled by a tdf signal

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 08.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_tdf_r.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.18.
 * The class sca_eln::sca_tdf::sca_r shall implement a primitive module for the
 * ELN MoC that represents a resistor, which resistance is controlled by a
 * TDF input signal. The primitive shall contribute the following equation to
 * the equation system:
 *
 *        v(p,n)(t) = scale * inp * i(p,n)(t)
 *
 * where scale is the constant scale coefficient, inp is the TDF input signal,
 * v(p,n)(t) is the voltage across terminals p and n, and i(p,n)(t) is the
 * current flowing through the primitive from terminal p to terminal n. The
 * product of scale and inp shall be interpreted as the resistance in ohm.
 */

/*****************************************************************************/

#ifndef SCA_ELN_TDF_R_H_
#define SCA_ELN_TDF_R_H_

namespace sca_eln
{

namespace sca_tdf
{

//    class sca_r : public implementation-derived-from sca_core::sca_module,
//                  protected sca_util::sca_traceable_object
class sca_r: public    sca_eln::sca_module,
		     public sca_util::sca_traceable_object
{
public:
	sca_eln::sca_terminal p;
	sca_eln::sca_terminal n;

	::sca_tdf::sca_in<double> inp;

	sca_core::sca_parameter<double> scale;

	virtual const char* kind() const;

	explicit sca_r(sc_core::sc_module_name, double scale_ = 1.0);


	//begin implementation specific
private:
	virtual void matrix_stamps();

	void read_rval();

	long nadd;
	double init_value;

	 bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
	 void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);
	 sca_util::sca_complex calculate_ac_result(sca_util::sca_complex* res_vec);

	//end implementation specific

};

} // namespace sca_tdf

typedef sca_eln::sca_tdf::sca_r sca_tdf_r;

} // namespace sca_eln


#endif /* SCA_ELN_TDF_R_H_ */