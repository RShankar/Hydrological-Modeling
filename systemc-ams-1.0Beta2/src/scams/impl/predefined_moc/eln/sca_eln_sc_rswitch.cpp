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

 sca_eln_sc_rswitch.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 10.11.2009

 SVN Version       :  $Revision: 1113 $
 SVN last checkin  :  $Date: 2011-01-23 17:09:13 +0100 (Sun, 23 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_sc_rswitch.cpp 1113 2011-01-23 16:09:13Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_sc_rswitch.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_eln
{

namespace sca_de
{

sca_rswitch::sca_rswitch(sc_core::sc_module_name, double ron_, double roff_,
		bool off_state_) :
	p("p"), n("n"), ctrl("ctrl"), ron("ron", ron_), roff("roff", roff_),
			off_state("off_state", off_state_)
{
	through_value_available = true;
	through_value_type = "I";
	through_value_unit = "A";

	r_val = r_old = 1e5;
	ron_eff = 1.0;
	roff_eff = 1.0e6;

	first_call = true;
	short_cut = false;

	//TODO find better solution -> inserts additional port in database
	conv_port = new ::sca_tdf::sca_de::sca_in<bool>("converter_port");
	conv_port->bind(ctrl);
}

//////////////////

const char* sca_rswitch::kind() const
{
	return "sca_eln::sca_de::sca_rswitch";
}

////////////////////

void sca_rswitch::update_rval()
{
	if (get_time() == sc_core::SC_ZERO_TIME)
		read_rval();

	if (r_old != r_val)
	{
		if (short_cut)
		{
			B(nadd, p) = 1.0;
			B(nadd, n) = -1.0;
			B(p, nadd) = 1.0;
			B(n, nadd) = -1.0;

			B(nadd, nadd) = -r_val;
		}
		else
		{
			B(p, p) -= 1.0 / r_old;
			B(p, n) -= -1.0 / r_old;
			B(n, p) -= -1.0 / r_old;
			B(n, n) -= 1.0 / r_old;

			B(p, p) += 1.0 / r_val;
			B(p, n) += -1.0 / r_val;
			B(n, p) += -1.0 / r_val;
			B(n, n) += 1.0 / r_val;
		}

		r_old = r_val;
		request_reinit(1,r_val);
	}
}

//////////////////

void sca_rswitch::read_rval()
{
	bool sw = conv_port->read();
	if (sw ^ off_state)
		r_val = ron_eff;
	else
		r_val = roff_eff;
}

/////////////////////////

static const double R_MIN_VAL = 1e-16;
static const double R_MAX_VAL = 1e16;

static const double R_MIN_SW_VAL = 1e-2;

//////////////////

void sca_rswitch::matrix_stamps()
{
	if (first_call)
	{
		first_call = false;

		ron_eff = ron.get();
		roff_eff = roff.get();

		if (((fabs(ron_eff) < R_MIN_SW_VAL) && (fabs(ron_eff) < R_MAX_VAL))
				|| ((fabs(roff_eff) < R_MIN_SW_VAL) && (fabs(roff_eff)
						< R_MAX_VAL)))
		{
			if (roff_eff > R_MAX_VAL)
				roff_eff = R_MAX_VAL;
			else if (roff_eff < -R_MAX_VAL)
				roff_eff = -R_MAX_VAL;

			if (ron_eff > R_MAX_VAL)
				ron_eff = R_MAX_VAL;
			else if (ron_eff < -R_MAX_VAL)
				ron_eff = -R_MAX_VAL;

			short_cut = true;
		}
		else
		{
			short_cut = false;
			if (fabs(ron_eff) < R_MIN_VAL)
			{
				if (ron_eff < 0.0)
					ron_eff = -R_MIN_VAL;
				else
					ron_eff = R_MIN_VAL;
			}

			if (fabs(roff_eff) < R_MIN_VAL)
			{
				if (roff_eff < 0.0)
					roff_eff = -R_MIN_VAL;
				else
					roff_eff = R_MIN_VAL;
			}
		}

		r_old = r_val = roff_eff;
	}

	if (short_cut)
	{
		nadd = add_equation();

		B(nadd, p) = 1.0;
		B(nadd, n) = -1.0;
		B(p, nadd) = 1.0;
		B(n, nadd) = -1.0;

		B(nadd, nadd) = -r_val;
	}
	else
	{
		B(p, p) += 1.0 / r_val;
		B(p, n) += -1.0 / r_val;
		B(n, p) += -1.0 / r_val;
		B(n, n) += 1.0 / r_val;
	}

	add_pre_solve_method(SCA_VMPTR(sca_rswitch::update_rval));
	add_post_solve_method(SCA_VMPTR(sca_rswitch::read_rval));
}

bool sca_rswitch::trace_init(
		sca_util::sca_implementation::sca_trace_object_data& data)
{
	data.type = through_value_type;
	data.unit = through_value_unit;

	//trace will be activated after every complete cluster calculation
	//by teh synchronization layer
	get_sync_domain()->add_solver_trace(data);
	return true;
}

void sca_rswitch::trace(long id,
		sca_util::sca_implementation::sca_trace_buffer& buffer)
{
	sca_core::sca_time ctime = sca_eln::sca_module::get_time();

	double through_value;

	if (short_cut)
	{
		through_value = x(nadd);
	}
	else
	{
		through_value = (x(p) - x(n)) / r_val;
	}
	buffer.store_time_stamp(id, ctime, through_value);
}


////////////

sca_util::sca_complex sca_rswitch::calculate_ac_result( sca_util::sca_complex* res_vec)
{
	if (short_cut)
	{
		return res_vec[nadd];
	}
	else
	{
		//if reference node return 0.0
		sca_util::sca_complex rp = long(p) < 0 ? 0.0 : res_vec[p];
		sca_util::sca_complex rn = long(n) < 0 ? 0.0 : res_vec[n];

		return (rp - rn) / r_val;
	}
}

} //namespace sca_de
} //namespace sca_eln

