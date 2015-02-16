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

 sca_lsf_signal.h - linear signal flow signal

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 05.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_lsf_signal.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.2.1.2.
 The class sca_lsf::sca_signal shall define a primitive channel for the
 LSF MoC. It shall be used for connecting modules of class sca_lsf::sca_module
 using ports of class sca_lsf::sca_in and sca_lsf::sca_out.
 There shall not be any application-specific access to the associated interface.
 */

/*****************************************************************************/

#ifndef SCA_LSF_SIGNAL_H_
#define SCA_LSF_SIGNAL_H_

namespace sca_lsf
{

//class sca_signal : protected implementation-derived-from sca_core::sca_signal_if,
//                                                         sca_core::sca_prim_channel
class sca_signal: virtual public sca_core::sca_implementation::sca_conservative_signal,
                  virtual public sca_lsf::sca_signal_if
{
public:
	sca_signal();
	explicit sca_signal(const char*);

	virtual const char* kind() const;

private:
	// Disabled
	sca_signal(const sca_lsf::sca_signal&);
	//begin implementation specific

	long get_node_number() const;

	virtual bool trace_init(
				sca_util::sca_implementation::sca_trace_object_data& data);

	//is called after cluster was calculated
	virtual void trace(long id,
				sca_util::sca_implementation::sca_trace_buffer& trace_buffer);

	sca_lsf::sca_module* lsf_module;

	// end implementation specific

};

} // namespace sca_lsf


#endif /* SCA_LSF_SIGNAL_H_ */
