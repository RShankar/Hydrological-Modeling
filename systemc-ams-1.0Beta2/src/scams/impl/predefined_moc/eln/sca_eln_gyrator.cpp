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

 sca_eln_gyrator.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 10.11.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_gyrator.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_gyrator.h"

namespace sca_eln
{

sca_gyrator::sca_gyrator(sc_core::sc_module_name, double g1_,
		double g2_) :
	p1("p1"), n1("n1"), p2("p2"), n2("n2"), g1("g1", g1_), g2("g2", g2_)
{
	through_value_available = false;
	through_value_type = "I";
	through_value_unit = "A";
}


const char* sca_gyrator::kind() const
{
	return "sca_eln::sca_gyrator";
}

void sca_gyrator::matrix_stamps()
{
    B(p2, p1) += -g1;
    B(p2, n1) +=  g1;
    B(n2, p1) +=  g1;
    B(n2, n1) += -g1;
    B(p1, p2) +=  g2;
    B(p1, n2) += -g2;
    B(n1, p2) += -g2;
    B(n1, n2) +=  g2;
}

} //namespace sca_eln

