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

  sca_eln_tdf_vsource.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_tdf_vsource.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_tdf_vsource.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"


namespace sca_eln
{

namespace sca_tdf
{

sca_vsource::sca_vsource(sc_core::sc_module_name, double scale_) :
	p("p"), n("n"), inp("inp"), scale("scale", scale_)
{
    through_value_available = true;
    through_value_type      = "Current";
    through_value_unit      = "A";
}

const char* sca_vsource::kind() const
{
	return "sca_eln::sca_tdf::sca_vsource";
}



double sca_vsource::v_t()
{
    if(!sca_ac_analysis::sca_ac_is_running())
    {
    	return scale * inp.read();
    }
    else
    {
        double val = scale * sca_ac_analysis::sca_ac(inp).real();
        return val;
    }

}


void sca_vsource::matrix_stamps()
{
    nadd = add_equation();

    B(nadd,p) +=  1.0;
    B(nadd,n) += -1.0;
    B(p,nadd) +=  1.0;
    B(n,nadd) += -1.0;

    q(nadd).sub_element(SCA_MPTR(sca_vsource::v_t),this);
}


bool sca_vsource::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by teh synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_vsource::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = x(nadd);
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_vsource::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return res_vec[nadd];
}

} //namespace sca_tdf
} //namespace sca_eln
