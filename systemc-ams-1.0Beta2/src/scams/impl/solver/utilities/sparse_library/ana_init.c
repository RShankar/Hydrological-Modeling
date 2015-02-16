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

 ana_init.c - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 1108 $
 SVN last checkin  :  $Date: 2011-01-09 12:31:33 +0100 (Sun, 09 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ana_init.c 1108 2011-01-09 11:31:33Z karsten $

 *****************************************************************************/

/*****************************************************************************/

/*#define DEBUG*/


#include <malloc.h>


#include "ma_util.h"
#include "ma_sparse.h"
#include "ana_solv_data.h"

int ana_init (
   double* A,              /* sqaure matrix A */
   double* B,              /* sqaure matrix B */
   unsigned long size,     /* number of equations -> dim of A and B */
   double  h,              /* constant time step */
   sca_solv_data **sdatap, /* internal solver data */
   int reinit              /* reinit -> hold states */
   )
{
  double hinv;
  sca_solv_data *sdata;
  int err;

  int force_init;


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

  /*** common data ******/
  if(!reinit)
  {
     sdata =(sca_solv_data*)malloc(sizeof(sca_solv_data));
     *sdatap=sdata;
     sdata->size=0;
  }
  else
  {
     sdata=*sdatap;
  }


  sdata->critical_column = -1;
  sdata->critical_row    = -1;

  force_init = (reinit==0) || (size!=sdata->size);

  if(force_init && (sdata->size!=0))
  {
    free(sdata->xp);
    free(sdata->x_last);
    free(sdata->r1);
    free(sdata->r2);
    free(sdata->A);
    free(sdata->B);
    free(sdata->sZ_euler);
    free(sdata->sW_euler);
    free(sdata->code_euler);
    free(sdata->sZ_trapez);
    free(sdata->sW_trapez);
    free(sdata->code_trapez);
    free(sdata->W);
    free(sdata->Z);
  }

  sdata->h     = h;
  sdata->size  = size;

  if(force_init)
  {
     sdata->xp    = (double *)calloc(size,(unsigned)sizeof(double));
     sdata->x_last= (double *)calloc(size,(unsigned)sizeof(double));
     sdata->r1    = (double *)calloc(size,(unsigned)sizeof(double));
     sdata->r2    = (double *)calloc(size,(unsigned)sizeof(double));

     sdata->A     =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));
     sdata->B     =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));

     sdata->W     =(double *)calloc(size*size,(unsigned)sizeof(double));
     sdata->Z     =(double *)calloc(size*size,(unsigned)sizeof(double));


     MA_InitSparse(sdata->A);
     /* MA_InitSparse(sdata->B); */

  }

  if(reinit<2) sdata->algorithm=EULER;   /* start/restart with Euler backward method */
  else         sdata->algorithm=TRAPEZ;  /* if only h changes keep trapezoidal method */


  MA_ConvertFullToSparse(A, size, sdata->A, 0);
  /* MA_ConvertFullToSparse(B, size, sdata->B, 0); */


  /**** initialization for Euler backward method ******/

  if(force_init)
  {
     sdata->sZ_euler  =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));
     sdata->sW_euler  =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));
     sdata->code_euler=(struct spcode*)calloc(1,(unsigned)sizeof(struct spcode));

     MA_InitSparse(sdata->sZ_euler);
     MA_InitSparse(sdata->sW_euler);
     MA_InitCode(sdata->code_euler);
  }


  if(reinit<2)
  {
     hinv = 1.0/h;
     MA_SumMatrixWeighted(sdata->Z, hinv, A, 1.0, B, size, size);
     MA_ProductValueMatrix(sdata->W, hinv, A, size, size);

     MA_ConvertFullToSparse(sdata->Z, size, sdata->sZ_euler, 0);
     MA_ConvertFullToSparse(sdata->W, size, sdata->sW_euler, 0);

     err=MA_LequSparseCodegen(sdata->sZ_euler,sdata->code_euler);
     if(err)
     {
    	 sdata->critical_column=sdata->code_euler->critical_column;
    	 sdata->critical_row=sdata->code_euler->critical_line;
    	 return(err);
     }
  }


  /*** initialization for trapezoidal method *******/

  if(force_init)
  {
     sdata->sZ_trapez  =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));
     sdata->sW_trapez  =(struct sparse*)calloc(1,(unsigned)sizeof(struct sparse));
     sdata->code_trapez=(struct spcode*)calloc(1,(unsigned)sizeof(struct spcode));

     MA_InitSparse(sdata->sZ_trapez);
     MA_InitSparse(sdata->sW_trapez);
     MA_InitCode(sdata->code_trapez);
  }


  hinv = 2.0/h;
  MA_SumMatrixWeighted(sdata->Z, hinv, A, 1.0, B, size, size);
  MA_ProductValueMatrix(sdata->W, hinv, A, size, size);

  MA_ConvertFullToSparse(sdata->Z, size, sdata->sZ_trapez, 0);
  MA_ConvertFullToSparse(sdata->W, size, sdata->sW_trapez, 0);

  err=MA_LequSparseCodegen(sdata->sZ_trapez,sdata->code_trapez);

  if(err)
  {
 	 sdata->critical_column=sdata->code_trapez->critical_column;
 	 sdata->critical_row=sdata->code_trapez->critical_line;
 	 return(err);
  }

  /*************************************************************/

  return(0);


}

