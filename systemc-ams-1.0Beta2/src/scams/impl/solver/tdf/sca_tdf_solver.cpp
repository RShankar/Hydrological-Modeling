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

  sca_tdf_solver.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 25.08.2009

   SVN Version       :  $Revision: 1104 $
   SVN last checkin  :  $Date: 2011-01-09 11:45:25 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_solver.cpp 1104 2011-01-09 10:45:25Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/impl/solver/tdf/sca_tdf_solver.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/core/sca_object_manager.h"
#include "scams/impl/synchronization/sca_synchronization_obj_if.h"
#include "scams/impl/synchronization/sca_synchronization_layer.h"

namespace sca_tdf
{

namespace sca_implementation
{

//////////////////////////////////////////////////////////////////////

sca_tdf_solver::sca_tdf_solver(std::vector<sca_core::sca_module*>& mods,
		                       std::vector<sca_core::sca_interface*>& chans) :
	sca_solver_base( ::sc_core::sc_gen_unique_name("sca_tdf_solver"), mods,chans )
{
}

//////////////////////////////////////////////////////////////////////

sca_tdf_solver::~sca_tdf_solver()
{
}


//////////////////////////////////////////////////////////////////////

void sca_tdf_solver::initialize()
{

 ::sca_core::sca_implementation::sca_get_curr_simcontext()->get_sca_object_manager()->
    get_synchronization_if()->registrate_solver_instance(this);
#ifdef SCA_IMPLEMENTATION_DEBUG
  std::cout << "\t\t" << " timed dataflow solver instance initialized" << std::endl;
#endif
}

//////////////////////////////////////////

void sca_tdf_solver::register_methods(sca_core::sca_module* mod,
                                      sca_tdf::sca_module::sca_module_method init_fct,
                                      sca_tdf::sca_module::sca_module_method proc_fct,
                                      sca_tdf::sca_module::sca_module_method post_fct)
{
  module = mod;

  sync_obj_name = mod->name();

  init_method        = static_cast<sca_core::sca_implementation::sc_object_method>(init_fct);
  init_method_object = mod;

  processing_method        = static_cast<sca_core::sca_implementation::sc_object_method>(proc_fct);
  processing_method_object = mod;

  post_method        = static_cast<sca_core::sca_implementation::sc_object_method>(post_fct);
  post_method_object = mod;
}

}
}
