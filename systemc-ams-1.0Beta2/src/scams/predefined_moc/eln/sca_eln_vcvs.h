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

 sca_eln_vcvs.h - electrical linear net voltage controlled voltage source

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_vcvs.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.8.
 * The class sca_eln::sca_vcvs shall implement a primitive module for the
 * ELN MoC that represents a voltage controlled voltage source. The primitive
 * shall contribute the following equation to the equation system:
 *
 *       v(np,nn)(t) = value * v(ncp,ncn)(t)
 *
 * where value is the scale coefficient of the control voltage v(ncp,ncn)(t)
 * across terminals ncp and ncn, and v(np,nn)(t) is the voltage across terminals
 * np and nn.
 */

/*****************************************************************************/

#ifndef SCA_ELN_VCVS_H_
#define SCA_ELN_VCVS_H_

namespace sca_eln
{

//  class sca_vcvs : public implementation-derived-from sca_core::sca_module
class sca_vcvs: public sca_eln::sca_module
{
public:
	sca_eln::sca_terminal ncp;
	sca_eln::sca_terminal ncn;

	sca_eln::sca_terminal np;
	sca_eln::sca_terminal nn;

	sca_core::sca_parameter<double> value;

	virtual const char* kind() const;

	explicit sca_vcvs(sc_core::sc_module_name, double value_ = 1.0);

	//begin implementation specific

private:
		 virtual void matrix_stamps();

		 long nadd;

	//end implementation specific

};

} // namespace sca_eln

#endif /* SCA_ELN_VCVS_H_ */
