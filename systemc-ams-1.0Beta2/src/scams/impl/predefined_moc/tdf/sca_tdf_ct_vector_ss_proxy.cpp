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

 sca_tdf_ltf_ss_proxy.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 27.12.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_ct_vector_ss_proxy.cpp 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/impl/predefined_moc/tdf/sca_tdf_ct_vector_ss_proxy.h"
#include "scams/impl/solver/utilities/sparse_library/linear_analog_solver.h"

namespace sca_tdf
{
namespace sca_implementation
{

sca_ct_vector_ss_proxy::sca_ct_vector_ss_proxy(::sc_core::sc_object* object)
{
	a_old_ref = NULL;
	b_old_ref = NULL;
	c_old_ref = NULL;
	d_old_ref = NULL;

	reinit_request = true;

	q_ss = NULL;
	a_ss = NULL;
	b_ss = NULL;
	memsize = 0;

	first_step = true;
	time_step = sc_core::SC_ZERO_TIME;
	time_interval = sc_core::SC_ZERO_TIME;
	sdata = NULL;
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
		str << "\n\tA " << "sca_tdf::sca_ltf_ss"
				<< " must be used only in the context of a "
				<< "sca_tdf::sca_module (SCA_TDF_MODULE). " << std::endl;

		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	delay_buffer = NULL;
	dc_init=true;
}

//////////////////////////////////////////////////////////////////

void sca_ct_vector_ss_proxy::set_max_delay(sca_core::sca_time mdelay)
{
	if (delay_buffer != NULL)
	{
		delete delay_buffer;
		delay_buffer = NULL;
	}

	max_delay = mdelay;
	if (mdelay != sc_core::SC_ZERO_TIME)
	{
		delay_buffer = new sca_ct_delay_buffer_vector;
		delay_buffer->set_max_delay(mdelay);
	}
}

inline bool sca_ct_vector_ss_proxy::matrix_changed(const sca_util::sca_matrix<
		double>& matrix, sca_util::sca_matrix<double>& matrix_old,
		const sca_util::sca_matrix<double>*& matrix_old_ref)
{
	//was there a write access or has the object changed since the last call
	if (matrix.get_access_flag() || (&matrix_old != matrix_old_ref))
	{
		//has the dimension changed since the last call
		if ((matrix.n_cols() != matrix_old.n_cols()) || (matrix.n_rows()
				!= matrix_old.n_rows()))
		{
			return true;
		}

		//has a value changed since the last call
		for (unsigned int i = 0; i < matrix.n_cols(); i++)
			for (unsigned int j = 0; j < matrix.n_rows(); j++)
			{
				if (matrix(j, i) != matrix_old(j, i))
					return true;
			}

	}

	return false;
}

//quick check for coefficient change
inline bool sca_ct_vector_ss_proxy::coeff_changed(const sca_util::sca_matrix<
		double>& a, const sca_util::sca_matrix<double>& b,
		const sca_util::sca_matrix<double>& c, const sca_util::sca_matrix<
				double>& d)
{
	if (matrix_changed(a, a_old, a_old_ref) || matrix_changed(b, b_old,
			b_old_ref) || matrix_changed(c, c_old, c_old_ref)
			|| matrix_changed(d, d_old, d_old_ref))
	{
		a_old = a;
		b_old = b;
		c_old = c;
		d_old = d;

		a_old_ref = &a;
		b_old_ref = &b;
		c_old_ref = &c;
		d_old_ref = &d;

		const_cast<sca_util::sca_matrix<double>*> (&a)->reset_access_flag();
		const_cast<sca_util::sca_matrix<double>*> (&b)->reset_access_flag();
		const_cast<sca_util::sca_matrix<double>*> (&c)->reset_access_flag();
		const_cast<sca_util::sca_matrix<double>*> (&d)->reset_access_flag();

		return true;
	}

	return false;

}

inline void sca_ct_vector_ss_proxy::setup_equation_system()
{
	number_of_equations = a_old.n_cols();

	//preparing memory for holding the two equation systems
	unsigned long new_memsize;
	new_memsize = 2 * number_of_equations * number_of_equations
			+ number_of_equations;

	if (new_memsize != memsize)
	{
		if (q_ss != NULL)
			delete[] q_ss;
		q_ss = new double[new_memsize];

		if (q_ss == NULL)
		{
			std::ostringstream str;
			str << "Can't allocate enough memory for sca_ltf_ss:  "
					<< ltf_object->name() << " required size: " << memsize
					<< " * " << sizeof(double) << " Bytes" << std::endl;
			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
		}

		memsize = new_memsize;
	}

	a_ss = q_ss + number_of_equations;
	b_ss = a_ss + number_of_equations * number_of_equations;
	memset((void*) (q_ss), 0, new_memsize * sizeof(double)); //set all to zero


	// 0 = ds - As
	double* b_ref = b_ss;
	for (unsigned long m = 0; m < b_old.n_rows(); ++m)
	{
		a_ss[m * (number_of_equations + 1)] = 1.0; //ds

		double* a = a_old.get_flat() + m * a_old.n_cols(); // row m
		for (unsigned long n = 0; n < a_old.n_cols(); ++n, ++b_ref)
		{
			*b_ref = -a[n]; // -a[n,m]*s
		}
	}

}

inline void sca_ct_vector_ss_proxy::initialize_equation_system(int init_mode,
		double h)
{
	if (number_of_equations > 0)
	{
		int err = ana_reinit(a_ss, b_ss, number_of_equations, h, &sdata,
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

inline void sca_ct_vector_ss_proxy::initialize()
{
	unsigned long ax, ay, bx, by, cx, cy, dx, dy;

	//if internal S is used it will be reset for
	//every re-initialization
	s_intern.remove();

	ax = a_old.n_cols();
	ay = a_old.n_rows();
	bx = b_old.n_cols();
	by = b_old.n_rows();
	cx = c_old.n_cols();
	cy = c_old.n_rows();
	dy = d_old.n_rows();
	dx = d_old.n_cols();

	if (ay != by)
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tDimension mismatch with matrices A and B."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}
	if ((ax != cx) & (cy > 0))
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tDimension mismatch with matrices A and C."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	if (ax != ay)
	{
		std::ostringstream str;
		str << ltf_object->name() << ":\n\tMatrix A must be square."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	if ((cy != dy) & (cx > 0))
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tDimension mismatch with matrices C and D."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	if ((bx != dx) & (by > 0))
	{
		std::ostringstream str;
		str << ltf_object->name()
				<< ":\n\tDimension mismatch with matrices B and D."
				<< std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

	state_size = ax;
	unsigned long s_size = 0;

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
				<< s_size
				<< ") does not match the number of equations (e.g. dimension of square matrix A: "
				<< state_size << ")." << std::endl;
		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
	}

}

inline sca_util::sca_vector<double> sca_ct_vector_ss_proxy::calculate(
		sca_util::sca_vector<double>& invalue)
{

	s = statep->get_flat();



		double* b = b_old.get_flat();
		unsigned long by = b_old.n_rows();
		for (unsigned n = 0; n < by; ++n)
		{
			q_ss[n] = 0.0;
			for (unsigned m = 0; m < b_old.n_cols(); ++m)
			{
				//q_ss[n] -= b_old(m,n) * invalue[m];  // 0 = ds - As - Bx
				q_ss[n] -= b[m * by + n] * invalue[m]; // 0 = ds - As - Bx
			}
		}

		if (number_of_equations > 0) //solve fractional part
		{
			if(!dc_init) ana_solv(q_ss, s, sdata);
			dc_init=false;
		}



	unsigned long nout = (c_old.n_rows() == 0) ? d_old.n_rows()
			: c_old.n_rows();
	sca_util::sca_vector<double> tmp_out(nout);
	double* tmp_out_ref = tmp_out.get_flat();
	double* c_ref = c_old.get_flat();
	double* d_ref = d_old.get_flat();
	double* in_ref = invalue.get_flat();
	unsigned long cx = c_old.n_cols();
	unsigned long dx = d_old.n_cols();
	unsigned long dy = d_old.n_rows();

	for (unsigned long n = 0; n < dy; ++n, ++tmp_out_ref)
	{
		(*tmp_out_ref) = 0.0;
		for (unsigned long m = 0; m < cx; ++m)
		{
			//*tmp_out_ref+=c_old(m,n) * s[m];    // y = C*s
			*tmp_out_ref += c_ref[m * dy + n] * s[m]; // y = C*s
		}

		for (unsigned long m = 0; m < dx; ++m)
		{
			//*tmp_out_ref+=d_old(m,n) * in_ref[m];    // y = C*s + D*x
			*tmp_out_ref += d_ref[m * dy + n] * in_ref[m]; // y = C*s + D*x
		}
	}

	return tmp_out;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, sca_util::sca_vector<double>& state,
		const sca_util::sca_vector<double>& x, sca_core::sca_time tstep)
{
	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_ss calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(a, b, c, d);

	sc_dt::int64 ccnt = **(parent_module->call_counter);
	if (ccnt != module_activations)
	{
		current_time = sc_core::SC_ZERO_TIME; //time from start of current activation
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

		if (current_time + time_interval > module_time_step)
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
		if (ccnt == 0) //first activation
		{
			last_in_time = sc_core::SC_ZERO_TIME - in_time_step;
		}
		else //first activation
		{
			//the first timestep of an invocation includes the remaining time of the last
			//module invocation
			first_in_time_step = parent_module->get_time() - last_in_time;
		}
		module_activations = ccnt;
	}

	intype = IN_SCALAR;
	in_scalar = x;
	number_of_in_values = 1;
	in_index_offset = 0;

	statep = &state;

	//delay of input signal
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

		//store in value in buffer
		delay_buffer->store_value(parent_module->get_time() + current_time, x);
	}

	current_time += time_interval;
	in_last.resize(b.n_cols()?b.n_cols():d.n_cols());
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, sca_util::sca_vector<double>& state,
		const sca_util::sca_matrix<double>& x, sca_core::sca_time tstep)
{

	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_ss calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(a, b, c, d);

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

	number_of_in_values = x.n_cols();
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
	in_vector = &x;

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
			sca_util::sca_vector<double> tmp(x.n_rows());
			for (unsigned long j = 0; j < tmp.length(); j++)
				tmp(j) = x(j, i);
			delay_buffer->store_value(current_time + i * in_time_step, tmp);
		}
	}

	in_last.resize(b.n_cols()?b.n_cols():d.n_cols());
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, sca_util::sca_vector<double>& state,
		const sca_tdf::sca_in<sca_util::sca_vector<double> >& x)
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

	reinit_request = coeff_changed(a, b, c, d);

	time_interval = parent_module->get_timestep();

	intype = IN_SCA_PORT;
	in_sca_port = &x;
	number_of_in_values = x.get_rate();
	in_time_step = time_interval / number_of_in_values;
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
			delay_buffer->store_value(x.get_time(i), x.read(i));
		}
	}

