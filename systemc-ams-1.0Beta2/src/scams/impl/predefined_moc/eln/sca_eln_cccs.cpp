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

  sca_eln_cccs.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_cccs.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_cccs.h"

namespace sca_eln
{
sca_cccs::	sca_cccs(sc_core::sc_module_name, double value_) :
	ncp("ncp"), ncn("ncn"), np("np"), nn("nn"), value("value", value_)
{
    through_value_available = false;
    through_value_type      = "I";
    through_value_unit      = "A";
}

const char* sca_cccs::kind() const
{
	return "sca_eln::sca_cccs";
}



void sca_cccs::matrix_stamps()
{
   nadd = add_equation();

   B(nadd,ncp) =  1.0;
   B(nadd,ncn) = -1.0;
   B(ncp,nadd) =  1.0;
   B(ncn,nadd) = -1.0;

   B(np,nadd)  =  value;
   B(nn,nadd)  = -value;
}

} //namespace sca_eln

