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

 sca_lsf_in.h - linear signal flow inport

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 05.03.2009

 SVN Version       :  $Revision: 1031 $
 SVN last checkin  :  $Date: 2010-06-04 18:26:43 +0200 (Fri, 04 Jun 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_lsf_in.h 1031 2010-06-04 16:26:43Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.2.1.3.
 The class sca_lsf::sca_in shall define a port class for the LSF MoC. The port
 shall be bound to a primitve channel of class sca_lsf::sca_signal or a port of
 class sca_lsf::sca_in.
 */

/*****************************************************************************/

#ifndef SCA_LSF_IN_H_
#define SCA_LSF_IN_H_

namespace sca_lsf
{

//class sca_in : public implementation-derived-from sca_core::sca_port&lt; sca_lsf::sca_signal_if >
class sca_in: public sca_core::sca_port<sca_lsf::sca_signal_if>
{
public:
	sca_in();
	explicit sca_in(const char*);

	virtual const char* kind() const;

private:
	// Other members
	//implementation-defined

	// Disabled
	sca_in(const sca_lsf::sca_in&);

	//begin implementation specific

public:  //otherwise all lsf modules must become friend
     /** returns node number of connected wire*/
	 long get_signal_number();

	 /** allows node number access with terminal name only ->
	   * better look and feel for describing matrix_stamps */
	 operator long();

	 // end implementation specific

};

} // namespace sca_lsf

#endif /* SCA_LSF_IN_H_ */
