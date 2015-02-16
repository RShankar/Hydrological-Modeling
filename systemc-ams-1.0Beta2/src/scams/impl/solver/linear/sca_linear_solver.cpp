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

 sca_linear_solver.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 10.11.2009

 SVN Version       :  $Revision: 1157 $
 SVN last checkin  :  $Date: 2011-02-09 23:08:11 +0100 (Wed, 09 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_linear_solver.cpp 1157 2011-02-09 22:08:11Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/impl/solver/linear/sca_linear_solver.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/core/sca_object_manager.h"
#include "scams/impl/synchronization/sca_synchronization_layer.h"
#include "scams/impl/solver/utilities/sparse_library/linear_analog_solver.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"

namespace sca_core
{
namespace sca_implementation
{

//////////////////////////////////////////////////////////////////////

sca_linear_solver::sca_linear_solver(std::vector<sca_module*>& mods, std::vector<sca_interface*>& chans) :
	sca_core::sca_implementation::sca_solver_base(sc_core::sc_gen_unique_name(
			"sca_linear_solver"), mods,chans)
{

#ifdef SCA_IMPLEMENTATION_DEBUG
	std::cout << get_name() << ": constructed." << std::endl;
#endif

	call_id=0;

	 //the solver supports ac-simulation
	 sca_ac_analysis::sca_implementation::sca_ac_domain_register_entity(
	 this,
	 NULL,
	 static_cast<sca_ac_analysis::sca_implementation::sca_add_ac_domain_eq_method> (&sca_linear_solver::ac_domain_eq_method),
	 static_cast<sca_ac_analysis::sca_implementation::sca_calc_add_eq_cons_method> (&sca_linear_solver::ac_add_eq_cons_method));


	 number_of_timesteps=0;
	 number_of_reinitializations=0;

}

//////////////////////////////////////////////////////////////////////

sca_linear_solver::~sca_linear_solver()
{
}

//////////////////////////////////////////////////////////////////////

void sca_linear_solver::initialize()
{

	//register solver to synchronization layer
	sca_core::sca_implementation::sca_get_curr_simcontext()->get_sca_object_manager()->get_synchronization_if()-> registrate_solver_instance(
			this);

#ifdef SCA_IMPLEMENTATION_DEBUG
	std::cout << "\t\t" << " linear solver instance initialized" << std::endl;
#endif
}

//////////////////////////////////////////

sc_core::sc_object* sca_linear_solver::get_object_of_equation(long eq_nr)
{
	if(eq_nr<0) return NULL;

	for(unsigned long i=0;i<associated_channel_list.size();i++)
	{
		sca_core::sca_implementation::sca_conservative_signal* sig;
		sig=dynamic_cast<sca_core::sca_implementation::sca_conservative_signal*>(associated_channel_list[i]);

		if(sig!=NULL)
		{
			if(sig->get_node_number()==eq_nr) return sig;
		}
	}

	for(unsigned long i=0;i<associated_module_list.size();i++)
	{
		sca_core::sca_implementation::sca_conservative_module* mod;
		mod=dynamic_cast<sca_core::sca_implementation::sca_conservative_module*>(associated_module_list[i]);

		if(mod!=NULL)
		{
			for(unsigned long j=0;j<mod->add_equations.size();j++)
			{
				if(mod->add_equations[j] == eq_nr) return mod;
			}
		}
	}

	return NULL;
}


//////////////////////////////////////////

void sca_linear_solver::init_eq_system()
{
	equation_if->reinit_equations();

	ac_equation_initialized = false;

	x.resize(A->n_cols());
	x_flat = x.get_flat();


	unsigned long& info_mask(
	            sca_core::sca_implementation::sca_get_curr_simcontext()->
	            get_information_mask());

	if (info_mask & (
                          sca_util::sca_info::sca_eln_solver.mask |
                          sca_util::sca_info::sca_lsf_solver.mask))
	{
	  if((info_mask & sca_util::sca_info::sca_eln_solver.mask) &&
	     ( dynamic_cast<sca_eln::sca_module*>(associated_module_list[0])!=NULL))
	    {
		std::ostringstream sta_str;
		sta_str << "\n\tELN solver instance: " << get_name()
                        << " (cluster " << cluster_id << ")" << std::endl << "\t\thas "
			<< x.length() << " equations for "
			<< associated_module_list.size() << " modules (e.g. "
			<< associated_module_list[0]->name() << ")," << std::endl << "\t\t"
			<< from_analog.size() << " inputs and " << to_analog.size()
			<< " outputs to other (TDF) SystemC-AMS domains,\n" << "\t\t"
			<< from_systemc.size() << " inputs and " << to_systemc.size()
			<< " outputs to SystemC de." << std::endl;
		sta_str << "\t\t" << get_current_period() << " initial time step"
				<< std::endl;

		SC_REPORT_INFO("SystemC-AMS", sta_str.str().c_str());
	    }


	   if((info_mask & sca_util::sca_info::sca_lsf_solver.mask) &&
	       (dynamic_cast<sca_lsf::sca_module*>(associated_module_list[0])!=NULL))
	   {
	       std::ostringstream sta_str;
	       sta_str << "\n\tLSF solver instance: " << get_name()
	               << " (cluster " << cluster_id << ")" << std::endl << "\t\thas "
	               << x.length() << " equations for "
	               << associated_module_list.size() << " modules (e.g. "
	               << associated_module_list[0]->name() << ")," << std::endl << "\t\t"
	               << from_analog.size() << " inputs and " << to_analog.size()
	               << " outputs to other (TDF) SystemC-AMS domains,\n" << "\t\t"
	               << from_systemc.size() << " inputs and " << to_systemc.size()
	               << " outputs to SystemC de." << std::endl;
	      sta_str << "\t\t" << get_current_period() << " initial time step"
	                                  << std::endl;

	                  SC_REPORT_INFO("SystemC-AMS", sta_str.str().c_str());
           }

	}

	dt = get_current_period().to_seconds();

	//std::cout << " A: " << std::endl << *A << std::endl << std::endl << " B : " << std::endl << *B << std::endl;

	int err;
	err = ana_init(A->get_flat(), B->get_flat(), A->n_cols(), dt,
			&internal_solver_data, 0);


	if (err != 0)
	{
		long row=-1, column=-1;
		ana_get_error_position(internal_solver_data,&row,&column);

		sc_core::sc_object* obj_row=get_object_of_equation(row);
		sc_core::sc_object* obj_column=get_object_of_equation(column);

		std::ostringstream str;
		str << "Initialization equation system failed in " << get_name()
				<< ": " << err << std::endl;
		str << "  the error is in the following net (max. 50): " << std::endl;
		for (unsigned long i = 0; i < associated_module_list.size(); ++i)
		{
			if (i >= 50)
				break;
			str << "\t" << associated_module_list[i]->name() << std::endl;
		}

		if((obj_row!=NULL)||(obj_column!=NULL))
		{
			str << std::endl;
			str << "The error is may  be near: " << std::endl << "\t\t";
			if(obj_row!=NULL)
			{
				str << obj_row->name();
				if(obj_column!=obj_row) str << "  and " << std::endl << "\t\t";
			}

			if(obj_column!=obj_row) str << obj_column->name();
			str << std::endl;
		}

		SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());

	}

	call_id++;
}

//////////////////////////////////////////

void sca_linear_solver::solve_eq_system()
{
	current_time = get_current_time().to_seconds();
	number_of_timesteps++;

	double new_dt = get_current_period().to_seconds();

	int init_flag = 0;

	if (ac_equation_initialized) //restore time domain equations
	{
		equation_if->reinit_equations();
		ac_equation_initialized = false;
	}

	if (fabs(new_dt - dt) > 1e-15)
	{
		init_flag = 2; // in general no Euler step after initialization

		// !!!!!!!!! workaround for iteration !!!!!!!!!!!!!
		if (new_dt < 1e-18) // time step smaller than 1e-3 fs
		{
			new_dt = 1e-18; // limit to very small step size (1e-3 fs)
			init_flag = 1; // Euler step after init - may there are jumps at inputs
		}

		dt = new_dt;
	}

	for (sca_util::sca_implementation::sca_method_vector::iterator
			it = pre_solve_methods->begin(); it
			!= pre_solve_methods->end(); ++it)
	{
		it->call_method();
	}

	if (equation_if->is_reinit_request() != 0)
	{
		// Euler step after reinitialization yields a necessary DC step
		// !!!!!! as long as there are no changes in matrix A !!!!!!
		if (equation_if->is_reinit_request() != 2)
			init_flag = 1;
		else
			init_flag = 2; //for test purposes euler supressed


		equation_if->is_reinit_request() = 0;

		//remove deleted methods
		for (unsigned long i=0;i<pre_solve_methods->size();i++)
		{
			if(!(*pre_solve_methods)[i].is_valid())
			{
				pre_solve_methods->erase(pre_solve_methods->begin()+i);
				i--;
			}
		}

		for (unsigned long i=0;i<post_solve_methods->size();i++)
		{
			if(!(*post_solve_methods)[i].is_valid())
			{
				post_solve_methods->erase(post_solve_methods->begin()+i);
				i--;
			}
		}
	}

	if (init_flag)
	{
		number_of_reinitializations++;
		int err = ana_reinit(A->get_flat(), B->get_flat(), A->n_cols(), dt,
				&internal_solver_data, init_flag);
		if (err)
		{
			long row=-1, column=-1;
			ana_get_error_position(internal_solver_data,&row,&column);

			sc_core::sc_object* obj_row=get_object_of_equation(row);
			sc_core::sc_object* obj_column=get_object_of_equation(column);

			std::ostringstream str;
			str << "Reinitialization failed in " << get_name() << ": " << err
					<< std::endl;
			str << "  during reinitialization equation system at "
					<< get_current_time() << " (dt = " << dt << ")." << std::endl;
			str << "  the error is in the following net (max. 50): " << std::endl;
			for (unsigned long i = 0; i < associated_module_list.size(); ++i)
			{
				if (i >= 50)
					break;
				str << "\t" << associated_module_list[i]->name() << std::endl;
			}

			str << std::endl;
			str << "Parameters of the following modules changed for the current time step:" << std::endl;
			for (unsigned long i = 0; i < associated_module_list.size(); ++i)
			{
				sca_core::sca_implementation::sca_conservative_module* cmod;
				cmod=dynamic_cast<sca_core::sca_implementation::sca_conservative_module*>(associated_module_list[i]);

				if(call_id==cmod->call_id_request_init)
				{
					str << "\t" << cmod->name();
					if(cmod->value_set) str << " changed to " << cmod->new_value;
					str << std::endl;
				}
			}

			if((obj_row!=NULL)||(obj_column!=NULL))
			{
				str << std::endl;
				str << "The error is may  be near: " << std::endl << "\t\t";
				if(obj_row!=NULL)
				{
					str << obj_row->name();
					if(obj_column!=obj_row) str << "  and " << std::endl << "\t\t";
				}

				if(obj_column!=obj_row) str << obj_column->name();
				str << std::endl;
			}


			SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
			return;
		}
	}

	ana_solv(q->get_calc_flat(), x_flat, internal_solver_data);

	//the call_id is used to identify modules which requested a reinit
	call_id++; //post solve contains to the new call id, due it may requests a reinit

	for (sca_util::sca_implementation::sca_method_vector::iterator
			it = post_solve_methods->begin(); it
			!= post_solve_methods->end(); ++it)
	{
		it->call_method();
	}


	for (std::vector<sca_util::sca_implementation::sca_trace_object_data*>::iterator
			it =  solver_traces.begin();
	        it != solver_traces.end(); ++it)
	{
		(*it)->trace();
	}

}

///////////////////////////////////////////

void sca_linear_solver::assign_equation_system(sca_linear_equation_if& eqs)
{
	init_method
			= static_cast<sca_core::sca_implementation::sc_object_method> (&sca_linear_solver::init_eq_system);
	init_method_object = this;

	processing_method
			= static_cast<sca_core::sca_implementation::sc_object_method> (&sca_linear_solver::solve_eq_system);
	processing_method_object = this;

	post_method = NULL;
	post_method_object = NULL;

	equations = &(eqs.get_equation_system());
	equation_if = &eqs;

	//for faster access
	A = &(equations->A);
	B = &(equations->B);
	q = &(equations->q);
	post_solve_methods = &(equations->post_solve_methods);
	pre_solve_methods = &(equations->pre_solve_methods);
	reinit_methods = &(equations->reinit_methods);
}

//////////////////////////////////////////////////////////////////////////////

/** Methods for supporting ac-simulation */
void sca_linear_solver::ac_domain_eq_method(sca_util::sca_matrix<double>*& A_o,
		sca_util::sca_matrix<double>*& B_o, sca_util::sca_vector<sca_util::sca_complex >*& q_o)
{
	//this allows frequency dependent matrix stamps - may be slow downs the performance
	equation_if->reinit_equations();
	ac_equation_initialized = true;

	A_o = A;
	B_o = B;

	double* qf = q->get_calc_flat(); //real only !!!!
	q_ac.resize(q->dimx());
	for (unsigned long i = 0; i < q->dimx(); ++i)
		q_ac(i) = qf[i];

	q_o = &q_ac;
}

//////////////////////////////////////////////////////////////////////////////

/** Methods for supporting ac-simulation */
void sca_linear_solver::ac_add_eq_cons_method(
		sca_util::sca_matrix<sca_util::sca_complex >*& con_matr,
		sca_util::sca_vector<sca_util::sca_complex >& y)
{
	//???? we assume static linear equtions (f-independent) only ??????

	unsigned long nstate = A->n_cols();
	//store current x-vector and than set to zero
	std::vector<double> x_state(nstate);
	for (unsigned long i = 0; i < nstate; ++i)
	{
		x_state[i] = x_flat[i];
		x_flat[i] = 0.0;
	}

	//for all x set to 1
	//call all post solve
	//for all outs
	//insert influence in matrix
	for (unsigned long i = 0; i < nstate; ++i)
	{
		x_flat[i] = 1.0;

		//call all post-solve methods
		for (sca_util::sca_implementation::sca_method_vector::iterator
				it = post_solve_methods->begin(); it
				!= post_solve_methods->end(); ++it)
			it->call_method();

		for (unsigned long j = 0; j < y.length(); ++j)
			con_matr_ac(i,j) = y(j);

		x_flat[i] = 0.0;
	}

	con_matr = &con_matr_ac;

	//restore old state
	for (unsigned long i = 0; i < nstate; ++i)
		x_flat[i] = x_state[i];
}

std::string sca_linear_solver::get_name_associated_names(int max_num)
{
	std::ostringstream str;
	str << get_name() << " containing modules";

	unsigned int max=max_num;
	if(max<0) max=associated_module_list.size();
	else
	{
		if(max>associated_module_list.size())
		{
			max=associated_module_list.size();
		}
		else
		{
			str << " (max. " << max << " printed)";
		}
	}

	str << ":" <<std::endl;
	for(unsigned long i=0;i<max;i++)
	{
		str << "\t\t" << associated_module_list[i]->name() << std::endl;
	}

	return str.str();
}


void sca_linear_solver::print_post_solve_statisitcs()
{
	//number of modules with most re-initialization to print
	static const unsigned long N_TOP_MOD=10;

	unsigned long& info_mask(
	            sca_core::sca_implementation::sca_get_curr_simcontext()->
	            get_information_mask());

	if(!(info_mask & sca_util::sca_info::sca_lsf_solver.mask)) return;

	//find 5 modules with highest reinit request cnt
	std::vector<sca_core::sca_implementation::sca_conservative_module*> top_vec;
	top_vec.resize(N_TOP_MOD,NULL);
	for (unsigned long k = 0; k < associated_module_list.size(); k++)
	{
		sca_core::sca_implementation::sca_conservative_module* curm;
		curm= dynamic_cast<sca_core::sca_implementation::sca_conservative_module*>
																(associated_module_list[k]);
		if (curm == NULL) continue;

		for (unsigned int i = 0; i < top_vec.size(); i++)
		{
			if (top_vec[i] != NULL)
			{
				if (top_vec[i]->reinit_request_cnt < curm->reinit_request_cnt)
				{
					sca_core::sca_implementation::sca_conservative_module* tmpm;
					tmpm = top_vec[i];
					top_vec[i] = curm;
					curm = tmpm;
				}
			}
			else
			{
				top_vec[i] = curm;
			}
		}
	}

	std::ostringstream str;
	sca_eln::sca_module* elnm=dynamic_cast<sca_eln::sca_module*>(top_vec[0]);
	sca_lsf::sca_module* lsfm=dynamic_cast<sca_lsf::sca_module*>(top_vec[0]);

	if ((elnm != NULL) || (lsfm != NULL))
	{
		str << std::endl;
		if (elnm != NULL)
		{
			str << "\tELN solver instance: " << get_name() << " (cluster " << cluster_id << ")";
		}
		else
		{
			str << "\tLSF solver instance: " << get_name() << " (cluster " << cluster_id << ")";
		}

		str << std::endl;
		str << "\t\thas calculated " << number_of_timesteps << " time steps the equation system was ";
		str << number_of_reinitializations << " times re-initialized"<< std::endl;
		str << "\t\tthe following max. " << N_TOP_MOD;
		str << " modules requested the most re-initializations:";
		str << std::endl;
		for (unsigned long j = 0; j < top_vec.size(); j++)
		{
			str << "\t\t\t" << top_vec[j]->name() << "\t"
					<< top_vec[j]->reinit_request_cnt;
			str << std::endl;
		}

		SC_REPORT_INFO("SystemC-AMS",str.str().c_str());
	}
}


} // namespace sca_implementation
} // namespace sca_core
