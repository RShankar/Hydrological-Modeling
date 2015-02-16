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

 sca_tdf_ct_ltf_nd_proxy.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 20.10.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_ct_ltf_nd_proxy.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_TDF_CT_LTF_ND_PROXY_H_
#define SCA_TDF_CT_LTF_ND_PROXY_H_

#include "systemc-ams"
#include "scams/predefined_moc/tdf/sca_tdf_ct_proxy.h"
#include "scams/impl/predefined_moc/tdf/sca_ct_delay_buffer.h"

class sca_solv_data;

namespace sca_tdf
{
namespace sca_implementation
{
class sca_ct_ltf_nd_proxy: public sca_tdf::sca_ct_proxy
{
public:

	void set_max_delay(sca_core::sca_time mdelay);


	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			double input,
			double k,
			sca_core::sca_time tstep);


	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_util::sca_vector<double>& input,
			double k,
			sca_core::sca_time tstep);


	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_in<double>& input,
			double k);

	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k);

	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			double input, double k,
			sca_core::sca_time tstep);

	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_util::sca_vector<double>& input,
			double k,
			sca_core::sca_time tstep);

	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_in<double>& input,
			double k);

	void register_nd(
			const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k);

	sca_ct_ltf_nd_proxy(::sc_core::sc_object* ltf_object);

protected:

	//overloaded virtual methods
	double convert_to_double();
	void   convert_to_sca_port(sca_tdf::sca_out<double>&);
	void   convert_to_vector(sca_util::sca_vector<double>&, unsigned long nsamples);
	void   convert_to_sc_port(sca_tdf::sca_de::sca_out<double>& port);


	const sca_util::sca_vector<double>* num_old_ref;
	const sca_util::sca_vector<double>* den_old_ref;

	sca_util::sca_vector<double> num_old;
	sca_util::sca_vector<double> den_old;

	sca_util::sca_vector<double> s_intern;

	sca_core::sca_time current_time;
	sca_core::sca_time module_time_step;

	bool reinit_request;

	//allow overloadind for skipping test
	virtual bool coeff_changed(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den);

	sca_core::sca_time time_interval;

	sca_util::sca_vector<double>* statep;

	sca_tdf::sca_module* parent_module;
	sc_core::sc_object* ltf_object;

	void initialize();
	double calculate(double);

	//solver data
	bool first_step;
	double q_dn;

	sca_core::sca_time time_step;

	double* eq_in;

	//equation system Adx+Bx+q(t) for fractional and non-fractional part
	double *A, *B, *q;
	double *A2, *B2, *q2;
	sca_solv_data   *sdata, *sdata2;
	unsigned long memsize;
	unsigned long dn, nn;


	//references for fast access
	double* den_ltf;
	double* num_ltf;

	//pointer to memory for non fractional numerator coefficients
	double* num2_ltf;

	double* s;

	unsigned long den_size, num_size, state_size;

	unsigned long number_of_equations;
	unsigned long number_of_equations2;

	void setup_equation_system();
	void initialize_equation_system(int init_mode,double h);


	double last_h;
	double in_last;

	///variables for input handling

	unsigned long number_of_in_values;
	sca_core::sca_time in_time_step;
	sca_core::sca_time first_in_time_step;
	sca_core::sca_time last_in_time;
	sca_core::sca_time last_time;

	bool calculate_to_end;


	enum inval_type
	{
		IN_SCALAR,
		IN_VECTOR,
		IN_SCA_PORT,
		IN_SC_PORT
	};

	inval_type intype;

	double in_scalar;
	const sca_util::sca_vector<double>* in_vector;
	const sca_tdf::sca_in<double>*      in_sca_port;
	const sca_tdf::sca_de::sca_in<double>* in_sc_port;

	enum outval_type
	{
		OUT_SCALAR,
		OUT_VECTOR,
		OUT_SCA_PORT,
		OUT_SC_PORT
	};

	outval_type outtype;

	double out_scalar;
	sca_util::sca_vector<double>* out_vector;
	sca_tdf::sca_out<double>*      out_sca_port;
	sca_tdf::sca_de::sca_out<double>* out_sc_port;

	bool pending_calculation;

	unsigned long in_index_offset;

	sc_dt::int64 module_activations;

	double scale_factor;

	double get_in_value_by_index(unsigned long k);
	void write_out_value_by_index(double val, unsigned long k);
	void calculate_timeinterval(
			unsigned long& current_in_index,
			         long& current_out_index,
			         long number_of_out_sample,
			sca_core::sca_time& next_in_time,
			sca_core::sca_time& next_out_time,
			sca_core::sca_time& next_in_time_step,
			sca_core::sca_time& out_time_step);

	bool causal_warning_reported;

	sca_core::sca_time ct_in_delay;
	sca_core::sca_time max_delay;

	sca_tdf::sca_implementation::sca_ct_delay_buffer_double* delay_buffer;

	bool dc_init;

};
}
}

#endif /* SCA_TDF_CT_LTF_ND_PROXY_H_ */
