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

  sca_eln_ideal_transformer.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_ideal_transformer.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_ideal_transformer.h"

namespace sca_eln
{

sca_ideal_transformer::sca_ideal_transformer(sc_core::sc_module_name, double ratio_) :
	p1("p1"), n1("n1"), p2("p2"), n2("n2"), ratio("ratio", ratio_)
{
	through_value_available = false;
	through_value_type = "I";
	through_value_unit = "A";
}


const char* sca_ideal_transformer::kind() const
{
	return "sca_eln::sca_ideal_transformer";
}

void sca_ideal_transformer::matrix_stamps()
{
    nadd = add_equation();

    B(nadd, p1) =  1.0;
    B(nadd, n1) = -1.0;
    B(nadd, p2) = -ratio;
    B(nadd, n2) =  ratio;
    B(p1,   nadd) =  1.0;
    B(n1,   nadd) = -1.0;
    B(p2,   nadd) = -ratio;
    B(n2,   nadd) =  ratio;
}

} //namespace sca_eln

