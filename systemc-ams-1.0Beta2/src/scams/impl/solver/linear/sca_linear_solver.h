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

 sca_linear_solver.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 09.11.2009

 SVN Version       :  $Revision: 1157 $
 SVN last checkin  :  $Date: 2011-02-09 23:08:11 +0100 (Wed, 09 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_linear_solver.h 1157 2011-02-09 22:08:11Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_LINEAR_SOLVER_H_
#define SCA_LINEAR_SOLVER_H_

#include "scams/impl/core/sca_solver_base.h"
#include "scams/utility/data_types/sca_matrix.h"
#include "scams/impl/solver/linear/sca_linear_equation_if.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/util/data_types/sca_function_vector.h"

class sca_solv_data;

namespace sca_core
{
namespace sca_implementation
{

/**
  Solver class for an analog linear solver.
*/
class sca_linear_solver : public sca_core::sca_implementation::sca_solver_base
{

public:

	sca_linear_solver(std::vector<sca_module*>& mods, std::vector<sca_interface*>& chans);
	~sca_linear_solver();

    /** Implements the pure virtual initialize method of @ref #sca_solver_base. */
    void initialize();

    void assign_equation_system(sca_core::sca_implementation::sca_linear_equation_if& eqs);

    sca_util::sca_vector<double>& get_state_vector() { return x; }

    sc_core::sc_object* get_object_of_equation(long eq_nr);

    void print_post_solve_statisitcs();

    double current_time;
    sc_dt::uint64 call_id;

private:

    std::string get_name_associated_names(int max_num=-1);

    void init_eq_system();
    void solve_eq_system();

    //reference to equation system
    sca_core::sca_implementation::sca_linear_equation_if::equation_system* equations;
    sca_core::sca_implementation::sca_linear_equation_if*                  equation_if;

    sca_util::sca_matrix<double>*   A;
    sca_util::sca_matrix<double>*   B;
    sca_util::sca_implementation::sca_function_vector<double>* q;

    sca_util::sca_implementation::sca_method_vector* pre_solve_methods;
    sca_util::sca_implementation::sca_method_vector* post_solve_methods;
    sca_util::sca_implementation::sca_method_vector* reinit_methods;

    sca_util::sca_vector<double>  x;  //state vector

    double*           x_flat;

    double dt;  // time step

    sca_solv_data* internal_solver_data;


    //Methods for supporting ac-simulation
    void ac_domain_eq_method( sca_util::sca_matrix<double>*& A_o,
                              sca_util::sca_matrix<double>*& B_o,
                              sca_util::sca_vector<sca_util::sca_complex >*& q_o );

    void ac_add_eq_cons_method( sca_util::sca_matrix<sca_util::sca_complex >*& con_matr,
                                           sca_util::sca_vector<sca_util::sca_complex >& y );

    sca_util::sca_vector<sca_util::sca_complex > q_ac;
    sca_util::sca_matrix<sca_util::sca_complex > con_matr_ac;

    bool ac_equation_initialized;

    unsigned long number_of_timesteps;
    unsigned long number_of_reinitializations;


};


} // namespace sca_implementation
} // namespace sca_core



#endif /* SCA_LINEAR_SOLVER_H_ */
