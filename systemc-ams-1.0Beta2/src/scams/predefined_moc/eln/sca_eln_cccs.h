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

 sca_eln_cccs.h - electrical linear net current controlled current source

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_cccs.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.11.
 * The class sca_eln::sca_cccs shall implement a primitive module for the
 * ELN MoC that represents a current controlled current source. The primitive
 * shall contribute the following equations to the equation system:
 *
 *         i(np,nn)(t)   = value * i(ncp,ncn)(t)
 *         v(ncp,ncn)(t) = 0
 *
 * where value is the scale coefficient of the current i(ncp,ncn)(t) flowing
 * through the primitive from terminal ncp to terminal ncn, i(np,nn)(t) is the
 * current flowing through the primitive from terminal np to terminal nn, and
 * v(ncp,ncn)(t) is the voltage across terminals ncp and ncn.
 */

/*****************************************************************************/

#ifndef SCA_ELN_CCCS_H_
#define SCA_ELN_CCCS_H_

namespace sca_eln
{

//  class sca_cccs : public implementation-derived-from sca_core::sca_module
class sca_cccs: public sca_eln::sca_module
{
public:
	sca_eln::sca_terminal ncp;
	sca_eln::sca_terminal ncn;

	sca_eln::sca_terminal np;
	sca_eln::sca_terminal nn;

	sca_core::sca_parameter<double> value;

	virtual const char* kind() const;

	explicit sca_cccs(sc_core::sc_module_name, double value_ = 1.0);


	//begin implementation specific

private:
		 virtual void matrix_stamps();

		 long nadd;

	//end implementation specific

};

} // namespace sca_eln

#endif /* SCA_ELN_CCCS_H_ */