	in_last.resize(b.n_cols()?b.n_cols():d.n_cols());

}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, sca_util::sca_vector<double>& state,
		const sca_tdf::sca_de::sca_in<sca_util::sca_vector<double> >& x)
{
	if (pending_calculation)
	{
		std::ostringstream str;
		str
				<< "The previous ltf_ss calculation was performed without assigning"
					" the result for: " << ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = true;

	reinit_request = coeff_changed(a, b, c, d);

	time_interval = parent_module->get_timestep();

	intype = IN_SC_PORT;
	in_sc_port = &x;
	number_of_in_values = x.get_rate();
	in_time_step = time_interval / number_of_in_values;
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
			sca_util::sca_vector<double> value;
			value = const_cast<sca_tdf::sca_de::sca_in<sca_util::sca_vector<
					double> >*> (&x)->read(i);
			delay_buffer->store_value(x.get_time(i), value);
		}
	}

	in_last.resize(b.n_cols()?b.n_cols():d.n_cols());
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, const sca_util::sca_vector<double>& x,
		sca_core::sca_time tstep)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_ss(a, b, c, d, ct_delay, s_intern, x, tstep);
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, const sca_util::sca_matrix<double>& x,
		sca_core::sca_time tstep)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_ss(a, b, c, d, ct_in_delay, s_intern, x, tstep);
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, const sca_tdf::sca_in<
				sca_util::sca_vector<double> >& x)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_ss(a, b, c, d, ct_delay, s_intern, x);
}

