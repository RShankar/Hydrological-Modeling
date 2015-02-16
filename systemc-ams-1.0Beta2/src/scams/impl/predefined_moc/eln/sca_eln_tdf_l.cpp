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

  sca_eln_tdf_l.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.11.2009

   SVN Version       :  $Revision: 1155 $
   SVN last checkin  :  $Date: 2011-02-08 22:18:52 +0100 (Tue, 08 Feb 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_tdf_l.cpp 1155 2011-02-08 21:18:52Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_tdf_l.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"
#include <limits>


namespace sca_eln
{

namespace sca_tdf
{

sca_l::sca_l(sc_core::sc_module_name, double scale_, double phi0_) :
p("p"), n("n"), inp("inp"), scale("scale", scale_), phi0("phi0", phi0_)
{
    through_value_available = true;
    through_value_type      = "I";
    through_value_unit      = "A";

    init_value=1.0;
    nadd1=-1;
    nadd2= -1;

    dc_init=true;
}

const char* sca_l::kind() const
{
	return "sca_eln::sca_tdf::sca_l";
}



void sca_l::read_lval()
{
	//epsilon: difference between 1 and the smallest value greater than 1
	static const double relerr=6.0*std::numeric_limits<double>::epsilon();

    double curr_value = scale * inp.read();

    double err_tmp=fabs(curr_value-init_value);
    if(err_tmp>relerr*fabs(curr_value))
    {
        init_value = curr_value;

       	if (!sca_ac_analysis::sca_ac_is_running() && dc_init &&
       			(phi0.get()== sca_util::SCA_UNDEFINED))
		{
			B(nadd2, nadd1) = init_value;
		}
		else
		{
			B(nadd1, nadd1) = init_value;
		}

        if(!dc_init) request_reinit(2,init_value); //suppress Euler step -> we assume continuous behavior
        else         request_reinit();
    }
}


void sca_l::matrix_stamps()
{
	// nadd1 - i
	// nadd2 - psi
    nadd1 = add_equation();
    nadd2 = add_equation();

    add_pre_solve_method(SCA_VMPTR(sca_l::read_lval));

	if(!sca_ac_analysis::sca_ac_is_running() && dc_init)
	{
		if (phi0.get() == sca_util::SCA_UNDEFINED)
		{
			//short cut
			B(nadd1, p) = 1.0;
			B(nadd1, n) = -1.0;

			B(p, nadd1) = 1.0;
			B(n, nadd1) = -1.0;

			B(nadd2, nadd2) = -1.0; //psi=L*i
			B(nadd2, nadd1) = init_value;
		}
		else
		{
			B(p, nadd1) = 1.0;
			B(n, nadd1) = -1.0;

			// L*i = psi
			B(nadd1, nadd1) = init_value;
			B(nadd1, nadd2) = -1.0;

			// psi = psi0
			B(nadd2, nadd2) = -1.0;
			q(nadd2).set_value(phi0.get());
		}
		add_post_solve_method(SCA_VMPTR(sca_l::post_solve));
	}
	else
	{
		B(p, nadd1) =  1.0;
		B(n, nadd1) = -1.0;

		// L*i = psi
		B(nadd1, nadd1) =  init_value;
		B(nadd1, nadd2) = -1.0;

	    //dpsi = v
	    B(nadd2, p) =  1.0;
	    B(nadd2, n) = -1.0;
	    A(nadd2, nadd2) = -1.0;
	}
}


void sca_l::post_solve() //set capacitor stamps after first step
{
	//reset dc stamps
	B(nadd2, nadd2) =  0.0;
	q(nadd2).set_value(0.0);

	B(nadd2, nadd1) = 0.0;

	B(nadd1, p) = 0.0;
	B(nadd1, n) = 0.0;

	B(p, nadd1) = 0.0;
	B(n, nadd1) = 0.0;

	remove_post_solve_method(SCA_VMPTR(sca_l::post_solve));

	//set transient stamps
	B(p, nadd1) =  1.0;
	B(n, nadd1) = -1.0;

	// L*i = psi
	B(nadd1, nadd1) =  init_value;
	B(nadd1, nadd2) = -1.0;

    //dpsi = v
    B(nadd2, p) =  1.0;
    B(nadd2, n) = -1.0;
    A(nadd2, nadd2) = -1.0;

	request_reinit();

	dc_init=false;

}



bool sca_l::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by teh synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_l::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = x(nadd1);
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_l::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return res_vec[nadd1];
}


} //namespace sca_tdf
} //namespace sca_eln

