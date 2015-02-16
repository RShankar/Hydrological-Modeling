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

  sca_conservative_module.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 04.11.2009

   SVN Version       :  $Revision: 1157 $
   SVN last checkin  :  $Date: 2011-02-09 23:08:11 +0100 (Wed, 09 Feb 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_conservative_module.cpp 1157 2011-02-09 22:08:11Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/predefined_moc/conservative/sca_conservative_module.h"


namespace sca_core
{
namespace sca_implementation
{

const char* sca_conservative_module::kind() const
{
	return "sca_core::sca_implementation::sca_conservative_module";
}


sca_conservative_module::sca_conservative_module()
{
  //only base class - has to be assigned by derived class
  view_interface = NULL;
  cluster_id = -1;
  reinit_request_cnt=0;

  call_id_p=NULL;
  call_id_request_init=0;
  value_set=false;
}


sca_conservative_module::sca_conservative_module(sc_core::sc_module_name)
{
	  //only base class - has to be assigned by derived class
	  view_interface = NULL;
	  cluster_id = -1;
	  reinit_request_cnt=0;

	  call_id_p=NULL;
	  call_id_request_init=0;
	  value_set=false;
}


//////////////////////////////////////////////

sca_conservative_module::~sca_conservative_module()
{
}

/**
 * reinitialization of equation system is forced
 */
void sca_conservative_module::request_reinit(int mode)
{
	//if euler step requested - do euler step
	if(*reint_request_i != 1) *reint_request_i = mode;

	call_id_request_init=*call_id_p;

	reinit_request_cnt++;
}


void sca_conservative_module::request_reinit(int mode,double new_val)
{
	value_set=true;
	new_value=new_val;
	request_reinit(mode);
}

void sca_conservative_module::request_reinit(double new_val)
{
	request_reinit(1,new_val);
}

}
}
