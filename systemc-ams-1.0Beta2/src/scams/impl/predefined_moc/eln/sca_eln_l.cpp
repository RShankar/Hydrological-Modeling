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

  sca_eln_l.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 1155 $
   SVN last checkin  :  $Date: 2011-02-08 22:18:52 +0100 (Tue, 08 Feb 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_l.cpp 1155 2011-02-08 21:18:52Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_l.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"


namespace sca_eln
{

sca_l::sca_l(sc_core::sc_module_name, double value_, double phi0_) :
p("p"), n("n"), value("value", value_), phi0("phi0", phi0_)
{
    through_value_available = true;
    through_value_type      = "I";
    through_value_unit      = "A";

    dc_init=true;
}


const char* sca_l::kind() const
{
	return "sca_eln::sca_l";
}



void sca_l::matrix_stamps()
{
	//nadd = i
	nadd = add_equation();

	if (!sca_ac_analysis::sca_ac_is_running() && dc_init)
	{
		if (phi0.get() == sca_util::SCA_UNDEFINED)
		{
			//short cut
			B(nadd, p) = 1.0;
			B(nadd, n) = -1.0;

			B(p, nadd) = 1.0;
			B(n, nadd) = -1.0;
		}
		else
		{
			B(nadd, nadd) = -1.0;

			double ph_l = phi0.get() / value.get();

			//test for NaN and infinity
			if (ph_l == std::numeric_limits<double>::infinity() || ph_l
					== -std::numeric_limits<double>::infinity() || ph_l != ph_l)
			{
				// (phi>0)/0 -> infinity
				if (ph_l == ph_l)
				{
					std::ostringstream str;
					str
							<< "The value of the inductance cannot be zero if phi0 is greater zero "
							<< "(results in an infinite initial current) for: "
							<< name();
					SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
				}

				q(nadd).set_value(0.0);

				B(p,nadd) = 1.0;
				B(n,nadd) = -1.0;


			}
			else
			{
				q(nadd).set_value(ph_l);

				B(p,nadd) = 1.0;
				B(n,nadd) = -1.0;
			}

		}
		add_post_solve_method(SCA_VMPTR(sca_l::post_solve));
	}
	else
	{
		A(nadd, nadd) = -value.get(); // L*dphi = v

		B(nadd, p) = 1.0;
		B(nadd, n) = -1.0;

		B(p, nadd) = 1.0;
		B(n, nadd) = -1.0;
	}
}


void sca_l::post_solve()
{
	//reset dc stamps
	B(nadd,nadd) = 0.0;
	q(nadd).set_value(0.0);

	B(p,nadd) = 0.0;
	B(n,nadd) = 0.0;


	remove_post_solve_method(SCA_VMPTR(sca_l::post_solve));

	//set transient stamps
	A(nadd,nadd) = -value.get();  // L*dphi = v

	B(nadd,p) =  1.0;
	B(nadd,n) = -1.0;


	B(p,nadd) =  1.0;
	B(n,nadd) = -1.0;

	request_reinit();

	dc_init=false;
}


bool sca_l::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by the synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_l::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = x(nadd);
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_l::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return res_vec[nadd];
}


} //namespace sca_eln

