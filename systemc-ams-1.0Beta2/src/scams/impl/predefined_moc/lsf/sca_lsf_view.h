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

  sca_lsf_view.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 05.01.2010

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_view.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "scams/impl/solver/linear/sca_linear_equation_if.h"
#include "scams/impl/predefined_moc/conservative/sca_conservative_view.h"


#ifndef SCA_LSF_VIEW_H_
#define SCA_LSF_VIEW_H_

namespace sca_lsf
{
namespace sca_implementation
{

class lin_eqs_cluster: public sca_core::sca_implementation::sca_conservative_cluster,
		public sca_core::sca_implementation::sca_linear_equation_if
{
public:
	sca_core::sca_implementation::sca_linear_equation_if::equation_system* eqs;
	long reinit_request;

	friend class sca_linnet_view;

public:

	lin_eqs_cluster();
	~lin_eqs_cluster();

	/**
	 * implementation of methods from sca_linear_equation_if
	 */
	sca_core::sca_implementation::sca_linear_equation_if::equation_system& get_equation_system()
	{
		return *eqs;
	}

	void reinit_equations();
	long& is_reinit_request()
	{
		return reinit_request;
	}

};

//////////////////////////

class sca_lsf_view: public sca_core::sca_implementation::sca_conservative_view
{
protected:

	sca_core::sca_implementation::sca_linear_solver* solver;

public:

	sca_core::sca_implementation::sca_conservative_cluster* create_cluster()
	{
		return new lin_eqs_cluster;
	}

	/** Overwritten (virtual) method for equation setup */
	virtual void setup_equations();

private:

	sca_core::sca_implementation::sca_linear_equation_if::equation_system* eqs;

};


} // namespace sca_implementation
} // namespace sca_lsf


#endif /* SCA_LSF_VIEW_H_ */
