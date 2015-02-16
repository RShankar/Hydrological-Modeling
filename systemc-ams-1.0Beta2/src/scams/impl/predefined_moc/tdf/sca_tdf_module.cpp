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

 sca_tdf_module.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 05.08.2009

 SVN Version       :  $Revision: 1186 $
 SVN last checkin  :  $Date: 2011-05-05 17:37:00 +0200 (Thu, 05 May 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_module.cpp 1186 2011-05-05 15:37:00Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>

#include "scams/impl/predefined_moc/tdf/sca_tdf_view.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"
#include "scams/impl/core/sca_simcontext.h"

namespace sca_tdf
{

void sca_module::construct()
{
	//assign module to tdf - view
	view_interface = new sca_tdf::sca_implementation::sca_tdf_view;

	attr_method
			= static_cast<sca_module_method> (&sca_tdf::sca_module::set_attributes);
	init_method
			= static_cast<sca_module_method> (&sca_tdf::sca_module::initialize);
	sig_proc_method
			= static_cast<sca_module_method> (&sca_tdf::sca_module::processing);

	ac_processing_method= static_cast<sca_module_method> (&sca_tdf::sca_module::ac_processing);

	//obsolete
	post_method
			= static_cast<sca_module_method> (&sca_tdf::sca_module::post_proc);

	 reg_attr_method=false;
	 reg_init_method=false;
	 reg_processing_method=false;
	 reg_ac_processing_method=false;
	 reg_post_method=false;

	 no_default_processing=false;
	 no_default_ac_processing=false;

	 check_for_processing=false;

}

sca_module::sca_module(sc_core::sc_module_name)
{
	construct();
}

sca_module::sca_module()
{
	construct();
}

void sca_module::elaborate()
{
	sca_core::sca_module::elaborate(); // call base class method
	sca_ac_analysis::sca_implementation::sca_ac_domain_register_entity(
	        this,
	        static_cast<sca_ac_analysis::sca_implementation::sca_ac_domain_method>
				(ac_processing_method));

	//check for not allowed SystemC objects
	std::vector<sc_core::sc_object*> objl=get_child_objects();
	for(unsigned long i=0;i<objl.size();i++)
	{
		if(dynamic_cast<sc_core::sc_port_base*>(objl[i])!=NULL)
		{
			if(dynamic_cast<sca_core::sca_implementation::sca_port_base*>(objl[i])==NULL)
			{
				std::ostringstream str;
				str << " A sca_tdf::sca_module can contain sca_tdf ports only, port: " << objl[i]->name();
				str << " in module: " << name()
					<< " is not an sca_tdf::sca_in/out or sca_tdf::sc_in/out port";
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}
		}

		if(dynamic_cast<sc_core::sc_module*>(objl[i])!=NULL)
		{
			if(objl[i]->kind()!=std::string("sca_core::sca_implementation::systemc_ams_initializer"))
			{
				std::ostringstream str;
				str << " A sca_tdf::sca_module can not contain other modules. The module: "
				    << objl[i]->basename() << " kind: " << objl[i]->kind();
				str << " is instantiated in the context of module: " << name();
				SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
			}
		}
	}
}

//////////////////////////////////////////////

sca_module::~sca_module()
{
	delete view_interface;
}

//////////////////////////////////////////////

void sca_module::register_attr_method(sca_module_method funcp)
{
	if(sca_core::sca_implementation::sca_get_curr_simcontext()->construction_finished())
	{
		std::ostringstream str;
		str << "register_attr_method can be only called during module construction for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(reg_attr_method)
	{
		std::ostringstream str;
		str << "only one register_attr_method can be registered for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	reg_attr_method=true;


	attr_method = funcp;
}

//////////////////////////////////////////////

void sca_module::register_init_method(sca_module_method funcp)
{
	if(sca_core::sca_implementation::sca_get_curr_simcontext()->construction_finished())
	{
		std::ostringstream str;
		str << "register_init_method can be only called during module construction for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(reg_init_method)
	{
		std::ostringstream str;
		str << "only one register_init_method can be registered for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	reg_init_method=true;


	init_method = funcp;
}

//////////////////////////////////////////////

void sca_module::register_processing(sca_module_method funcp)
{
	if(sca_core::sca_implementation::sca_get_curr_simcontext()->construction_finished())
	{
		std::ostringstream str;
		str << "register_processing can be only called during module construction for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	check_for_processing=true;
	try { processing();}
	catch(...){}
	check_for_processing=false;

	if(!no_default_processing)
	{
		std::ostringstream str;
		str << "register_processing is called, however method processing was implemented for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(reg_processing_method)
	{
		std::ostringstream str;
		str << "only one register_processing_method can be registered for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	reg_processing_method=true;

	sig_proc_method = funcp;
}

//////////////////////////////////////////////


void sca_module::register_ac_processing(sca_tdf::sca_module::sca_module_method funcp)
{
	if(sca_core::sca_implementation::sca_get_curr_simcontext()->construction_finished())
	{
		std::ostringstream str;
		str << "register_ac_processing can be only called during module construction for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	//ignore errors
	try  { ac_processing(); }
	catch(...){}

	if(!no_default_ac_processing)
	{
		std::ostringstream str;
		str << "register_ac_processing is called, however method ac_processing was implemented for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}


	if(reg_ac_processing_method)
	{
		std::ostringstream str;
		str << "only one register_ac_processing_method can be registered for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	reg_ac_processing_method=true;

	ac_processing_method=funcp;
}

void sca_module::register_post_method(sca_module_method funcp)
{
	if(sca_core::sca_implementation::sca_get_curr_simcontext()->construction_finished())
	{
		std::ostringstream str;
		str << "register_post_method can be only called during module construction for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}

	if(reg_post_method)
	{
		std::ostringstream str;
		str << "only one register_post_method can be registered for ";
		str << "module: " << name();
		SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
	}
	reg_post_method=true;

	post_method = funcp;
}

const char* sca_module::kind() const
{
	return "sca_tdf::sca_module";
}

void sca_module::set_attributes()
{
}

void sca_module::initialize()
{
}

void sca_module::processing()
{
	 no_default_processing=true;

	 if(!check_for_processing)
	 {
		check_for_processing=true;
		std::ostringstream str;
		str << "No processing method implemented for TDF ";
		str << "module: " << name();
		SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());

	 }
}

void sca_module::ac_processing()
{
	 no_default_ac_processing=true;
}

sca_core::sca_time sca_module::get_time() const
{
	return sca_core::sca_module::sca_get_time();
}

}
//////////////////////////////////////////////