void sca_ct_vector_ss_proxy::register_ss(const sca_util::sca_matrix<double>& a,
		const sca_util::sca_matrix<double>& b, const sca_util::sca_matrix<
				double>& c, const sca_util::sca_matrix<double>& d,
		sca_core::sca_time ct_delay, const sca_tdf::sca_de::sca_in<
				sca_util::sca_vector<double> >& x)
{
	if (s_intern.length() != state_size)
		s_intern.resize(0);
	register_ss(a, b, c, d, ct_delay, s_intern, x);
}

inline sca_util::sca_vector<double> sca_ct_vector_ss_proxy::get_in_value_by_index(
		unsigned long k)
{
	switch (intype)
	{
	case IN_SCALAR:
		return in_scalar;
	case IN_VECTOR:
	{
		sca_util::sca_vector<double> tmp((*in_vector).n_rows());
		for (unsigned long i = 0; i < tmp.length(); i++)
			tmp[i] = (*in_vector)(i,k);
		return tmp;
	}
	case IN_SCA_PORT:
		return in_sca_port->read(k);
	case IN_SC_PORT:
		return const_cast<sca_tdf::sca_de::sca_in<sca_util::sca_vector<double> >*> (in_sc_port)->read(
				k);
	default:
		SC_REPORT_ERROR("SystemC-AMS","Internal not possible error");
	}
	return sca_util::sca_vector<double>();
}

