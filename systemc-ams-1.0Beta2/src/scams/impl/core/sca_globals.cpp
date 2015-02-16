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

  sca_globals.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 26.08.2009

   SVN Version       :  $Revision: 1039 $
   SVN last checkin  :  $Date: 2010-08-17 11:11:52 +0200 (Tue, 17 Aug 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_globals.cpp 1039 2010-08-17 09:11:52Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc>
#include "scams/core/sca_time.h"
#include "scams/impl/core/sca_globals.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/core/sca_object_manager.h"

namespace sca_core
{

namespace sca_implementation
{



void sca_terminate()
{
#if SYSTEMC_VERSION >= 20041012
    if (::sc_core::sc_start_of_simulation_invoked() && ! ::sc_core::sc_end_of_simulation_invoked())
        ::sc_core::sc_get_curr_simcontext()->end();
#else

    std::ostringstream str;
    str << "Warning: SystemC-AMS post_proc routines not activated for "
    " for SystemC < 2.1 -> update SystemC to 2.1" << endl;
    SC_REPORT_WARNING("SystemC-AMS",str.str.c_str());
#endif
}

void sca_systemc_ams_init()
{

    sca_object_manager *obj_manager;
    obj_manager = sca_get_curr_simcontext()->get_sca_object_manager();

    obj_manager->systemc_ams_init();
}



namespace sca_statistics
{

unsigned long statistic_mask(~0);

unsigned long sca_statistics_mask_cnt = 0;


// By default, consts have internal linkage. To ensure external linkage,
// declare the const explicitly by the specifier extern.

extern const unsigned long view = 1<<sca_statistics_mask_cnt++;
extern const unsigned long sdf_cluster = 1<<sca_statistics_mask_cnt++;
extern const unsigned long linear_solver = 1<<sca_statistics_mask_cnt++;
extern const unsigned long all = 0;



void sca_statistics_on(unsigned long mask)
{
	statistic_mask=mask;
}

void sca_statistics_off(unsigned long mask)
{
	statistic_mask=~mask;
}

}




}
}
