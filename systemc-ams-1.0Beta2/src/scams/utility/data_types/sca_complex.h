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

 sca_complex.h - data type for complex numbers

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 08.03.2009

 SVN Version       :  $Revision: 1041 $
 SVN last checkin  :  $Date: 2010-08-17 16:20:03 +0200 (Tue, 17 Aug 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_complex.h 1041 2010-08-17 14:20:03Z karsten $

 *****************************************************************************/
/*
 * LRM clause 6.2.1.1.
 * The type sca_util::sca_complex shall provide a type for complex numbers.
 */

/*****************************************************************************/

#ifndef SCA_COMPLEX_H_
#define SCA_COMPLEX_H_

namespace sca_util
{

typedef std::complex<double> sca_complex;

} // namespace sca_util

#endif /* SCA_COMPLEX_H_ */