inline void sca_ct_vector_ss_proxy::write_out_value_by_index(
		const sca_util::sca_vector<double>& val, unsigned long k)
{
	switch (outtype)
	{
	case OUT_SCALAR:
		out_scalar = val;
		return;
	case OUT_VECTOR:
		for (unsigned long i = 0; i < (*out_vector).n_rows(); i++)
			(*out_vector)(i, k) = val(i);
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


const sca_util::sca_vector<double>& sca_ct_vector_ss_proxy::convert_to_double_vector()
{
	if (!pending_calculation)
	{
		std::ostringstream str;
		str << "The the ltf_ss proxy object is may assigned twice for: "
				<< ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = false;

	//we have to calculate one output value

	int init_mode = 0;
	//first step is until current outtime - the next invalue minus delay
	double h = first_in_time_step.to_seconds();

	if (first_step)
	{
		init_mode = 0; //create datastructure, equation system, first step euler
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;

		//we propagate first value to negative time -> if we have a delay, the first value will
		//be seen at time zero
		in_last = get_in_value_by_index(0);
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

	//resize in_val vector to required size
	unsigned long nout=c_old.n_rows()?c_old.n_rows():d_old.n_rows();
	sca_util::sca_vector<double> in_val(nout);

	//if no delay buffer available
	if (delay_buffer == NULL)
	{
		//		std::cout << " first_in_time_step: " << first_in_time_step << " h: " << h << " last_h: "
		//				<< last_h << " current_time: " << current_time << std::endl;

		if (ct_in_delay == sc_core::SC_ZERO_TIME) //if no delay - very simple (less effort)
		{
			in_last = get_in_value_by_index(in_index_offset);
			last_in_time += first_in_time_step;

			out_scalar = calculate(in_last); //out value is at time from in value
			first_step = false;
		}
		else // if delay - more complicated (effort)
		{
			//get in value at current time
			sca_util::sca_vector<double> tmp = get_in_value_by_index(
					in_index_offset);

			//calculate delayed in value
			for (unsigned long i = 0; i < nout; i++) //calculate delayed signals
			{
				if (first_step) //the first value must be the initial value (before invalue)
				{
					in_val[i] = in_last[i];
				}
				else
				{
					in_val[i] = (tmp[i] - in_last[i])
							/ (first_in_time_step.to_seconds())
							* (first_in_time_step.to_seconds() - ct_in_delay.to_seconds()) + in_last[i];
				}
			}

			h = (first_in_time_step - ct_in_delay).to_seconds();
			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			out_scalar = calculate(in_val); //calculate outvalues for current time
			first_step = false;

			h = ct_in_delay.to_seconds(); //calculate states for current invalue

			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			in_last = tmp;
			tmp = calculate(in_last);
			last_in_time += first_in_time_step;
		}

		//There are further in timesteps available
		if (number_of_in_values > in_index_offset + 1)
		{
			h = in_time_step.to_seconds(); //calculate following timesteps for in values

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
		}
	}
	else //delay_buffer!=NULL

	{
		//we can read/calculate until the timestamp of the current sample - delay
		sca_core::sca_time next_in_time = last_in_time + first_in_time_step;

//		std::cout << " calculate out value for: " << next_in_time << std::endl;

		if (ct_in_delay >= next_in_time) //value is before time 0
		{
			last_in_time += first_in_time_step;
			delay_buffer->set_time_reached(last_in_time);

			out_scalar.resize(c_old.n_rows()?c_old.n_rows():d_old.n_rows());
			out_scalar = in_last; //we propagate first input

//			std::cout << "last_in_time:  " << last_in_time << std::endl;

			first_step = false;
			return out_scalar;
		}

		sca_core::sca_time atime;
		sca_core::sca_time delayed_in_time = next_in_time - ct_in_delay;
		sca_util::sca_vector<double> values;

		sca_core::sca_time read_until;
		if (last_in_time > ct_in_delay)
			read_until = last_in_time - ct_in_delay;
		else //we start the integrator timescale at ct_in_delay
		{
			read_until = sc_core::SC_ZERO_TIME;
			last_in_time = ct_in_delay;
		}

		//read values until required time
		while (!delay_buffer->get_next_value_after(atime, values, read_until)
				&& atime < delayed_in_time)
		{
			h = (atime - read_until).to_seconds();

//			std::cout << " calc for " << atime << " at: " << atime
//					+ ct_in_delay << " values1 " << values(1) << "  v2 "
//					<< values(2) << " lastt: " << read_until << " h: " << h
//					<< std::endl;

			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			calculate(values);


			read_until = atime;
			last_in_time = atime + ct_in_delay;
		}

		h = (next_in_time - last_in_time).to_seconds();
		if (fabs(h - last_h) > 1e-15)
		{
			last_h = h;
			initialize_equation_system(2, h);
		}

		if (delay_buffer->get_value(delayed_in_time, values))
		{
//			std::cout << "delay_in_time: " << delayed_in_time
//					<< " next_in_time: " << next_in_time << std::endl;

			SC_REPORT_ERROR("SystemC-AMS","Internal error due a Bug");
		}

		out_scalar = calculate(values);

//		std::cout << " calc at " << next_in_time << " values1 " << values(1)
//				<< "  v2 " << values(2) << " lastt: " << last_in_time
//				<< " out3 " << out_scalar(3) << " out4 " << out_scalar(4)
//				<< " h: " << h << std::endl;

		last_in_time = next_in_time;
		delay_buffer->set_time_reached(last_in_time);
	}

	first_step = false;
	return out_scalar;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline void sca_ct_vector_ss_proxy::calculate_timeinterval(
		unsigned long& current_in_index, long& current_out_index,
		long number_of_out_values, sca_core::sca_time& next_in_time,
		sca_core::sca_time& next_out_time,
		sca_core::sca_time& next_in_time_step,
		sca_core::sca_time& out_time_step)
{
	if (!pending_calculation)
	{
		std::ostringstream str;
		str << "The the ltf_ss proxy object is may assigned twice for: "
				<< ltf_object->name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	pending_calculation = false;

	//we have to calculate one output value

	int init_mode = 0;
	//first step is until current outtime - the next invalue minus delay
	double h = first_in_time_step.to_seconds();

	if (first_step)
	{
		init_mode = 0; //create datastructure, equation system, first step euler
		initialize();
		setup_equation_system();
		initialize_equation_system(init_mode, h);
		last_h = h;

		//we propagate the first value to negative time
		in_last=get_in_value_by_index(0);


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

	first_step = false;

#ifdef SCA_DEBUG_LTF_SS
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

					sca_util::sca_vector<double> outp;
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
				unsigned long dimy=c_old.n_rows()>0?c_old.n_rows():d_old.n_rows();
				sca_util::sca_vector<double> inp(dimy);
				sca_util::sca_vector<double> tmp = get_in_value_by_index(current_in_index);

				for (unsigned long i = 0; i < dimy; i++)
				{
					inp[i] = (tmp[i] - in_last[i])
							/ (next_in_time_step.to_seconds())
							* (next_out_time - last_in_time).to_seconds() + in_last[i];
				}

				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				sca_util::sca_vector<double> outp = calculate(inp);
				write_out_value_by_index(outp, current_out_index);

				current_out_index++;
				next_out_time += out_time_step;

			}
#ifdef SCA_DEBUG_LTF_SS
			std::cout << "ct: " << last_time << "  not: " << next_out_time
			<< " nit: " << next_in_time << " coi: "
			<< current_out_index << " cii: " << current_in_index
			<< " last_in_time: " << last_in_time << " h: " << h << std::endl;
#endif
		} //while(current_in_index < number_of_in_values)
	}
	else //delay_buffer!=NULL
	{
		unsigned long nin = (b_old.n_cols() == 0) ? d_old.n_cols()
				: b_old.n_cols();

		long outcnt = 0;
		while (next_out_time <= ct_in_delay)
		{
			h = (next_out_time - last_time).to_seconds();
			if (fabs(h - last_h) > 1e-15)
			{
				last_h = h;
				initialize_equation_system(2, h);
			}

			sca_util::sca_vector<double> value_in(nin);
			//if (next_out_time == ct_in_delay)
			//{
			//we propagate value at time zero at input
			delay_buffer->get_value(sc_core::SC_ZERO_TIME, value_in);
			//}
			sca_util::sca_vector<double> outp = calculate(value_in);

			write_out_value_by_index(outp, current_out_index);
			current_out_index++;
			last_time = next_out_time;

			delay_buffer->set_time_reached(next_out_time);

			next_out_time += out_time_step;
			outcnt++;

			if (outcnt >= number_of_out_values)
				break;
		}

		sca_core::sca_time last_delay_time = last_time - ct_in_delay;

		while (outcnt < number_of_out_values)
		{
			sca_util::sca_vector<double> values(nin);
			sca_core::sca_time ntime;

			if ((!delay_buffer->get_next_value_after(ntime, values,
					last_delay_time))
					&& (ntime < (next_out_time - ct_in_delay)))
			{ //there are input values before next_out_itme
				h = (ntime + ct_in_delay - last_time).to_seconds();
				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				last_delay_time = ntime;
				calculate(values);
				last_time = ntime + ct_in_delay;
			}
			else
			{
				if (ntime != (next_out_time - ct_in_delay))
				{
					if (delay_buffer->get_value(next_out_time - ct_in_delay,
							values))
					{
						SC_REPORT_ERROR("SystemC-AMS","Internal Error due a Bug");
					}
				}

				last_delay_time = next_out_time - ct_in_delay;

				h = (next_out_time - last_time).to_seconds();
				if (fabs(h - last_h) > 1e-15)
				{
					last_h = h;
					initialize_equation_system(2, h);
				}

				sca_util::sca_vector<double> outp = calculate(values);

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

void sca_ct_vector_ss_proxy::convert_to_sca_port(sca_tdf::sca_out<
		sca_util::sca_vector<double> >& port)
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

#ifdef SCA_DEBUG_LTF_SS
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

void sca_ct_vector_ss_proxy::convert_to_sc_port(sca_tdf::sca_de::sca_out<
		sca_util::sca_vector<double> >& port)
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

#ifdef SCA_DEBUG_LTF_SS
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

void sca_ct_vector_ss_proxy::convert_to_matrix(
		sca_util::sca_matrix<double>& vec, unsigned long nsamples)
{
	unsigned long number_of_out_samples = number_of_in_values;

	outtype = OUT_VECTOR;
	out_vector = &vec;

	unsigned long current_in_index = in_index_offset;
	long current_out_index = 0;

	unsigned long nout = (c_old.n_rows()) ? c_old.n_rows() : d_old.n_rows();

	if (nsamples == 0)
	{
		if (vec.n_cols() == 0)
		{
			vec.resize(nout, number_of_out_samples);
		}
		else
		{
			number_of_out_samples = vec.n_cols();
			vec.resize(nout, number_of_out_samples);
		}
	}
	else
	{
		vec.resize(nout, nsamples);
		number_of_out_samples=nsamples;
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
