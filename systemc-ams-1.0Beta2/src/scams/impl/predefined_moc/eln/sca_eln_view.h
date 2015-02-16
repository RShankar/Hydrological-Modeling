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

 sca_eln_view.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 06.11.2009

 SVN Version       :  $Revision: 1103 $
 SVN last checkin  :  $Date: 2011-01-08 21:11:02 +0100 (Sat, 08 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_view.h 1103 2011-01-08 20:11:02Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_ELN_VIEW_H_
#define SCA_ELN_VIEW_H_

#include "scams/impl/solver/linear/sca_linear_equation_if.h"
#include "scams/impl/predefined_moc/conservative/sca_conservative_view.h"

namespace sca_core
{
namespace sca_implementation
{
class sca_linear_solver;
}
}

namespace sca_eln
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

class sca_eln_view: public sca_core::sca_implementation::sca_conservative_view
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
} // namespace sca_eln


#endif /* SCA_ELN_VIEW_H_ */
