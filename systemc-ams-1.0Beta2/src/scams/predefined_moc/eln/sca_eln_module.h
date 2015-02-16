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

 sca_eln_module.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 06.11.2009

 SVN Version       :  $Revision: 1113 $
 SVN last checkin  :  $Date: 2011-01-23 17:09:13 +0100 (Sun, 23 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_module.h 1113 2011-01-23 16:09:13Z karsten $

 *****************************************************************************/
/*
 *
 *
 *
 */

/*****************************************************************************/

#ifndef SCA_ELN_MODULE_H_
#define SCA_ELN_MODULE_H_

namespace sca_util
{

namespace sca_implementation
{

template<class T>
class sca_method_object;

template<class T>
class sca_method_list;

template<class T>
class sca_function_vector;

class sca_method_vector;

}

}


namespace sca_eln
{

class sca_node;

namespace sca_implementation
{
class lin_eqs_cluster;
class sca_eln_view;
}

/**
 * Base class for modules for a electrical linear net.
 * The equation system to be solved is:
 *                  A dx/dt  + B x = q(t)
 */
class sca_module: public sca_core::sca_implementation::sca_conservative_module
{
public:
	const char* kind() const;

protected:
	//constructor
	sca_module();

	//destructor
	~sca_module();

private:

	//used by the sca_eln_view to set references to eq-system and results
	void set_equations(
			sca_util::sca_matrix<double>& As,
			sca_util::sca_matrix<double>& Bs,
			sca_util::sca_implementation::sca_function_vector<double>& qs,
			sca_util::sca_vector<double>& xs,
			sca_util::sca_implementation::sca_method_vector& post_solve_methods,
			sca_util::sca_implementation::sca_method_vector& pre_solve_methodss,
			double& ctime,
			long& reinit_requests,
			sc_dt::uint64& call_id);

protected:

	virtual void matrix_stamps() = 0;



	/**
	 * access to the equation system
	 */
	double& A(long a, long b);
	double& B(long a, long b);
	const double& A(long a, long b) const;
	const double& B(long a, long b) const;
	sca_util::sca_implementation::sca_method_list<double>& q(long a);

	/**
	 * result/state vector
	 */
	double& x(long a);

	/**
	 * returns current time
	 */
	double sca_get_time();

	/**
	 * overall number of equations
	 */
	unsigned long nequations();

	/**
	 * add new equations ;
	 * returns number (index starting by 0) of first new equation
	 */
	unsigned long add_equation(unsigned long num = 1);

	/**
	 * add method which will be called before the solver for each time step
	 * can be used to schedule reinitialization
	 */
	void add_pre_solve_method(const sca_util::sca_implementation::sca_method_object<void>&);
	void remove_pre_solve_method(const sca_util::sca_implementation::sca_method_object<void>&);

	/**
	 * add method which will be called after the solver for each time step
	 * can be used to access results
	 */
	void add_post_solve_method(const sca_util::sca_implementation::sca_method_object<void>&);
	void remove_post_solve_method(const sca_util::sca_implementation::sca_method_object<void>&);

	//redirect to base class to prevent friend declaration for each module or
	//protected declaration in sca_core::sca_module (in this case accesable
	//e.g. in tdf modules)
	sca_core::sca_implementation::sca_solver_base* get_sync_domain()
	{
		return sca_core::sca_module::get_sync_domain();
	}

	//redirect to base class to prevent friend declaration  ... see above
	sca_core::sca_time get_time()
	{
		return sca_core::sca_module::sca_get_time();
	}



	std::string through_value_type;
	std::string through_value_unit;
	bool through_value_available;

private:

	sca_util::sca_matrix<double>* Ai;
	sca_util::sca_matrix<double>* Bi;
	sca_util::sca_implementation::sca_function_vector<double>* qi;

	sca_util::sca_vector<double>* xi;

	sca_util::sca_implementation::sca_method_vector* post_solve_methodsi;
	sca_util::sca_implementation::sca_method_vector* pre_solve_methodsi;


	double* current_time;

	friend class sca_eln::sca_implementation::lin_eqs_cluster;
	friend class sca_eln::sca_implementation::sca_eln_view;
	friend class sca_eln::sca_node;
};

}

#endif /* SCA_ELN_MODULE_H_ */
