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

  sca_synchronization_layer_process.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 26.08.2009

   SVN Version       :  $Revision: 995 $
   SVN last checkin  :  $Date: 2010-05-21 13:59:51 +0200 (Fri, 21 May 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_synchronization_layer_process.h 995 2010-05-21 11:59:51Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_SYNCHRONIZATION_LAYER_MODULE_H_
#define SCA_SYNCHRONIZATION_LAYER_MODULE_H_


#include <systemc>

#include "scams/impl/synchronization/sca_synchronization_alg.h"

namespace sca_core
{
namespace sca_implementation
{


class sca_synchronization_layer_process
{

public:

  sca_synchronization_layer_process(
                    sca_synchronization_alg::sca_cluster_objT* ccluster
                    );

  ~sca_synchronization_layer_process();

 protected:

  void cluster_process();

 private:

      sca_synchronization_alg::sca_cluster_objT* cluster;

      sc_core::sc_time*  cluster_period;
      sc_core::sc_time*  cluster_start_time;
      sc_core::sc_event* cluster_resume_event;
      sc_core::sc_time*  next_cluster_start_time;
      sc_core::sc_time*  next_cluster_period;
      std::vector<sca_util::sca_implementation::sca_trace_object_data*>* cluster_traces;

        bool*    request_new_period;
        bool*    request_new_start_time;

       sc_core::sc_event time_out_event;

        std::vector<const ::sc_core::sc_event*>* reactivity_events;

        bool    cp_backup_avail;
        sc_core::sc_time cp_backup;
        sc_core::sc_time last_start_time;


};


}
}


#endif /* SCA_SYNCHRONIZATION_LAYER_MODULE_H_ */
