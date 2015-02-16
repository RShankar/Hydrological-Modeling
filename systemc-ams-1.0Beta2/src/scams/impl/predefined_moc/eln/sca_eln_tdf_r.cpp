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

  sca_eln_tdf_r.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 1113 $
   SVN last checkin  :  $Date: 2011-01-23 17:09:13 +0100 (Sun, 23 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_tdf_r.cpp 1113 2011-01-23 16:09:13Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_tdf_r.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

#include <limits>

namespace sca_eln
{

namespace sca_tdf
{

sca_r::	sca_r(sc_core::sc_module_name, double scale_) :
	p("p"), n("n"), inp("inp"), scale("scale", scale_)
{
    through_value_available = true;
    through_value_type      = "I";
    through_value_unit      = "A";

    init_value=1.0;
}


const char* sca_r::kind() const
{
	return "sca_eln::sca_tdf::sca_r";
}



void sca_r::read_rval()
{
	//epsilon: difference between 1 and the smallest value greater than 1
	static const double relerr=6.0*std::numeric_limits<double>::epsilon();

    double curr_value = scale * inp.read();

    double err_tmp=fabs(curr_value-init_value);
    if(err_tmp>relerr*fabs(curr_value))
    {
        init_value = curr_value;

        B(nadd, nadd) = -curr_value;
        request_reinit(2,curr_value); //suppress Euler step -> we assume continuous behavior
    }
}


void sca_r::matrix_stamps()
{
    nadd = add_equation();

    B(nadd, p) =  1.0;
    B(nadd, n) = -1.0;
    B(p, nadd) =  1.0;
    B(n, nadd) = -1.0;

    B(nadd, nadd) = -init_value;

    add_pre_solve_method(SCA_VMPTR(sca_r::read_rval));
}

bool sca_r::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by teh synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_r::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = x(nadd);
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_r::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return res_vec[nadd];
}




} //namespace sca_tdf
} //namespace sca_eln

