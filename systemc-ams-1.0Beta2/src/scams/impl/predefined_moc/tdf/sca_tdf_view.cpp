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

  sca_tdf_view.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 25.08.2009

   SVN Version       :  $Revision: 1149 $
   SVN last checkin  :  $Date: 2011-02-06 18:02:02 +0100 (Sun, 06 Feb 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_view.cpp 1149 2011-02-06 17:02:02Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "scams/impl/predefined_moc/tdf/sca_tdf_view.h"

#include "systemc-ams"
#include "scams/impl/core/sca_view_object.h"
#include "scams/impl/solver/tdf/sca_tdf_solver.h"
#include "scams/predefined_moc/tdf/sca_tdf_module.h"
#include "scams/impl/predefined_moc/tdf/sca_tdf_signal_impl_base.h"


#include<typeinfo>



namespace sca_tdf
{

namespace sca_implementation
{

////////////////////////////////////////////////

sca_tdf_view::sca_tdf_view()
{}

////////////////////////////////////////////////

sca_tdf_view::~sca_tdf_view()
{}

////////////////////////////////////////////////

void sca_tdf_view::setup_equations()
{
    sca_tdf_solver* solver;



    for(sca_core::sca_implementation::sca_view_object::sca_module_list_iteratorT mit=
              view_datas->get_module_list().begin();
         mit!=view_datas->get_module_list().end(); ++mit
       )
    {
        sca_tdf::sca_module* sdfm=dynamic_cast<sca_tdf::sca_module*>(*mit);

        if(sdfm==NULL)
        {
            /**
              this should not be occur due the sca_sdf-view is only
              used by sca_sdf_modules
            */

            std::ostringstream str;
            str << "Error: Module: " << (*mit)->name() << " has instantiated a tdf-view "
            << " however it is not a sca_tdf::sca_module" << std::endl;
            ::sc_core::sc_report_handler::report( \
            	    ::sc_core::SC_ERROR, "SystemC-AMS", str.str().c_str(), __FILE__, __LINE__ );
        }

        //call registered attribute method of current module
        sdfm->allow_attributes_access_flag=true;
        (sdfm->*(sdfm->attr_method))();
        sdfm->allow_attributes_access_flag=false;

        //every module represents a solver instance
		std::vector<sca_core::sca_module*> tmp_mod;
		tmp_mod.push_back(sdfm);

		//no channels associated
		std::vector<sca_core::sca_interface*> tmp_chan;

		solver = new sca_tdf::sca_implementation::sca_tdf_solver(tmp_mod,tmp_chan);


        //initialize modules callcounter with solver reference
        sdfm->call_counter = &(solver->get_call_counter_ref());

        // register solver methods
        solver->register_methods(sdfm,
                                 sdfm->init_method,
                                 sdfm->sig_proc_method,
                                 sdfm->post_method);

        //propagate module timestep settings
        sdfm->timestep_elaborated=true; //forbid further setting
        if(sdfm->timestep_is_set)
        {
        	solver->timestep_is_set = true;
        	solver->timestep_set = sdfm->timestep_set;
        	solver->module_timestep_set = sdfm;
        	solver->module_min_timestep_set = sdfm;         //not required for tdf
        	solver->min_timestep_set = sdfm->timestep_set;  //not required for tdf
        }

        //for each port
        for(sca_core::sca_module::sca_port_base_list_iteratorT pit=
                  (*mit)->get_port_list().begin();
             pit!=(*mit)->get_port_list().end(); ++pit
           )
        {
            sca_core::sca_interface* sca_if = (*pit)->sca_get_interface();

            sca_tdf_signal_impl_base* sdf_if =
                dynamic_cast<sca_tdf_signal_impl_base*>(sca_if);


            unsigned long port_id = (*pit)->get_if_id();
            sca_tdf::sca_implementation::sca_port_attributes* attr;
            attr = dynamic_cast<sca_tdf::sca_implementation::sca_port_attributes*>(*pit);



            //sort ports
            switch((*pit)->get_port_type())
            {
            case sca_core::sca_implementation::sca_port_base::SCA_IN_PORT:

                if(sdf_if==NULL)
                {
                    std::ostringstream str;
                    str  << "Error tdf module: " << (*mit)->name() << " has instantiated a non TDF port: "
                    << (*pit)->sca_name() << std::endl;
                    ::sc_core::sc_report_handler::report( \
                     	    ::sc_core::SC_ERROR, "SystemC-AMS", str.str().c_str(), __FILE__, __LINE__ );
                }


                solver->push_back_from_analog(*pit,
                		sca_if,
                		sdf_if->get_rate(port_id),
                		sdf_if->get_delay(port_id),
                		sdf_if->get_T(port_id),
                		sdf_if->get_t0(port_id)
                                             );
                break;

            case sca_core::sca_implementation::sca_port_base::SCA_OUT_PORT:

                if(sdf_if==NULL)
                {
                    std::ostringstream str;
                    str  << "Error tdf module: " << (*mit)->name() << " has instantiated a non TDF port: "
                    << (*pit)->sca_name() << std::endl;
                    ::sc_core::sc_report_handler::report( \
                     	    ::sc_core::SC_ERROR, "SystemC-AMS", str.str().c_str(), __FILE__, __LINE__ );
                }

                solver->push_back_to_analog(  *pit,
                		sca_if,
                   		sdf_if->get_rate(port_id),
                    		sdf_if->get_delay(port_id),
                    		sdf_if->get_T(port_id),
                    		sdf_if->get_t0(port_id)
                                           );
                break;

            case sca_core::sca_implementation::sca_port_base::SCA_SC_IN_PORT:
                if(attr==NULL)
                {
                    ::sc_core::sc_report_handler::report( \
                      	    ::sc_core::SC_ERROR, "SystemC-AMS",
                      	    "Internal error (should not be possible)",
                      	    __FILE__, __LINE__ );
                	return;
                }


                solver->push_back_from_systemc( *pit,
                		NULL,
                        attr->rate,
                         attr->delay,
                         attr->timestep_calculated,
                         attr->timeoffset_calculated
                                              );
                break;

            case sca_core::sca_implementation::sca_port_base::SCA_SC_OUT_PORT:

                if(attr==NULL)
                {
                    ::sc_core::sc_report_handler::report( \
                      	    ::sc_core::SC_ERROR, "SystemC-AMS",
                      	    "Internal error (should not be possible)",
                      	    __FILE__, __LINE__ );
                	return;
                }

                solver->push_back_to_systemc( *pit,
                		NULL,
                        attr->rate,
                         attr->delay,
                         attr->timestep_calculated,
                         attr->timeoffset_calculated
                                            );
                break;

            default:

            	std::ostringstream str;
            	str << " not allowed port instantiated in a tdf module for port: ";
            	str <<  (*pit)->sca_name();
                ::sc_core::sc_report_handler::report( \
                 	    ::sc_core::SC_ERROR, "SystemC-AMS", str.str().c_str(), __FILE__, __LINE__ );

            }
        }
    }
}


}
}
////////////////////////////////////////////////
