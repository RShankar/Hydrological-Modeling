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

 sca_vcd_trace_file.h - functions for creatin/closing sca-vcd trace files

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 08.03.2009

 SVN Version       :  $Revision: 1035 $
 SVN last checkin  :  $Date: 2010-07-22 13:33:16 +0200 (Thu, 22 Jul 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_vcd_trace_file.h 1035 2010-07-22 11:33:16Z karsten $

 *****************************************************************************/
/*
 * LRM clause 6.1.1.3.
 * The function sca_util::sca_create_vcd_trace_file shall create a new file
 * handle object of class sca_util::sca_trace_file, open a new VCD file
 * associated with the file handle, and return a pointer to the file handle. If
 * the file name has no ".vcd" extension, the file name shall be constructed by
 * appending the character string ".vcd" to the character string passed as an
 * argument to the function.
 * NOTE:
 *       VCD trace file functionality shall be supported for the tracing of
 *       regular SystemC signals with the addition to trace AMS signals,
 *       voltages across electrical nodes and currents through electrical
 *       primitives.
 *
 * LRM clause 6.1.1.4.
 * The function sca_util::sca_close_vcd_trace_file shall close the VCD file and
 * delete the file handle pointed to by the argument.
 */

/*****************************************************************************/

#ifndef SCA_VCD_TRACE_FILE_H_
#define SCA_VCD_TRACE_FILE_H_

namespace sca_util
{

//LRM clause 6.1.1.3.
sca_util::sca_trace_file* sca_create_vcd_trace_file(const char* name);

sca_util::sca_trace_file* sca_create_vcd_trace_file(std::ostream& os);

//LRM clause 6.1.1.4.
void sca_close_vcd_trace_file(sca_util::sca_trace_file* tf);


}

#endif /* SCA_VCD_TRACE_FILE_H_ */
