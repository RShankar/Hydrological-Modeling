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

 ana_reinit.c - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 1148 $
 SVN last checkin  :  $Date: 2011-02-06 15:25:23 +0100 (Sun, 06 Feb 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ana_reinit.c 1148 2011-02-06 14:25:23Z karsten $

 *****************************************************************************/

/*****************************************************************************/


/*#define DEBUG*/
#include <stdio.h>
#include <malloc.h>

#include "ma_typedef.h"
#include "ma_util.h"
#include "ma_sparse.h"
#include "ana_solv_data.h"

int ana_init (double*,double*,unsigned long,double,sca_solv_data**,int);

void ana_get_error_position(sca_solv_data* data,long* row,long* column)
{
	(*row)=data->critical_row;
	(*column)=data->critical_column;
}

int ana_reinit (
   double* A,
   double* B,
   unsigned long size,
   double  h,
   sca_solv_data **sdatap,
   int reinit  )
{
  double hinv;
  double *W, *Z;
  sca_solv_data *sdata;
  int err;

#ifdef SCA_IMPLEMENTATION_DEBUG
 {

    int x,y;

    printf("\n");
    for(y=0;y<size;y++)
    {
       for(x=0;x<size;x++)
       {
          printf("\t%e",A[x+size*y]);
       }
       printf("\n");
    }

    printf("\n\n\n");

    for(y=0;y<size;y++)
    {
       for(x=0;x<size;x++)
       {
          printf("\t%e",B[x+size*y]);
       }
       printf("\n");
    }

 }

#endif

  if((reinit<2 || sdatap==NULL) || size!=(*sdatap)->size)
    return ana_init(A, B, size, h, sdatap, reinit);
  else
  {
	  if((*sdatap)->algorithm==EULER)  /*if there was no timestep before -> we must reinit all */
	  {
		  return ana_init(A, B, size, h, sdatap, 1);
	  }
    sdata = *sdatap;
  }

  Z    = (double *)calloc(size*size, (unsigned)sizeof(double));
  W    = (double *)calloc(size*size, (unsigned)sizeof(double));


  hinv = 2.0/h;
  sdata->h = h;

  MA_SumMatrixWeighted(Z, hinv, A, 1.0, B, size, size);
  MA_ProductValueMatrix(W, hinv, A, size, size);

  MA_ConvertFullToSparse(Z, size, sdata->sZ_trapez, 0);
  MA_ConvertFullToSparse(W, size, sdata->sW_trapez, 0);


  free(W);
  free(Z);


  err = MA_LequSparseCodegen(sdata->sZ_trapez, sdata->code_trapez);

  if(err)
  {
 	 sdata->critical_column=sdata->code_trapez->critical_column;
 	 sdata->critical_row=sdata->code_trapez->critical_line;
 	 return(err);
  }

  return err;

}

