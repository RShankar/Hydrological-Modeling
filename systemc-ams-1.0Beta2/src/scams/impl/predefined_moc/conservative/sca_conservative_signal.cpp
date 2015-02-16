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

  sca_conservative_signal.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 04.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_conservative_signal.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/predefined_moc/conservative/sca_conservative_signal.h"



namespace sca_core
{
namespace sca_implementation
{

const char* sca_conservative_signal::kind() const
{
	return "sca_core::sca_conservative_signal";
}

bool sca_conservative_signal::ignore_node() const
{
	return reference_node;
}

long sca_conservative_signal::get_node_number() const
{
	if(reference_node) return -1;
	else               return node_number;
}


sca_conservative_signal::sca_conservative_signal() :
     sca_core::sca_prim_channel(sc_core::sc_gen_unique_name("sca_conservative_signal"))
{
	construct();
}


sca_conservative_signal::sca_conservative_signal(const char* _name) :
	sca_core::sca_prim_channel(_name)
{
	construct();
}



void sca_conservative_signal::construct()
{
    node_number    = -1;
    equation_id    = -1;
    cluster_id     = -1;
    reference_node  = false;
    connects_cviews = false;
}


} //namespace sca_implementation
} //namespace sca_implementation

