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

  sca_eln_terminal.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_terminal.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_terminal.h"

namespace sca_eln
{

sca_terminal::sca_terminal() :
	sca_core::sca_port<sca_eln::sca_node_if>(sc_core::sc_gen_unique_name("sca_eln_terminal"))
{
		port_type = SCA_CONSERVATIVE_PORT;
}


sca_terminal::sca_terminal(const char* name_) :
	sca_core::sca_port<sca_eln::sca_node_if>(name_)
{
		port_type = SCA_CONSERVATIVE_PORT;
}

const char* sca_terminal::kind() const
{
	return "sca_eln::sca_terminal";
}


/** returns node number of connected wire*/
long sca_terminal::get_node_number()
{
	return (*this)-> get_node_number();
}

 /** allows node number access with terminal name only ->
  * better look and feel for describing matrix_stamps */
sca_terminal::operator long()
{
	return this->get_node_number();
}



sca_terminal::sca_terminal(const sca_eln::sca_terminal&)
{
}


} // namespace sca_eln
