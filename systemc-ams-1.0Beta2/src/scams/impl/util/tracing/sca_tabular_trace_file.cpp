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

 sca_tabular_trace_file.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 16.11.2009

 SVN Version       :  $Revision: 1027 $
 SVN last checkin  :  $Date: 2010-06-03 20:17:10 +0200 (Thu, 03 Jun 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tabular_trace_file.cpp 1027 2010-06-03 18:17:10Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/utility/tracing/sca_tabular_trace_file.h"
#include "scams/impl/util/tracing/sca_tabular_trace.h"

namespace sca_util
{

sca_util::sca_trace_file* sca_create_tabular_trace_file(const char* name)
{
   return new sca_util::sca_implementation::sca_tabular_trace(name);
}


sca_util::sca_trace_file* sca_create_tabular_trace_file(std::ostream& os)
{
	return new sca_util::sca_implementation::sca_tabular_trace(os);
}

//LRM clause 6.1.1.6.
void sca_close_tabular_trace_file(sca_util::sca_trace_file* tf)
{
	if(tf!=NULL) tf->close();
}

} // namespace sca_util

