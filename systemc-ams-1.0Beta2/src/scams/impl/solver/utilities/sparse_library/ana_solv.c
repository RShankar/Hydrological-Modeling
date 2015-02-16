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

 ana_solv.c - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 977 $
 SVN last checkin  :  $Date: 2010-03-04 20:23:45 +0100 (Thu, 04 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ana_solv.c 977 2010-03-04 19:23:45Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "ana_solv_data.h"
#include "ma_typedef.h"
#include "ma_util.h"
#include "ma_sparse.h"
#include <string.h>

void ana_solv (
   double* q,       /* time dependent vector */
   double* x,       /* state vector */
   sca_solv_data* sdata /* internal solver data */
   )
{
   double hinv;
   double *r1, *r2;
   unsigned long size;
   unsigned long i;
   double *xp, *x_last;



   r1    = sdata->r1;
   r2    = sdata->r2;
   size  =sdata->size;
   xp    =sdata->xp;
   x_last=sdata->x_last;

   /* 1. */
   memmove(x_last, x, size*sizeof(double));

   if(sdata->algorithm==EULER)
   {  /******* Euler backward ********/


      /* 2. */
      MA_ProductSparseVector(sdata->sW_euler, x, r1);
      /*MA_DifferenceVector(r, q, r, size);*/
      for(i=0;i<size;++i) r1[i] -= q[i];


      /* 3. */
      MA_LequSparseSolut(sdata->sZ_euler, sdata->code_euler, r1, x);

      hinv  = 1/sdata->h;

      for(i=0;i<size;++i) xp[i]=(x[i]-x_last[i])*hinv; /*derivation*/

      sdata->algorithm=TRAPEZ;
   }
   else
   {  /*** trapezoidal method ****/

      MA_ProductSparseVector(sdata->sW_trapez, x, r1);     /* W*x(i-1) */
      MA_ProductSparseVector(sdata->A, xp, r2);            /* A*xp(i-1) */

      for(i=0;i<size;++i)	r1[i] += r2[i] - q[i];     /* W*x(i-1) + A*xp(i-1) - q(i) */

      MA_LequSparseSolut(sdata->sZ_trapez, sdata->code_trapez, r1, x);

      hinv=2.0/sdata->h;
      for(i=0;i<size;++i) xp[i]=hinv*(x[i]-x_last[i])-xp[i]; /*new derivation*/
   }



}

