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

  sca_eln_r.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 1096 $
   SVN last checkin  :  $Date: 2010-12-13 21:47:59 +0100 (Mon, 13 Dec 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_r.cpp 1096 2010-12-13 20:47:59Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_r.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_eln
{
sca_r::sca_r(sc_core::sc_module_name, double value_) :
		p("p"), n("n"), value("value", value_)
{
    through_value_available = true;
    through_value_type      = "Current";
    through_value_unit      = "A";
    nadd=-1;
}

const char* sca_r::kind() const
{
	return "sca_eln::sca_r";
}

void sca_r::matrix_stamps()
{
	if(fabs(value.get())>1e-4)
	{
		B(p,p) +=  1.0/value.get();
		B(p,n) += -1.0/value.get();
		B(n,p) += -1.0/value.get();
		B(n,n) +=  1.0/value.get();
	}
	else
	{
	    nadd = add_equation();

	    B(nadd, p) =  1.0;
	    B(nadd, n) = -1.0;
	    B(p, nadd) =  1.0;
	    B(n, nadd) = -1.0;

	    B(nadd, nadd) = -value.get();
	}
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

    double through_value;

    if(nadd<0)
    {
    	through_value = (x(p)-x(n))/value.get();
    }
    else
    {
    	through_value=x(nadd);
    }
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_r::calculate_ac_result(sca_util::sca_complex* res_vec)
{
    //if reference node return 0.0
    sca_util::sca_complex rp = long(p) < 0 ? 0.0 : res_vec[p];
    sca_util::sca_complex rn = long(n) < 0 ? 0.0 : res_vec[n];

    if(nadd>=0) return res_vec[nadd];

    return (rp-rn)/value.get();
}

} //namespace sca_eln
