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

  sca_lsf_tdf_demux.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 07.01.2010

   SVN Version       :  $Revision: 1201 $
   SVN last checkin  :  $Date: 2011-05-10 17:30:26 +0200 (Tue, 10 May 2011) $
   SVN checkin by    :  $Author: voigt $
   SVN Id            :  $Id: sca_lsf_tdf_demux.cpp 1201 2011-05-10 15:30:26Z voigt $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/lsf/sca_lsf_tdf_demux.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_lsf
{
namespace sca_tdf
{

sca_demux::sca_demux(sc_core::sc_module_name) :
	x("x"), y1("y1"), y2("y2"), ctrl("ctrl")
{
	ctrl_old=false;
	ctrl_val=false;
}

const char* sca_demux::kind() const
{
	return "sca_lsf::sca_tdf::sca_demux";
}


void sca_demux::update_ctrl()
{
    if(get_time()==sc_core::SC_ZERO_TIME) read_ctrl();

    if(ctrl_val!=ctrl_old)
    {
    	ctrl_old=ctrl_val;

    	if(!ctrl_val)
    	{
    		B(y1,x) = 1.0;
       		B(y2,x) = 0.0;
    	}
    	else
    	{
    		B(y1,x) = 0.0;
       		B(y2,x) = 1.0;
    	}

        request_reinit(1,ctrl_val);
    }
}


void sca_demux::read_ctrl()
{
    ctrl_val = ctrl.read();
}


void sca_demux::matrix_stamps()
{
    B(y1,y1)  =  -1.0;
    B(y2,y2)  =  -1.0;

	if(!ctrl_val)
	{
		B(y1,x) = 1.0;
   		B(y2,x) = 0.0;
	}
	else
	{
		B(y1,x) = 0.0;
   		B(y2,x) = 1.0;
	}

    add_pre_solve_method(SCA_VMPTR(sca_demux::update_ctrl));
    add_post_solve_method(SCA_VMPTR(sca_demux::read_ctrl));
}


} //namespace sca_tdf
} //namespace sca_lsf

