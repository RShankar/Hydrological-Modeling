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

 sca_eln_c.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 10.11.2009

 SVN Version       :  $Revision: 1154 $
 SVN last checkin  :  $Date: 2011-02-07 21:17:48 +0100 (Mon, 07 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_c.cpp 1154 2011-02-07 20:17:48Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_c.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_eln
{
sca_c::sca_c(sc_core::sc_module_name, double value_, double q0_) :
	p("p"), n("n"), value("value", value_), q0("q0", q0_)
{
	dc_init=true;
}

const char* sca_c::kind() const
{
	return "sca_eln::sca_c";
}

void sca_c::matrix_stamps()
{
	//nadd - current
	nadd = add_equation();

	if (!sca_ac_analysis::sca_ac_is_running() && dc_init)
	{
		if (q0.get() == sca_util::SCA_UNDEFINED)
		{
			B(nadd, nadd) = -1.0; // i=0
		}
		else
		{
			//voltage source stamps for DC-step

			B(nadd, p) = 1.0;
			B(nadd, n) = -1.0;
			B(p, nadd) = 1.0;
			B(n, nadd) = -1.0;

			double q0_c = q0.get() / value.get();

			//test for NaN and infinity
			if (q0_c == std::numeric_limits<double>::infinity() || q0_c
					== -std::numeric_limits<double>::infinity() || q0_c != q0_c)
			{
				// (q0>0)/0 -> infinity
				if (q0_c == q0_c)
				{
					std::ostringstream str;
					str
							<< "The value of the capacitance cannot be zero if q0 is greater zero "
							<< "(results in an ifinite initial voltage) for: "
							<< name();
					SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
				}

				q(nadd).set_value(0.0);
			}
			else
			{
				q(nadd).set_value(-q0_c);
			}

		}
		add_post_solve_method(SCA_VMPTR(sca_c::post_solve));
	}
	else //for restore for / after AC
	{
		B(p, nadd) = 1.0;
		B(n, nadd) = -1.0;

		A(nadd, p) = value; // C*dv
		A(nadd, n) = -value;
		B(nadd, nadd) = -1.0;
	}
}


void sca_c::post_solve() //set capacitor stamps after first step
{
	B(nadd, p)  = 0.0;
	B(nadd, n)  = 0.0;

	q(nadd).set_value(0.0);


	B(p,nadd) = 1.0;
	B(n,nadd) = -1.0;

	A(nadd, p) = value;  // C*dv
	A(nadd, n) = -value;
	B(nadd, nadd) = -1.0;

	remove_post_solve_method(SCA_VMPTR(sca_c::post_solve));

	request_reinit();

	dc_init=false;
}



bool sca_c::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by teh synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_c::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = x(nadd);
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_c::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return res_vec[nadd];
}

} //namespace sca_eln
