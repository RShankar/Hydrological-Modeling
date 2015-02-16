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

 sca_lsf_gain.h - linear signal flow gain

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 05.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_lsf_gain.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.2.1.7.
 The class sca_lsf::sca_gain shall implement a primitive module for the LSF MoC
 that realizes the multiplication of an LSF&nbsp;signal by a constant gain.
 The primitive shall contribute the following equation to the equation system:
 y(t) = k*x(t)
 where k is the constant gain coefficient, x(t is the LSF input signal, and
 y(t) is the LSF output signal.
 */

/*****************************************************************************/

#ifndef SCA_LSF_GAIN_H_
#define SCA_LSF_GAIN_H_

namespace sca_lsf
{

//class sca_gain : public implementation-derived-from sca_core::sca_module
class sca_gain: public sca_lsf::sca_module
{
public:
	sca_lsf::sca_in x; // LSF input

	sca_lsf::sca_out y; // LSF output

	sca_core::sca_parameter<double> k; // gain coefficient

	virtual const char* kind() const;

	explicit sca_gain(sc_core::sc_module_name, double k_ = 1.0);

// begin implementation specific

private:
	 virtual void matrix_stamps();

// end implementation specific

};

} // namespace sca_lsf

#endif /* SCA_LSF_GAIN_H_ */
