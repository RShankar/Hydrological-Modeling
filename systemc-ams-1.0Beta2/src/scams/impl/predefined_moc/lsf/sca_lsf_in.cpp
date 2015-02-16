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

  sca_lsf_out.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.01.2010

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_in.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/lsf/sca_lsf_in.h"

namespace sca_lsf
{

sca_in::sca_in() :
	sca_core::sca_port<sca_lsf::sca_signal_if>(sc_core::sc_gen_unique_name("sca_lsf_in"))
{
		port_type = SCA_IN_PORT;
}


sca_in::sca_in(const char* name_) :
	sca_core::sca_port<sca_lsf::sca_signal_if>(name_)
{
		port_type = SCA_IN_PORT;
}

const char* sca_in::kind() const
{
	return "sca_lsf::sca_in";
}


/** returns node number of connected wire*/
long sca_in::get_signal_number()
{
	return (*this)-> get_node_number();
}

 /** allows node number access with terminal name only ->
  * better look and feel for describing matrix_stamps */
sca_in::operator long()
{
	return this->get_signal_number();
}



sca_in::sca_in(const sca_lsf::sca_in&)
{
}


} // namespace sca_lsf


