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

 sca_simcontext.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 13.05.2009

 SVN Version       :  $Revision: 1198 $
 SVN last checkin  :  $Date: 2011-05-10 15:31:38 +0200 (Tue, 10 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_simcontext.cpp 1198 2011-05-10 13:31:38Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/impl/core/sca_globals.h"

//#include "scams/impl/util/sca_trace_base.h"
#include "scams/impl/core/sca_simcontext.h"

#include "scams/impl/core/sca_object_manager.h"
#include "scams/impl/core/sca_solver_manager.h"
#include "scams/impl/core/sca_solver_base.h"


#include<string>
#include <iostream>

//STL usage
#include <string>

#define HAVE_CONFIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "????"
#endif

// \TODO to be moved to config.h
#define SCA_RELEASE_DATE    "2011_05_12"

using namespace sca_core;
using namespace sca_core::sca_implementation;

namespace sca_core
{
namespace sca_implementation
{

static const sca_core::sca_time not_valid_sca_time;

const sca_core::sca_time& NOT_VALID_SCA_TIME()
{
	//TODO find a better solution, which may not sets the resolution fixed flag
	//after we set the time, the time resolution can't be changed
	//(except the value is zero) -thus we are not able to initialize the constant

	if(not_valid_sca_time==sc_core::SC_ZERO_TIME)
	(*const_cast<sca_core::sca_time*>(&not_valid_sca_time))=
		sc_core::SC_ZERO_TIME-sc_core::sc_get_time_resolution();

	return not_valid_sca_time;
}

// Not MT-safe!
static sca_simcontext* sca_curr_simcontext = 0;

class systemc_ams_initializer : sc_core::sc_module
{
	void end_of_elaboration();
	void end_of_simulation();

	const char* kind() const
	{
		return "sca_core::sca_implementation::systemc_ams_initializer";
	}


public:

	bool construction_finished;

	SC_CTOR(systemc_ams_initializer)
	{
		sca_curr_simcontext=NULL;
		construction_finished=false;
	}

	//using this may we are later able to provide a solution without
	//static variable
	sca_simcontext* sca_curr_simcontext;
};


void systemc_ams_initializer::end_of_elaboration()
{
	construction_finished=true;
	sca_core::sca_implementation::sca_systemc_ams_init();
}


void systemc_ams_initializer::end_of_simulation()
{

	if(sca_curr_simcontext!=NULL)
	{
		std::vector<sca_solver_base*>& solvers=
				sca_curr_simcontext->get_sca_solver_manager()->get_solvers();

		for(unsigned int i=0;i<solvers.size();i++)
		{
			solvers[i]->print_post_solve_statisitcs();
		}


		delete sca_curr_simcontext;
		sca_curr_simcontext=NULL;
	}
}

sca_simcontext* sca_get_curr_simcontext()
{
	if (sca_curr_simcontext == 0)
	{
#ifdef PURIFY
		static sca_simcontext sca_default_global_context;
		sca_curr_simcontext = &sca_default_global_context;
#else
		sca_curr_simcontext = new sca_simcontext;
#endif
	}

	return sca_curr_simcontext;
}



//////////////////////////////////////////////////////////////////////////////////

sca_object_manager* sca_simcontext::get_sca_object_manager()
{
	return m_sca_object_manager;
}

//////////////////////////////////////////////////////////////////////////////////

sca_solver_manager* sca_simcontext::get_sca_solver_manager()
{
	return m_sca_solver_manager;
}

//////////////////////////////////////////////////////////////////////////////////

sca_view_manager* sca_simcontext::get_sca_view_manager()
{
	return m_sca_view_manager;
}

//////////////////////////////////////////////////////////////////////////////////


void sca_simcontext::sca_pln()
{
	static bool lnp = false;
	if (!lnp)
	{
#ifdef REVISION
		std::string revision(REVISION);
#else
		std::string revision;
#endif
		std::string::size_type br;
		while ((br = revision.find("$")) != std::string::npos)
			revision.erase(br, 1);
		while ((br = revision.find("Rev:")) != std::string::npos)
			revision = revision.substr(br + 4);

		std::cerr << std::endl << std::endl;
		std::cerr << "     SystemC AMS extensions 1.0 Version: " << VERSION << " --- Build"
				<< revision << " " << SCA_RELEASE_DATE << std::endl;
		std::cerr
				<< "          Copyright (c) 2010-2011  by Fraunhofer-Gesellschaft"
				<< std::endl;
		std::cerr
				<< "                 Institut Integrated Circuits / EAS"
				<< std::endl;
		std::cerr
				<< "            Licensed under the Apache License, Version 2.0"
				<< std::endl;
		std::cerr << std::endl << std::endl;

		lnp = true;
	}
}

//////////////////////////////////////////////////////////////////


bool sca_simcontext::initialized()
{
	bool ret_val;
	if (m_sca_object_manager == NULL)
		ret_val = false;
	else
		ret_val = m_sca_object_manager->initialized();

	return ret_val;
}

//////////////////////////////////////////////////////////////////


sca_simcontext::sca_simcontext()
{
	sca_pln();

	time_domain_simulation_flag = true; //default time domain simulation

	m_sca_object_manager = new sca_object_manager; //create objectmanager

	m_sca_solver_manager = m_sca_object_manager->solvers;
	m_sca_view_manager = m_sca_object_manager->views;

	//establish reference to main simcontext
	sc_kernel_simcontext = sc_core::sc_get_curr_simcontext();

	//if no traces available, all traces are initialized
	traces_initialized=true;

	information_mask=~0;  //default all information on

	ac_db=NULL;

	scams_init=new systemc_ams_initializer(sc_core::sc_gen_unique_name("sca_implementation"));
	scams_init->sca_curr_simcontext=this;

}

//////////////////////////////////////////////////////////////////

unsigned long& sca_simcontext::get_information_mask()
{
  return information_mask;
}

bool& sca_simcontext::all_traces_initialized()
{
	return traces_initialized;
}


//////////////////////////////////////////////////////////////////

sca_simcontext::~sca_simcontext()
{
	delete m_sca_object_manager;
	//delete scams_init; //we cant destroy a module
	                     //addtionally the end_of_simulation callback
	                     //of this modules calls the sca_simcontext
	                     //destructor

	sca_curr_simcontext = 0; //will not work ifdef PURIFY
}

//////////////////////////////////////////////////////////////////

sca_module* sca_simcontext::get_current_sca_module()
{
	::sc_core::sc_module* base_module = sc_kernel_simcontext->hierarchy_curr();

	//not inside any module
	if (base_module == NULL)
		return NULL;

	sca_module* module = dynamic_cast<sca_module*> (base_module);

	//if module==NULL current module no sca_module
	return module;
}

//////////////////////////////////////////////////////////////////

std::vector<sca_util::sca_implementation::sca_trace_file_base*>* sca_simcontext::get_trace_list()
{
	return &(m_sca_object_manager->trace_list);
}


//////////////////////////////////////////////////////////////////

bool sca_simcontext::construction_finished()
{
	return scams_init->construction_finished;
}


//////////////////////////////////////////////////////////////////

bool sca_simcontext::time_domain_simulation()
{
	return time_domain_simulation_flag;
}

//////////////////////////////////////////////////////////////////

void sca_simcontext::set_no_time_domain_simulation()
{
	time_domain_simulation_flag = false;
}

//////////////////////////////////////////////////////////////////

void sca_simcontext::set_time_domain_simulation()
{
	time_domain_simulation_flag = true;
}

//////////////////////////////////////////////////////////////////

}
}
