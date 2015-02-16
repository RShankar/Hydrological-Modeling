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

 sca_tdf_ltf_nd_proxy.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 21.10.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_ct_ltf_nd_proxy.cpp 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/impl/predefined_moc/tdf/sca_tdf_ct_ltf_nd_proxy.h"
#include "scams/impl/solver/utilities/sparse_library/linear_analog_solver.h"

namespace sca_tdf
{
namespace sca_implementation
{

sca_ct_ltf_nd_proxy::sca_ct_ltf_nd_proxy(::sc_core::sc_object* object)
{
	num_old_ref = NULL;
	den_old_ref = NULL;
	reinit_request = true;
	q = NULL;
	A = NULL;
	B = NULL;
	q2 = NULL;
	A2 = NULL;
	B2 = NULL;
	num2_ltf = NULL;
	memsize = 0;
	first_step = true;
	time_step = sc_core::SC_ZERO_TIME;
	time_interval = sc_core::SC_ZERO_TIME;
	sdata = NULL;
	sdata2 = NULL;
	last_h = -1.0;
	module_activations = -1;
	pending_calculation = false;
	causal_warning_reported = false;

	ltf_object = object;
	sc_core::sc_object* pobj = object->get_parent();
	parent_module = dynamic_cast<sca_tdf::sca_module*> (pobj);

	if (!parent_module)
	{
		std::ostringstream str;
		if (object != NULL)
			str << object->name() << ":";
		str << "\n\tA " << "sca_tdf::sca_ltf_nd"
				<< " must be used only in the context of a "
				<< "sca_tdf::sca_module (SCA_TDF_MODULE). " << std::endl;

		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	delay_buffer = NULL;
	dc_init=true;
}

//////////////////////////////////////////////////////////////////

void sca_ct_ltf_nd_proxy::set_max_delay(sca_core::sca_time mdelay)
{
	if (delay_buffer != NULL)
	{
		delete delay_buffer;
		delay_buffer = NULL;
	}

	max_delay = mdelay;
	if (mdelay != sc_core::SC_ZERO_TIME)
	{
		delay_buffer = new sca_ct_delay_buffer_double;
		delay_buffer->set_max_delay(mdelay);
	}
}

//////////////////////////////////////////////////////////////////


//quick check for coefficient change
inline bool sca_ct_ltf_nd_proxy::coeff_changed(const sca_util::sca_vector<
		double>& num, const sca_util::sca_vector<double>& den)
{
	//was there a write access or has the object changed since the last call
	if (num.get_access_flag() || &num != num_old_ref)
	{
		num_old_ref = &num;

		//has the dimension changed since the last call
		if (num.length() != num_old.length())
		{
			num_old = num;
			den_old = den;
			den_old_ref = &den;

			const_cast<sca_util::sca_vector<double>*> (&num)->reset_access_flag();
			const_cast<sca_util::sca_vector<double>*> (&den)->reset_access_flag();

			return true;
		}

		//has a value changed since the last call
		for (unsigned int i = 0; i < num.length(); i++)
			if (num[i] != num_old[i])
			{
				num_old = num;
				den_old = den;
				den_old_ref = &den;

				const_cast<sca_util::sca_vector<double>*> (&num)->reset_access_flag();
				const_cast<sca_util::sca_vector<double>*> (&den)->reset_access_flag();

				return true;
			}
	}

	//the same for the denumerator
	if (den.get_access_flag() || &den != den_old_ref)
	{
		den_old_ref = &den;

		if (den.length() != den_old.length())
		{
			num_old = num;
			den_old = den;
			num_old_ref = &num;

			const_cast<sca_util::sca_vector<double>*> (&num)->reset_access_flag();
			const_cast<sca_util::sca_vector<double>*> (&den)->reset_access_flag();

			return true;
		}

		for (unsigned int i = 0; i < den.length(); i++)
			if (den[i] != den_old[i])
			{
				num_old = num;
				den_old = den;
				num_old_ref = &num;

				const_cast<sca_util::sca_vector<double>*> (&num)->reset_access_flag();
				const_cast<sca_util::sca_vector<double>*> (&den)->reset_access_flag();

				return true;
			}
	}

	return false;
}

inline void sca_ct_ltf_nd_proxy::setup_equation_system()
{
	//double* reference for fast access
	den_ltf = den_old.get_flat();
	num_ltf = num_old.get_flat();

	den_size = den_old.length();
	num_size = num_old.length();
	state_size = (den_size > num_size) ? den_size - 1 : num_size;

	number_of_equations = den_size - 1;
	if (state_size > number_of_equations)
	{
		number_of_equations2 = state_size - number_of_equations;
	}
	else
	{
		number_of_equations2 = 0;
	}

	//preparing memory for holding the two equation systems
	unsigned long new_memsize;
	new_memsize = num_size + 2* number_of_equations * number_of_equations
			+ number_of_equations + 2* number_of_equations2
			* number_of_equations2 + number_of_equations2;

	if (new_memsize != memsize)
	{
		if (num2_ltf != NULL)
			delete[] num2_ltf;
		num2_ltf = new double[new_memsize];

		if (num2_ltf == NULL)
		{
			std::ostringstream str;
			str << "Can't allocate enough memory for sca_ltf_nd:  "
					<< ltf_object->name() << " required size: " << memsize
					<< " * " << sizeof(double) << " Bytes" << std::endl;
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}

		memsize = new_memsize;
	}

	q = num2_ltf + num_size;
	A = q + number_of_equations;
	B = A + number_of_equations * number_of_equations;
	q2 = B + number_of_equations * number_of_equations;
	A2 = q2 + number_of_equations2;
	B2 = A2 + number_of_equations2 * number_of_equations2;
	memset((void*) (num2_ltf), 0, new_memsize * sizeof(double)); //set all to zero


	//prepare matrices Adx+Bx+q(t)=0 for fractional part

	q_dn = 1.0 / den_ltf[number_of_equations];

	if (number_of_equations > 0)
	{
		eq_in = &q[number_of_equations - 1];

		A[0] = 1.0;
		B[number_of_equations - 1] = q_dn * den_ltf[0];
		for (unsigned long i = 1; i < number_of_equations; ++i)
		{
			A[i * (number_of_equations + 1)] = 1.0;
			B[i * (number_of_equations + 1) - 1] = -1.0;
			B[(i + 1) * number_of_equations - 1] = q_dn * den_ltf[i];
		}
	}

	//in the case num.size() > den.size() setup equation for non-fractional part
	if (number_of_equations2 > 0)
	{
		memcpy((void*) num2_ltf, (void*) num_ltf, num_size * sizeof(double));

		for (long i = long(num_size) - 1; i >= long(number_of_equations); --i)
		{
			num2_ltf[i] *= q_dn;
			double c = num2_ltf[i];
			unsigned long start = i - number_of_equations;
			for (long j = start; j < i; ++j)
				num2_ltf[j] -= c * den_ltf[j - start];
		}

		B2[state_size - den_size] = 1.0;
		for (unsigned long i = 1; i < number_of_equations2; ++i)
		{
			A2[i * (number_of_equations2 + 1)] = 1.0;
			B2[i * (number_of_equations2 + 1) - 1] = -1.0;
		}
	}
}

inline void sca_ct_ltf_nd_proxy::initialize_equation_system(int init_mode,
		double h)
{
	if (number_of_equations > 0)
	{
		int err = ana_reinit(A, B, number_of_equations, h, &sdata, init_mode);
		if (err)
		{
			std::ostringstream str;
			str << ltf_object->name()
					<< ":\n\tInitializing DAE system returned code: " << err
					<< ", step size = " << sca_core::sca_time(h,
					sc_core::SC_SEC) << "." << std::endl;
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}
	}

	if (number_of_equations2 > 0)
	{
		int err = ana_reinit(A2, B2, number_of_equations2, h, &sdata2,
				init_mode);
		if (err)
		{
			std::ostringstream str;
			str << ltf_object->name()
					<< ":\n\tInitializing DAE system returned code: " << err
					<< ", step size = " << sca_core::sca_time(h,
					sc_core::SC_SEC) << "." << std::endl;
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}
	}
}

inline void sca_ct_ltf_nd_proxy::initialize()
{
	register unsigned long s_size=0;

	//if internal S is used it will be reset for
	//every re-initialization
	s_intern.remove();

	dn = den_old.length();
	if (dn == 0)
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tThe denumerator's polynomial must be at least non-zero."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	nn = num_old.length();
	if (nn == 0)
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tAn empty numerator does not make sense." << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	state_size = (dn > nn) ? dn - 1 : nn;

	if (statep != NULL)
	{
		if (statep->length() == 0)
		{
			statep->resize(state_size);
		}

		s_size = statep->length();
	}

	if (state_size != s_size)
	{
		std::ostringstream str;
		str << ltf_object->name() << ":\n\tThe size of the state vector ("
				<< s_size << ") does not match the number of equations ("
				<< state_size << ")." << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}
}

inline double sca_ct_ltf_nd_proxy::calculate(double invalue)
{
	if (number_of_equations > 0)
	{
		*eq_in = -q_dn * scale_factor * invalue;
	}

	if (number_of_equations2 > 0)
	{
		for (unsigned long i = number_of_equations, j = state_size
				- number_of_equations - 1; i < state_size; ++i, --j)
		{
			q2[j] = -num2_ltf[i] * scale_factor * invalue;
		}
	}

	s = statep->get_flat();

	if (number_of_equations > 0) //solve fractional part
	{
		if(!dc_init) ana_solv(q, s, sdata);
	}

	double scalar_out = 0.0;

	if (number_of_equations2 > 0) //solve non-fractional part
	{
		if(!dc_init)
		{
			ana_solv(q2, s + number_of_equations, sdata2);
		}
		else
		{
			//algebraic connection
			s[number_of_equations]=-q2[0];
		}

		scalar_out = s[number_of_equations];

		for (unsigned long i = 0; i < number_of_equations; ++i)
		{
			scalar_out += num2_ltf[i] * s[i];
		}
	}
	else
	{
		for (unsigned long i = 0; i < num_size; ++i)
		{
			scalar_out += num_ltf[i] * s[i];
		}
	}

	dc_init=false;
	return scalar_out;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		sca_util::sca_vector<double>& state, double input, double k,
		sca_core::sca_time tstep)
{
	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_nd calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(num, den);

	sc_dt::int64 ccnt = **(parent_module->call_counter);
	if (ccnt != module_activations)
	{
		current_time = sc_core::SC_ZERO_TIME;
		module_time_step = parent_module->get_timestep();
	}

	if (tstep == sc_core::SC_ZERO_TIME)
	{
		if (current_time == module_time_step)
		{
			std::ostringstream str;
			str
					<< "The current timestep becomes zero due the time distance between "
						"the time reached by the last execution has been reached the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name() << " ltf time since activation: "
					<< current_time << " module timestep: " << module_time_step;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
		time_interval = module_time_step - current_time;
		calculate_to_end = true;
	}
	else
	{
		time_interval = tstep;
		calculate_to_end = false;

		if (current_time > module_time_step)
		{
			std::ostringstream str;
			str << "The requested timestep ( " << tstep
					<< " ) is larger than the time distance between "
						"the time reached by the last execution and the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name() << " ltf time since activation: "
					<< current_time << " module timestep: " << module_time_step;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	in_time_step = time_interval;
	first_in_time_step = in_time_step;

	if (ccnt != module_activations)
	{
		if (ccnt == 0)
		{
			last_in_time = sc_core::SC_ZERO_TIME - in_time_step;
		}
		else
		{
			first_in_time_step = parent_module->get_time() - last_in_time;
		}
		module_activations = ccnt;
	}

	intype = IN_SCALAR;
	in_scalar = input;
	number_of_in_values = 1;
	scale_factor = k;
	in_index_offset = 0;

	statep = &state;

	ct_in_delay = ct_delay;

	if (delay_buffer == NULL)
	{
		if (ct_delay > in_time_step)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the in timestep: "
					<< in_time_step
					<< " otherwise you have to specify the maximum delay by the method: "
					<< " set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}
	else
	{
		if (ct_delay > max_delay)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the maximum delay: "
					<< max_delay
					<< " set by the method: set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		delay_buffer->store_value(parent_module->get_time()+current_time, input);
	}

	current_time += time_interval;
}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		sca_util::sca_vector<double>& state,
		const sca_util::sca_vector<double>& input, double k,
		sca_core::sca_time tstep)
{

	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_nd calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(num, den);

	sc_dt::int64 ccnt = **(parent_module->call_counter);
	if (ccnt != module_activations)
	{
		current_time = sc_core::SC_ZERO_TIME;
		module_time_step = parent_module->get_timestep();
	}

	if (tstep == sc_core::SC_ZERO_TIME)
	{
		if (current_time == module_time_step)
		{
			std::ostringstream str;
			str
					<< "The current timestep becomes zero due the time distance between "
						"the time reached by the last execution has been reached the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name() << " ltf time since activation: "
					<< current_time << " module timestep: " << module_time_step;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
		time_interval = module_time_step - current_time;
		calculate_to_end = true;
	}
	else
	{
		time_interval = tstep;
		calculate_to_end = false;

		if (current_time > module_time_step)
		{
			std::ostringstream str;
			str << "The requested timestep ( " << tstep
					<< " ) is larger than the time distance between "
						"the time reached by the last execution and the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name() << " ltf time since activation: "
					<< current_time << " module timestep: " << module_time_step;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	number_of_in_values = input.length();
	in_time_step = time_interval / number_of_in_values;
	first_in_time_step = in_time_step;

	if (ccnt != module_activations)
	{
		if (ccnt == 0)
		{
			last_in_time = sc_core::SC_ZERO_TIME - in_time_step;
		}
		else
		{
			first_in_time_step = parent_module->get_time() - last_in_time;
		}
		module_activations = ccnt;
	}

	current_time += time_interval;

	in_index_offset = 0;
	intype = IN_VECTOR;
	in_vector = &input;
	scale_factor = k;

	statep = &state;

	ct_in_delay = ct_delay;

	if (delay_buffer == NULL)
	{
		if (ct_delay > in_time_step)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the in timestep: "
					<< in_time_step
					<< " otherwise you have to specify the maximum delay by the method: "
					<< " set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}
	else
	{
		if (ct_delay > max_delay)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the maximum delay: "
					<< max_delay
					<< " set by the method: set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		for (unsigned i = 0; i < number_of_in_values; i++)
		{
			delay_buffer->store_value(current_time + i * in_time_step, input[i]);
		}
	}

}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		sca_util::sca_vector<double>& state,
		const sca_tdf::sca_in<double>& input, double k)
{
	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_nd calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(num, den);

	time_interval = parent_module->get_timestep();

	intype = IN_SCA_PORT;
	in_sca_port = &input;
	number_of_in_values = input.get_rate();
	in_time_step = time_interval / number_of_in_values;
	scale_factor = k;
	calculate_to_end = true;

	sc_dt::int64 ccnt = **(parent_module->call_counter);
	if (ccnt != module_activations)
	{
		current_time = sc_core::SC_ZERO_TIME;
		module_activations = ccnt;
		module_time_step = parent_module->get_timestep();
		in_index_offset = 0;

		if (ccnt == 0)
		{
			last_in_time = sc_core::SC_ZERO_TIME - in_time_step;
			first_in_time_step = in_time_step;
		}
		else
		{
			first_in_time_step = parent_module->get_time() - last_in_time;
		}

	}
	else
	{
		sc_dt::uint64 ict = current_time.value();
		sc_dt::uint64 its = in_time_step.value();
		in_index_offset = (ict + its - 1) / its;
		sc_dt::uint64 rem_time = in_index_offset * its - ict;
		first_in_time_step = rem_time * sc_core::sc_get_time_resolution();

		if (in_index_offset >= number_of_in_values)
		{
			std::ostringstream str;
			str
					<< "The current timestep becomes zero due the time distance between "
						"the time reached by the last execution has been reached the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name() << " ltf time since activation: "
					<< current_time << " module timestep: " << module_time_step;
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	statep = &state;

	ct_in_delay = ct_delay;

	if (delay_buffer == NULL)
	{
		if (ct_delay > in_time_step)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the in timestep: "
					<< in_time_step
					<< " otherwise you have to specify the maximum delay by the method: "
					<< " set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}
	else
	{
		if (ct_delay > max_delay)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the maximum delay: "
					<< max_delay
					<< " set by the method: set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		for (unsigned i = 0; i < number_of_in_values; i++)
		{
			delay_buffer->store_value(input.get_time(i), input.read(i));
		}
	}

}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		sca_util::sca_vector<double>& state, const sca_tdf::sca_de::sca_in<
				double>& input, double k)
{
	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_nd calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(num, den);

	time_interval = parent_module->get_timestep();

	intype = IN_SC_PORT;
	in_sc_port = &input;
	number_of_in_values = input.get_rate();
	in_time_step = time_interval / number_of_in_values;
	scale_factor = k;
	calculate_to_end = true;

	sc_dt::int64 ccnt = **(parent_module->call_counter);
	if (ccnt != module_activations)
	{
		current_time = sc_core::SC_ZERO_TIME;
		module_activations = ccnt;
		module_time_step = parent_module->get_timestep();
		in_index_offset = 0;

		if (ccnt == 0)
		{
			last_time = sc_core::SC_ZERO_TIME;
			last_in_time = sc_core::SC_ZERO_TIME - in_time_step;
			first_in_time_step = in_time_step;
		}
		else
		{
			first_in_time_step = parent_module->get_time() - last_in_time;
		}
	}
	else
	{
		sc_dt::uint64 ict = current_time.value();
		sc_dt::uint64 its = in_time_step.value();
		in_index_offset = (ict + its - 1) / its;
		sc_dt::uint64 rem_time = in_index_offset * its - ict;
		first_in_time_step = rem_time * sc_core::sc_get_time_resolution();

		if (in_index_offset >= number_of_in_values)
		{
			std::ostringstream str;
			str
					<< "The current timestep becomes zero due the time distance between "
						"the time reached by the last execution has been reached the time of the "
						"current module activation (see LRM) for: "
					<< ltf_object->name();
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	statep = &state;

	ct_in_delay = ct_delay;

	if (delay_buffer == NULL)
	{
		if (ct_delay > in_time_step)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the in timestep: "
					<< in_time_step
					<< " otherwise you have to specify the maximum delay by the method: "
					<< " set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}
	else
	{
		if (ct_delay > max_delay)
		{
			std::ostringstream str;
			str << "The parameter ct_delay (" << ct_delay << ") of: "
					<< ltf_object->name()
					<< " must be smaller or equal than the maximum delay: "
					<< max_delay
					<< " set by the method: set_max_ct_delay (see LRM for details)";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		for (unsigned long i = 0; i < number_of_in_values; i++)
		{
			double value;
			value
					= const_cast<sca_tdf::sca_de::sca_in<double>*> (in_sc_port)->read(
							i);
			delay_buffer->store_value(input.get_time(i), value);
		}
	}

}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		double input, double k, sca_core::sca_time tstep)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_nd(num, den, ct_delay, s_intern, input, k, tstep);
}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		const sca_util::sca_vector<double>& input, double k,
		sca_core::sca_time tstep)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_nd(num, den, ct_in_delay, s_intern, input, k, tstep);
}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		const sca_tdf::sca_in<double>& input, double k)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_nd(num, den, ct_delay, s_intern, input, k);
}

void sca_ct_ltf_nd_proxy::register_nd(const sca_util::sca_vector<double>& num,
		const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
		const sca_tdf::sca_de::sca_in<double>& input, double k)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_nd(num, den, ct_delay, s_intern, input, k);
}

inline double sca_ct_ltf_nd_proxy::get_in_value_by_index(unsigned long k)
{
	switch (intype)
	{
	case IN_SCALAR:
		return in_scalar;
	case IN_VECTOR:
		return (*in_vector)[k];
	case IN_SCA_PORT:
		return in_sca_port->read(k);
	case IN_SC_PORT:
		return const_cast<sca_tdf::sca_de::sca_in<double>*> (in_sc_port)->read(
				k);
	default:
		SC_REPORT_ERROR("SystemC-AMS","Internal not possible error");
	}
	return 0.0;
}

inline void sca_ct_ltf_nd_proxy::write_out_value_by_index(double val,
		unsigned long k)
{
	switch (outtype)
	{
	case OUT_SCALAR:
		out_scalar = val;
		return;
	case OUT_VECTOR:
		(*out_vector)[k] = val;
		return;
	case OUT_SCA_PORT:
		out_sca_port->write(val, k);
		return;
	case OUT_SC_PORT:
		(out_sc_port)->write(val, k);
		return;
	default:
		SC_REPORT_ERROR("SystemC-AMS","Internal not possible error");
	}
	return;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


double sca_ct_ltf_nd_proxy::convert_to_double()
{
	if (!pending_calculation)
	{
		std::ostringstream str;
		str << "The the ltf_nd proxy object is may assigned twice for: "
				<< ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = false;

	int init_mode = 0;
	double h = first_in_time_step.to_seconds();

	if (first_step)
	{
		init_mode = 0; //create datastructure, equation system, first step euler
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;
	}
	else if (reinit_request)
	{
		init_mode = 1; //rebuilt equations, first step euler
		time_step = time_interval;
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;
	}
	else if (fabs(h - last_h) > 1e-15)
	{
		last_h = h;
		initialize_equation_system(2, h);
	}

	double scalar_out, in_val;
	if (delay_buffer == NULL)
	{
		if (ct_in_delay == sc_core::SC_ZERO_TIME)
		{
			in_last = get_in_value_by_index(in_index_offset);
			last_in_time += first_in_time_step;

			scalar_out = calculate(in_last);
		}
		else
		{
			if(first_step) //if value before time zero
			{
				in_val=0.0;
			}
			else
			{
				in_val = (get_in_value_by_index(in_index_offset) - in_last)
					/ (first_in_time_step.to_seconds())
					* (first_in_time_step.to_seconds()
							- ct_in_delay.to_seconds()) + in_last;
			}

			scalar_out = calculate(in_val);

			h = ct_in_delay.to_seconds();

			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			in_last = get_in_value_by_index(in_index_offset);
			calculate(in_last);

		}

		h = in_time_step.to_seconds();

		if (fabs(h - last_h) > 1e-15)
		{
			last_h = h;
			initialize_equation_system(2, h);
		}

		for (unsigned long i = in_index_offset + 1; i < number_of_in_values; i++)
		{
			in_last = get_in_value_by_index(i);
			calculate(in_last);
			last_in_time += in_time_step;
		}

		first_step = false;
	}
	else //delay_buffer!=NULL
	{
		first_step = false;

		sca_core::sca_time next_in_time = last_in_time + first_in_time_step;
		if (ct_in_delay < next_in_time)
			next_in_time -= ct_in_delay;
		else
		{
			last_in_time += first_in_time_step;
			delay_buffer->set_time_reached(last_in_time);
			return 0.0;
		}

		sca_core::sca_time atime;
		double value;

		//read values until required time
		while (!delay_buffer->get_next_value_after(atime, value, last_in_time)
				&& atime < next_in_time)
		{
			h = (atime - last_in_time).to_seconds();

			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			calculate(value);
			last_in_time = atime;
		}

		h = (next_in_time - last_in_time).to_seconds();
		if (fabs(h - last_h) > 1e-15)
		{
			last_h = h;
			initialize_equation_system(2, h);
		}

		if(delay_buffer->get_value(next_in_time,value))
		{
			SC_REPORT_ERROR("SystemC-AMS","Internal error due a Bug");
		}
		scalar_out = calculate(value);

		last_in_time = next_in_time;
		delay_buffer->set_time_reached(last_in_time);
	}


	return scalar_out;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline void sca_ct_ltf_nd_proxy::calculate_timeinterval(
		unsigned long& current_in_index, long& current_out_index,
		long number_of_out_values, sca_core::sca_time& next_in_time,
		sca_core::sca_time& next_out_time,
		sca_core::sca_time& next_in_time_step,
		sca_core::sca_time& out_time_step)
{
	if (!pending_calculation)
	{
		std::ostringstream str;
		str << "The the ltf_nd proxy object is may assigned twice for: "
				<< ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = false;

	int init_mode = 0;
	double h = first_in_time_step.to_seconds();

	if (first_step)
	{
		first_step = false;
		init_mode = 0; //create datastructure, equation system, first step euler
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;
	}
	else if (reinit_request)
	{
		init_mode = 1; //rebulit equations, first step euler
		time_step = time_interval;
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;
	}

#ifdef SCA_DEBUG_LTF_ND
	std::cout << "---------------------------------------------------" << std::endl;
#endif

	if (delay_buffer == NULL)
	{
		//we calculate until all in values are consumed
		while (current_in_index < number_of_in_values)
		{
			//in and out at the same timepoint (error smaller than 1 resolution time
			//step due timestep calculation may introduces those error
			if (sc_dt::sc_abs(sc_dt::int64((next_in_time.value()
					- next_out_time.value()))) <= 1)
			{

				in_last = get_in_value_by_index(current_in_index);
				current_in_index++;
				last_in_time = next_in_time;
				next_in_time += next_in_time_step;
				next_in_time_step = in_time_step;

				if (number_of_out_values > current_out_index)
				{
					sca_core::sca_time dt = next_out_time - last_time;
					h
							= (dt == sc_core::SC_ZERO_TIME) ? in_time_step.to_seconds()
									: dt.to_seconds();

					if (fabs(h - last_h) > 1e-15)
					{
						last_h = h;
						initialize_equation_system(2, h);
					}

					double outp;
					outp = calculate(in_last);

					write_out_value_by_index(outp, current_out_index);
					current_out_index++;
					next_out_time += out_time_step;
					last_time += dt;
				}

			}
			//next time in value only -> ignore result
			else if (next_in_time < next_out_time)
			{
				sca_core::sca_time dt = next_in_time - last_time;
				h = (dt == sc_core::SC_ZERO_TIME) ? in_time_step.to_seconds()
						: dt.to_seconds();

				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				//calculate ltf_nd for in_value - out not used
				in_last = get_in_value_by_index(current_in_index);
				calculate(in_last);

				current_in_index++;
				last_time += dt;
				last_in_time = next_in_time;
				next_in_time += next_in_time_step;
				next_in_time_step = in_time_step;
			}
			else //next time calculate out value -> interpolate in
			{
				sca_core::sca_time dt = next_out_time - last_time;
				h = (dt == sc_core::SC_ZERO_TIME) ? in_time_step.to_seconds()
						: dt.to_seconds();
				last_time += dt;

				//interpolation TODO may higher order??
				double inp;
				inp = (get_in_value_by_index(current_in_index) - in_last)
						/ (next_in_time_step.to_seconds())
						* (next_out_time - last_in_time).to_seconds() + in_last;

				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				double outp = calculate(inp);
				write_out_value_by_index(outp, current_out_index);

				current_out_index++;
				next_out_time += out_time_step;

			}
#ifdef SCA_DEBUG_LTF_ND
			std::cout << "ct: " << last_time << "  not: " << next_out_time
			<< " nit: " << next_in_time << " coi: "
			<< current_out_index << " cii: " << current_in_index
			<< " last_in_time: " << last_in_time << " h: " << h << std::endl;
#endif
		} //while(current_in_index < number_of_in_values)
	}
	else //delay_buffer!=NULL
	{
		long outcnt = 0;
		while(next_out_time<=ct_in_delay)
		{
			h = (next_out_time - last_time).to_seconds();
			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			double value_in=0.0;
			if(next_out_time==ct_in_delay)
			{
				delay_buffer->get_value(sc_core::SC_ZERO_TIME,value_in);
			}
			double outp = calculate(value_in);

			write_out_value_by_index(outp, current_out_index);
			current_out_index++;
			last_time=next_out_time;

			delay_buffer->set_time_reached(next_out_time);

			next_out_time += out_time_step;
			outcnt++;

			if(outcnt>=number_of_out_values) break;
		}

		sca_core::sca_time last_delay_time = last_time-ct_in_delay;

		while (outcnt < number_of_out_values)
		{
			double value;
			sca_core::sca_time ntime;

			if((!delay_buffer->get_next_value_after(ntime, value, last_delay_time)) &&
			   (ntime < (next_out_time-ct_in_delay)))
			{   //there are input values before next_out_itme
				h = (ntime+ct_in_delay - last_time).to_seconds();
				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				last_delay_time=ntime;
				calculate(value);
				last_time = ntime+ct_in_delay;
			}
			else
			{
				if (ntime != (next_out_time-ct_in_delay))
				{
					if(delay_buffer->get_value(next_out_time-ct_in_delay,value))
					{
						SC_REPORT_ERROR("SystemC-AMS","Internal Error due a Bug");
					}
				}

				last_delay_time=next_out_time-ct_in_delay;

				h = (next_out_time - last_time).to_seconds();
				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				double outp = calculate(value);

				write_out_value_by_index(outp, current_out_index);
				current_out_index++;
				last_time = next_out_time;
				delay_buffer->set_time_reached(next_out_time);
				next_out_time += out_time_step;
				outcnt++;
			}
		} //while (outcnt < number_of_out_values)

	} //else delay_buffer!=NULL
}

void sca_ct_ltf_nd_proxy::convert_to_sca_port(sca_tdf::sca_out<double>& port)
{

	sca_core::sca_time out_time_step = port.get_timestep();
	sca_core::sca_time next_out_time = port.get_time();

	outtype = OUT_SCA_PORT;
	out_sca_port = &port;

	unsigned long current_in_index = in_index_offset;

	//calculate out index offset if the calculation does not start with
	//the module start time
	long current_out_index = 0;
	sc_dt::int64 int_next_out_time = next_out_time.value();
	sc_dt::int64 int_last_time = last_in_time.value();
	if (int_next_out_time < int_last_time)
	{
		current_out_index = (int_last_time - int_next_out_time
				+ out_time_step.value() - 1) / out_time_step.value();
		next_out_time += out_time_step * current_out_index;
	}

	sca_core::sca_time next_in_time = last_in_time + first_in_time_step;
	sca_core::sca_time next_in_time_step = first_in_time_step;

	//enforce causality if out rate > in rate
	if (calculate_to_end)
	{
		sca_core::sca_time out_end = out_time_step * (port.get_rate() - 1);
		sca_core::sca_time in_end =
				(number_of_in_values - current_in_index - 1) * in_time_step
						+ ct_in_delay;

		next_in_time += ct_in_delay;
		last_in_time += ct_in_delay;

#ifdef SCA_DEBUG_LTF_ND
		std::cout<< " oend; " << out_end << " iend: " << in_end << std::endl;
#endif
		if (out_end > in_end)
		{
			sca_core::sca_time in_causal_delay;
			in_causal_delay = out_end - in_end + ct_in_delay;

			std::ostringstream str;
			str
					<< "The input time intervall ends before the requested output time "
					<< "intervall for: " << ltf_object->name()
					<< " to achieve causality set the parameter ct_delay at least to : "
					<< in_causal_delay << " for details see LRM";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		calculate_to_end = false;
	}

	calculate_timeinterval(current_in_index, current_out_index,
			port.get_rate(), next_in_time, next_out_time, next_in_time_step,
			out_time_step);

	//the register_nd functions are using the non delayed time to calculate
	//the remaining module timestep
	last_in_time -= ct_in_delay;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void sca_ct_ltf_nd_proxy::convert_to_sc_port(
		sca_tdf::sca_de::sca_out<double>& port)
{

	sca_core::sca_time out_time_step = port.get_timestep();
	sca_core::sca_time next_out_time = port.get_time();

	outtype = OUT_SC_PORT;
	out_sc_port = &port;

	unsigned long current_in_index = in_index_offset;

	//calculate out index offset if the calculation does not start with
	//the module start time
	long current_out_index = 0;
	sc_dt::int64 int_next_out_time = next_out_time.value();
	sc_dt::int64 int_last_time = last_in_time.value();
	if (int_next_out_time < int_last_time)
	{
		current_out_index = (int_last_time - int_next_out_time
				+ out_time_step.value() - 1) / out_time_step.value();
		next_out_time += out_time_step * current_out_index;
	}

	sca_core::sca_time next_in_time = last_in_time + first_in_time_step;
	sca_core::sca_time next_in_time_step = first_in_time_step;

	//enforce causality if out rate > in rate
	if (calculate_to_end)
	{
		sca_core::sca_time out_end = out_time_step * (port.get_rate() - 1);
		sca_core::sca_time in_end =
				(number_of_in_values - current_in_index - 1) * in_time_step
						+ ct_in_delay;

		next_in_time += ct_in_delay;
		last_in_time += ct_in_delay;

#ifdef SCA_DEBUG_LTF_ND
		std::cout<< " oend; " << out_end << " iend: " << in_end << std::endl;
#endif
		if (out_end > in_end)
		{
			sc_core::sc_time in_causal_delay;
			in_causal_delay = out_end - in_end + ct_in_delay;

			std::ostringstream str;
			str
					<< "The input time intervall ends before the requested output time "
					<< "intervall for: " << ltf_object->name()
					<< " to achieve causality set the parameter ct_delay at least to : "
					<< in_causal_delay << " for details see LRM";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		calculate_to_end = false;
	}

	calculate_timeinterval(current_in_index, current_out_index,
			port.get_rate(), next_in_time, next_out_time, next_in_time_step,
			out_time_step);

	//the register_nd functions are using the non delayed time to calculate
	//the remaining module timestep
	last_in_time -= ct_in_delay;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void sca_ct_ltf_nd_proxy::convert_to_vector(sca_util::sca_vector<double>& vec,
		unsigned long nsamples)
{
	unsigned long number_of_out_samples = number_of_in_values;

	outtype = OUT_VECTOR;
	out_vector = &vec;

	unsigned long current_in_index = in_index_offset;
	long current_out_index = 0;

	if (nsamples != 0)
	{
		if (vec.length() == 0)
		{
			number_of_out_samples = nsamples;
			vec.resize(nsamples);
		}
		else
		{
			number_of_out_samples = vec.length();
		}
	}

	sca_core::sca_time out_time_step = time_interval / number_of_out_samples;

	sca_core::sca_time next_in_time = last_in_time + first_in_time_step;
	sca_core::sca_time next_in_time_step = first_in_time_step;

	sca_core::sca_time next_out_time = next_in_time;

	next_in_time += ct_in_delay;
	last_in_time += ct_in_delay;

	//if required introduce causality delay
	if (number_of_out_samples > number_of_in_values)
	{
		sc_core::sc_time in_causal_delay;
		in_causal_delay = (number_of_out_samples - number_of_in_values)
				* out_time_step;

		if (in_causal_delay > ct_in_delay)
		{
			std::ostringstream str;
			str
					<< "The input time intervall ends before the requested output time "
					<< "intervall for: " << ltf_object->name()
					<< " to achieve causality set the parameter ct_delay at least to : "
					<< in_causal_delay << " for details see LRM";
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}
	}

	calculate_timeinterval(current_in_index, current_out_index,
			number_of_out_samples, next_in_time, next_out_time,
			next_in_time_step, out_time_step);

	//the register_nd functions are using the non delayed time to calculate
	//the remaining module timestep
	last_in_time -= ct_in_delay;

}

}
}
