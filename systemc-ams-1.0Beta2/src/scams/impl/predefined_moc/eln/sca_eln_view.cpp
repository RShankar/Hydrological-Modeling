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

 sca_eln_view.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 09.11.2009

 SVN Version       :  $Revision: 1113 $
 SVN last checkin  :  $Date: 2011-01-23 17:09:13 +0100 (Sun, 23 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_view.cpp 1113 2011-01-23 16:09:13Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/predefined_moc/eln/sca_eln_view.h"
#include "scams/predefined_moc/eln/sca_eln_module.h"
#include "scams/predefined_moc/tdf/sca_tdf_signal_if.h"
#include "scams/impl/solver/linear/sca_linear_equation_if.h"
#include "scams/impl/solver/linear/sca_linear_solver.h"

namespace sca_eln
{
namespace sca_implementation
{

lin_eqs_cluster::lin_eqs_cluster()
{
	eqs = 0;
	reinit_request = 0;
}

////////////////////////////////////////////////////////////////

lin_eqs_cluster::~lin_eqs_cluster()
{
	delete eqs;
}

////////////////////////////////////////////////////////////////

void lin_eqs_cluster::reinit_equations()
{
	//number of nodes
	unsigned long nnodes = channels.size();

	eqs->post_solve_methods.resize(0);
	eqs->pre_solve_methods.resize(0);
	eqs->reinit_methods.resize(0);

	eqs->A.resize(nnodes, nnodes);
	eqs->B.resize(nnodes, nnodes);
	eqs->q.resize(nnodes);

	eqs->A.reset();
	eqs->B.reset();
	eqs->q.reset();

	for (sca_conservative_cluster::iterator mit = modules.begin(); mit
			!= modules.end(); ++mit)
	{
		sca_eln::sca_module* lmod = dynamic_cast<sca_eln::sca_module*> (*mit);
		lmod->add_equations.resize(0);
		lmod->matrix_stamps();
	}
}

////////////////////////////////////////////////////////////////

void sca_eln_view::setup_equations()
{
#ifdef SCA_IMPLEMENTATION_DEBUG
	std::cout << "Setup equations for sca_linnet_view: "
	<< conservative_clusters.size() << std::endl;
#endif

	cluster_nets();

	//for all clusters
	for (ccluster_listT::iterator cit = conservative_clusters.begin(); cit
			!= conservative_clusters.end(); ++cit)
	{
		lin_eqs_cluster* lcl = dynamic_cast<lin_eqs_cluster*> (*cit);
		if (lcl == NULL)
		{
			SC_REPORT_ERROR("SystemC-AMS", "Internal Error");
		}

		eqs
				= new sca_core::sca_implementation::sca_linear_equation_if::equation_system;
		lcl->eqs = eqs;

		eqs->A.unset_auto_resizable();
		eqs->B.unset_auto_resizable();
		eqs->q.unset_auto_resizable();

		//ignore matrix access with negative indices -> reference node
		eqs->A.set_ignore_negative();
		eqs->B.set_ignore_negative();
		eqs->q.set_ignore_negative();


		std::vector<sca_core::sca_module*> tmp_mod;
		for (sca_core::sca_implementation::sca_conservative_cluster::iterator
				mit = lcl->begin(); mit != lcl->end(); ++mit) tmp_mod.push_back(*mit);

		std::vector<sca_core::sca_interface*> tmp_chan;
		for(unsigned long i=0;i<lcl->channels.size();i++) tmp_chan.push_back(lcl->channels[i]);

		solver = new sca_core::sca_implementation::sca_linear_solver(tmp_mod,tmp_chan);


#ifdef SCA_IMPLEMENTATION_DEBUG
		std::cout << "   sca_linear solver created" << std::endl;
#endif

		//set equation reference to modules
		for (sca_core::sca_implementation::sca_conservative_cluster::iterator
				mit = lcl->begin(); mit != lcl->end(); ++mit)
		{
			sca_eln::sca_module* lmod =
					dynamic_cast<sca_eln::sca_module*> (*mit);

			if (lmod == NULL)
			{
				std::ostringstream str;
				str << "Module: " << (*mit)->name()
						<< " is assigned to sca_linnet_view."
						<< " However it is not a sca_linnet_module."
						<< std::endl;
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}
			else
				lmod->set_equations(eqs->A, eqs->B, eqs->q,
						solver->get_state_vector(), eqs->post_solve_methods,
						eqs->pre_solve_methods, solver->current_time,
						lcl->reinit_request,
						solver->call_id);

			//timestep module->sync object propagation

			lmod->timestep_elaborated=true;  //forbid further setting
			if (lmod->timestep_is_set)
			{
				if (solver->timestep_is_set)
				{
					sc_dt::int64 diff = lmod->timestep_set.value()
							- solver->timestep_set.value();
					if ((diff > 1) || (diff < -1))
					{
						std::ostringstream str;
						str << "Inconsistent timestep between module: "
								<< lmod->name() << ": "
								<< lmod->timestep_set << " and module: ";
						if (solver->module_timestep_set == NULL)
							str << "unknown module";
						else
							str << solver->module_timestep_set->name();
						str << ": " << solver->timestep_set
						    << " (modules belong to the same eln cluster "
						    << "thus the timesteps must be equal)";

						SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
					}

					//we use larger timestep
					if (diff > 0)
					{
						solver->timestep_set = lmod->timestep_set;
						solver->module_timestep_set = lmod;

						//prevent drifting
						sc_dt::int64 diff_min = lmod->timestep_set.value()
								- solver->min_timestep_set.value();
						if (diff_min > 1)
						{
							std::ostringstream str;
							str << "Inconsistent timestep between module: "
									<< lmod->name() << " timestep: "
									<< lmod->timestep_set << " and module: ";
							if (solver->module_min_timestep_set == NULL)
								str << "unknown module";
							else
								str << solver->module_min_timestep_set->name();
							str << " timestep: " << solver->min_timestep_set;

							SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
						}
					}
					else
					{
						if (lmod->timestep_set < solver->min_timestep_set)
						{
							solver->module_min_timestep_set = lmod;
							solver->min_timestep_set = lmod->timestep_set;
						}
					}
				}
				else
				{
					solver->timestep_is_set = true;
					solver->timestep_set = lmod->timestep_set;
					solver->module_timestep_set = lmod;
					solver->module_min_timestep_set = lmod;
					solver->min_timestep_set = lmod->timestep_set;
				}
			}

			//end timestep module->sync object propagation

		} // for all modules


		solver->assign_equation_system(*lcl);

		//establish connection to other domains and dt-SystemC
		//sort ports
		for (std::vector<sca_core::sca_implementation::sca_port_base*>::iterator
				pit = (*cit)->foreign_ports.begin(); pit
				!= (*cit)->foreign_ports.end(); ++pit)
		{
			sca_core::sca_interface* sca_if = (*pit)->sca_get_interface();

			::sca_tdf::sca_implementation::sca_tdf_signal_impl_base
			* sdf_if =
			dynamic_cast< ::sca_tdf::sca_implementation::sca_tdf_signal_impl_base*> (sca_if);


			::sca_tdf::sca_implementation::sca_port_attributes* attr;
			attr = dynamic_cast< ::sca_tdf::sca_implementation::sca_port_attributes*>(*pit);


			unsigned long port_id = (*pit)->get_if_id();

			switch ((*pit)->get_port_type())
			{
			case sca_core::sca_implementation::sca_port_base::SCA_IN_PORT:

                if(sdf_if==NULL)
                {
                    std::ostringstream str;
                    str  << "Error module: " << (*pit)->get_parent_module()->name()
                         << " has bound non sca_tdf_interface at port: "
                         << (*pit)->get_port_number() << std::endl;
                    SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
                }

				solver->push_back_from_analog(*pit, sca_if, sdf_if->get_rate(
						port_id), sdf_if->get_delay(port_id), sdf_if->get_T(
						port_id), sdf_if->get_t0(port_id));
				break;

			case sca_core::sca_implementation::sca_port_base::SCA_OUT_PORT:

                if(sdf_if==NULL)
                {
                    std::ostringstream str;
                    str  << "Error module: " << (*pit)->get_parent_module()->name()
                         << " has bound non sca_tdf_interface at port: "
                    << (*pit)->get_port_number() << std::endl;
                    SC_REPORT_ERROR("SystemC-AMS", str.str().c_str());
                }

				solver->push_back_to_analog(*pit, sca_if, sdf_if->get_rate(
						port_id), sdf_if->get_delay(port_id), sdf_if->get_T(
						port_id), sdf_if->get_t0(port_id));
				break;

			case sca_core::sca_implementation::sca_port_base::SCA_SC_IN_PORT:

	             if(attr==NULL)
	             {
	            	SC_REPORT_ERROR("SystemC-AMS","Internal error (should not be possible)");
	                return;
	             }

				solver->push_back_from_systemc(*pit, NULL,
                        attr->rate,
                         attr->delay,
                         attr->timestep_calculated,
                         attr->timeoffset_calculated);
				break;

			case sca_core::sca_implementation::sca_port_base::SCA_SC_OUT_PORT:

	             if(attr==NULL)
	             {
	            	SC_REPORT_ERROR("SystemC-AMS","Internal error (should not be possible)");
	                return;
	             }

				solver->push_back_to_systemc(*pit, NULL,
                        attr->rate,
                         attr->delay,
                         attr->timestep_calculated,
                         attr->timeoffset_calculated);
				break;

			default:

				SC_REPORT_ERROR("SystemC-AMS", "Not supported sca-port direction in linnet view.");
			}
		} //for all foreign ports

	} //for all clusters

}

}
}