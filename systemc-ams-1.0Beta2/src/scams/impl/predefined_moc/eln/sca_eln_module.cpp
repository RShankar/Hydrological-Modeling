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

  sca_eln_module.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 06.11.2009

   SVN Version       :  $Revision: 1120 $
   SVN last checkin  :  $Date: 2011-01-24 18:22:26 +0100 (Mon, 24 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_module.cpp 1120 2011-01-24 17:22:26Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_module.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/util/data_types/sca_function_vector.h"
#include "scams/impl/core/sca_view_base.h"
#include "scams/impl/predefined_moc/eln/sca_eln_view.h"
#include <algorithm>

namespace sca_eln
{


//used by the sca_linnet_view to set references to eq-system and results
void sca_module::set_equations(
			sca_util::sca_matrix<double>& As,
			sca_util::sca_matrix<double>& Bs,
			sca_util::sca_implementation::sca_function_vector<double>& qs,
			sca_util::sca_vector<double>& xs,
			sca_util::sca_implementation::sca_method_vector& post_solve_methodss,
			sca_util::sca_implementation::sca_method_vector& pre_solve_methodss,
			double& ctime,
			long& reinit_requests,
			sc_dt::uint64& call_id)
{
	Ai = &As;
	Bi = &Bs;
	qi = &qs;
	xi = &xs;
	post_solve_methodsi = &post_solve_methodss;
	pre_solve_methodsi = &pre_solve_methodss;
	current_time = &ctime;
	reint_request_i = &reinit_requests;
	call_id_p=&call_id;
}



const char* sca_module::kind() const
{
	return "sca_eln::sca_module";
}

/**
 * access to the equation system
 */
const double& sca_module::A(long a, long b) const
{
	return (*Ai)(a, b);
}



const double& sca_module::B(long a, long b) const
{
	return (*Bi)(a, b);
}


double& sca_module::A(long a, long b)
{
	return (*Ai)(a, b);
}



double& sca_module::B(long a, long b)
{
	return (*Bi)(a, b);
}


sca_util::sca_implementation::sca_method_list<double>& sca_module::q(long a)
{
	return (*qi)(a);
}

/**
 * result/state vector
 */
double& sca_module::x(long a)
{
	return (*xi)(a);
}

/**
 * returns current time
 */
double sca_module::sca_get_time()
{
	return *current_time;
}

/**
 * overall number of equations
 */
unsigned long sca_module::nequations()
{
	return (*Ai).n_cols();
}

/**
 * add new equations ;
 * returns number (index starting by 0) of first new equation
 */
unsigned long sca_module::add_equation(unsigned long num)
{
	unsigned long n = nequations();
	unsigned long new_n = n + num;
	(*Ai).resize(new_n,new_n);
	(*Bi).resize(new_n,new_n);
	(*qi).resize(new_n);

	add_equations.push_back(n);

	return n;
}

/**
 * add method which will be called before the solver for each time step
 * can be used to schedule reinitialization
 */
void sca_module::add_pre_solve_method(
		const sca_util::sca_implementation::sca_method_object<void>& method_obj)
{
	pre_solve_methodsi->push_back(method_obj);
}

/**
 * removes pre_solve method
 */
void sca_module::remove_pre_solve_method(
		const sca_util::sca_implementation::sca_method_object<void>& method_obj)
{
	sca_util::sca_implementation::sca_method_vector::iterator it;
	it=std::find(pre_solve_methodsi->begin(),pre_solve_methodsi->end(),method_obj);

	//schedule for removal -> request_reinit required for final remove
	if(it!=post_solve_methodsi->end()) it->clear();
}

/**
 * add method which will be called after the solver for each time step
 * can be used to access results
 */
void sca_module::add_post_solve_method(
		const sca_util::sca_implementation::sca_method_object<void>& method_obj)
{
	post_solve_methodsi->push_back(method_obj);
}


/**
 * removes post_solve method
 */
void sca_module::remove_post_solve_method(
		const sca_util::sca_implementation::sca_method_object<void>& method_obj)
{
	sca_util::sca_implementation::sca_method_vector::iterator it;
	it=std::find(post_solve_methodsi->begin(),post_solve_methodsi->end(),method_obj);

	//schedule for removal -> request_reinit required for final remove

	if(it!=post_solve_methodsi->end()) it->clear();

}



sca_module::sca_module()
{
    //assign module to linnet - view
    view_interface = new sca_eln::sca_implementation::sca_eln_view;

    through_value_type = "-";
    through_value_unit = "-";
    through_value_available = false;
}


///////////////////////////////////////////////////////////////////////////////

sca_module::~sca_module()
{
    delete view_interface;
}

///////////////////////////////////////////////////////////////////////////////


} // namespace sca_eln
