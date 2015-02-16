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

 sca_assign_to_proxy.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 05.02.2010

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_assign_to_proxy.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_ASSIGN_TO_PROXY_H_
#define SCA_ASSIGN_TO_PROXY_H_

namespace sca_core
{

template<class T, class TV>
class sca_assign_to_proxy
{
public:
	sca_assign_to_proxy<T,TV>& operator=(const TV& val);

//begin implementation specific
	T* obj;
	unsigned long index;

//end implementation specific

};

//begin implementation specific

template<class T, class TV>
inline sca_assign_to_proxy<T,TV>& sca_assign_to_proxy<T,TV>::operator=(const TV& val)
{
	obj->write(val,index);
	return *this;
}

//end implementation specific

} //namespace sca_core
#endif /* SCA_ASSIGN_TO_PROXY_H_ */
