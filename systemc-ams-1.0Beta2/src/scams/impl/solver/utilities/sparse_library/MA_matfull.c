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

 MA_matfull.c - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: MA_matfull.c 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


/* /// C Module ///////////////////////////////////////// -*- Mode: C -*- /// */
/*
 *  Copyright (C) 1999 by Fraunhofer-Gesellschaft. All rights reserved.
 *  Fraunhofer Institute for Integrated Circuits, Design Automation Department
 *  Zeunerstra?e 38, D-01069 Dresden, Germany (http://www.eas.iis.fhg.de)
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
 *  File    :  MA_matfull.c
 *  Purpose :  procedures for matrix operations
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: MA_matfull.c 969 2010-03-01 21:34:37Z karsten $
 *
 */


#include "ma_util.h"


/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function multiplies a Matrix by a factor.
 *
 *  @param   result     resulting matrix
 *  @param   factor     factor (value)
 *  @param   origin     matrix before multiplication
 *  @param   lin      lines
 *  @param   col      columnes
 *  @return nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matfull.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

void MA_ProductValueMatrix(value* result,value factor,value* origin,count_near lin,count_near col)
{
   count_far i,len;
   /* printf("lin = %ld, col = %ld \n",(long int)lin,(long int)col);*/
   len = (count_far)lin * (count_far)col;
   for (i = 0; i < len; i++) *(result + i) = *(origin + i) * factor;
   /*printf("danach\n"); */
}
/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function calculates the weighted sum C = fa * A + fb * B.
 *
 *  @param   C     resulting matrix
 *  @param   fa    factor 1 (value)
 *  @param   A     matrix 1
 *  @param   fb    factor 2 (value)
 *  @param   B     matrix 2
 *  @param   lin   lines
 *  @param   col   columnes
 *  @return nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matfull.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

void MA_SumMatrixWeighted(value* C,value fa,value* A,value fb,value* B,count_near lin,count_near col)
{
   count_far i,len;
   len = (count_far)lin * (count_far)col;
   if (fa == 1.0)
   {
      if (fb == 1.0)
      {
         for (i = 0; i < len; i++) *(C + i) = *(A + i) + *(B + i);
      }
      else
      {
         for (i = 0; i < len; i++) *(C + i) = *(A + i) + *(B + i) * fb;
      }
   }
   else
   {
      if (fb == 1.0)
      {
         for (i = 0; i < len; i++) *(C + i) = *(A + i) * fa + *(B + i);
      }
      else
      {
         for (i = 0; i < len; i++) *(C + i) = *(A + i) * fa + *(B + i) * fb;
      }
   }

}
/* ////////////////////////////////////////////////////////////////////////// */


/* /// end of file ////////////////////////////////////////////////////////// */
