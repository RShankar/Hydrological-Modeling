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

 ma_util.h - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ma_util.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


/* /// C Header ///////////////////////////////////////// -*- Mode: C -*- /// */
/*
 *  Copyright (C) 1999 by Fraunhofer-Gesellschaft. All rights reserved.
 *  Fraunhofer Institute for Integrated Circuits, Design Automation Department
 *  Zeunerstraße 38, D-01069 Dresden, Germany (http://www.eas.iis.fhg.de)
 *
 *  Permission is hereby granted, without written agreement and without
 *  license or royalty fees, to use, copy, modify, and distribute this
 *  software and its documentation for any purpose, provided that the
 *  above copyright notice and the following two paragraphs appear in all
 *  copies of this software.
 *
 *  IN NO EVENT SHALL THE FRAUNHOFER INSTITUTE FOR INTEGRATED CIRCUITS
 *  BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 *  CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 *  DOCUMENTATION, EVEN IF THE FRAUNHOFER INSTITUTE HAS BEEN ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  THE FRAUNHOFER INSTITUTE FOR INTEGRATED CIRCUITS SPECIFICALLY DISCLAIMS
 *  ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 *  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE FRAUNHOFER INSTITUTE
 *  HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 *  ENHANCEMENTS, OR MODIFICATIONS.
 *
 *  Project :  KOSIM - MA
 *  File    :  ma_util.h
 *  Purpose :  header file for utilities of common mathematical purposes
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: ma_util.h 969 2010-03-01 21:34:37Z karsten $
 *
 */


#ifndef _ma_util_h_
#define _ma_util_h_


/* /// Headers ////////////////////////////////////////////////////////////// */

#include "ma_typedef.h"




void  MA_ProductValueMatrix(value* result,value factor,value* origin,count_near lin,count_near col);       /* multiplies a vector by
                                                   a value c = value * a */
void  MA_SumMatrixWeighted(value* C,value fa,value* A,value fb,value* B,count_near lin,count_near col);        /* calculates the weighted sum
                                                   C = value1 * a + value2 * b */

#endif /* _ma_util_h_ */


/* /// end of file ////////////////////////////////////////////////////////// */
