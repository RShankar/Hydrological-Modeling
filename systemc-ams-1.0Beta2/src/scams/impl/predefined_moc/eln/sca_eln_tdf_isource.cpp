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

  sca_eln_tdf_isource.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_tdf_isource.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_tdf_isource.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_eln
{

namespace sca_tdf
{

sca_isource::	sca_isource(sc_core::sc_module_name, double scale_) :
	p("p"), n("n"), inp("inp"), scale("scale", scale_)
{
    through_value_available = true;
    i_value=0.0;
}

const char* sca_isource::kind() const
{
	return "sca_eln::sca_tdf::sca_isource";
}



double sca_isource::i_t()
{

    if(!sca_ac_analysis::sca_ac_is_running())
    {
    	i_value=scale.get() * inp.read();
    	return i_value;
    }
    else
    {
        double val = scale.get() * sca_ac_analysis::sca_ac(inp).real();
        return val;
    }
}


void sca_isource::matrix_stamps()
{
    q(p).add_element(SCA_MPTR(sca_isource::i_t),this);
    q(n).sub_element(SCA_MPTR(sca_isource::i_t),this);
}

bool sca_isource::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by teh synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_isource::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = i_value;
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_isource::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return scale.get() * sca_ac_analysis::sca_implementation::sca_ac(inp);
}


} //namespace sca_tdf
} //namespace sca_eln
