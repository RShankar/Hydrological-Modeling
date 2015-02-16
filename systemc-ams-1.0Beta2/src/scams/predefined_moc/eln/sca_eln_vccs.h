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
/*
 * LRM clause 4.3.1.9.
 * The class sca_eln::sca_vccs shall implement a primitive module for the
 * ELN MoC that represents a voltage controlled current source. The primitive
 * shall contribute the following equation to the equation system:
 *
 *       i(np,nn)(t) = value * v(ncp,ncn)(t)
 *
 * where value is the scale coefficient in siemens of the control voltage
 * v(ncp,ncn)(t) across terminals ncp and ncn, and i(np,nn)(t) is the current
 * flowing through the primitive from terminal np to terminal nn.
 */

/*****************************************************************************

 sca_eln_vccs.h - electrical linear net voltage controlled current source

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_vccs.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_ELN_VCCS_H_
#define SCA_ELN_VCCS_H_

namespace sca_eln
{

//  class sca_vccs : public implementation-derived-from sca_core::sca_module
class sca_vccs: public sca_eln::sca_module
{
public:
	sca_eln::sca_terminal ncp;
	sca_eln::sca_terminal ncn;

	sca_eln::sca_terminal np;
	sca_eln::sca_terminal nn;

	sca_core::sca_parameter<double> value;

	virtual const char* kind() const;

	explicit sca_vccs(sc_core::sc_module_name, double value_ = 1.0);


	//begin implementation specific

private:
		 virtual void matrix_stamps();



	//end implementation specific

};

} // namespace sca_eln


#endif /* SCA_ELN_VCCS_H_ */
