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

 sca_eln_node.h - electrical linear net node

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 07.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_eln_node.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 * LRM clause 4.3.1.3.
 * The class sca_eln::sca_node shall define a primitive channel for the ELN MoC.
 * It shall be used for connecting ELN primitive modules using ports of class
 * sca_eln::sca_terminal. There shall not be any application-specific access to
 * the associated interface. The primitive channel shall represent an electrical
 * node.
 */

/*****************************************************************************/

#ifndef SCA_ELN_NODE_H_
#define SCA_ELN_NODE_H_


namespace sca_eln
{

//  class sca_node : protected implementation-derived-from sca_eln::sca_node_if,
//                                                         sca_core::sca_prim_channel
class sca_node: virtual public sca_core::sca_implementation::sca_conservative_signal,
                virtual public sca_eln::sca_node_if
{
public:
	sca_node();
	explicit sca_node(const char* name_);

	virtual const char* kind() const;

private:
	// Disabled
	sca_node(const sca_eln::sca_node&);

//begin implementation specific

	long get_node_number() const;

	virtual bool trace_init(
			sca_util::sca_implementation::sca_trace_object_data& data);

	//is called after cluster was calculated
	virtual void trace(long id,
			sca_util::sca_implementation::sca_trace_buffer& trace_buffer);

	sca_eln::sca_module* eln_module;

// end implementation specific
};

} // namespace sca_eln

#endif /* SCA_ELN_NODE_H_ */
