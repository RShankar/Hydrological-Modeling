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

 sca_assign_from_proxy.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 13.05.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_assign_from_proxy.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_ASSIGN_FROM_PROXY_H_
#define SCA_ASSIGN_FROM_PROXY_H_

namespace sca_core
{

template<class T>
class sca_assign_from_proxy
{
public:
	virtual void assign_to(T& obj)=0;
	virtual ~sca_assign_from_proxy();
};

template<class T>
inline sca_assign_from_proxy<T>::~sca_assign_from_proxy()
{
}

}

#endif /* SCA_ASSIGN_PROXY_BASE_H_ */
