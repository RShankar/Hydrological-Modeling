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

  sca_conservative_signal.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 04.11.2009

   SVN Version       :  $Revision: 1041 $
   SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_conservative_signal.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_CONSERVATIVE_SIGNAL_H_
#define SCA_CONSERVATIVE_SIGNAL_H_

namespace sca_core
{
namespace sca_implementation
{


class sca_conservative_signal :
	public sca_core::sca_prim_channel,
	virtual public sca_core::sca_implementation::sca_conservative_interface
{
 public:

    sca_conservative_signal();
    explicit sca_conservative_signal(const char* _name);

    virtual const char* kind() const;

    bool ignore_node() const;

    virtual long get_node_number() const;

 protected:


    friend class sca_conservative_view;

    long node_number;
    long equation_id;
    bool reference_node;
    long cluster_id;
    bool connects_cviews;

 private:

    void construct();

};

}
}


#endif /* SCA_CONSERVATIVE_SIGNAL_H_ */
