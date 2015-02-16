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

 ana_solv_data.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 1103 $
 SVN last checkin  :  $Date: 2011-01-08 21:11:02 +0100 (Sat, 08 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ana_solv_data.h 1103 2011-01-08 20:11:02Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef  ANA_SOLV_DATA_H
#define ANA_SOLV_DATA_H

#include "ma_typedef.h"

typedef enum sca_algE
{
   EULER =0,
   TRAPEZ=1
}sca_algT;



typedef struct sca_solv_dataS
{

      double h;
      unsigned long size;


      double *r1;      /*temporary vector*/
      double *r2;      /*temporary vector*/
      double *xp;     /*derivation of x-vector*/
      double *x_last; /*derivation of x-vector*/
      struct sparse *A;      /*A-Matrix*/
      struct sparse *B;      /*B-Matrix*/

      sca_algT   algorithm;

      double* W;
      double* Z;


      /***** data for euler *****/

      struct sparse *sZ_euler;
      struct sparse *sW_euler;
      struct spcode *code_euler;


      /***** data for trapez *****/

      struct sparse *sZ_trapez;
      struct sparse *sW_trapez;
      struct spcode *code_trapez;

      long critical_row;
      long critical_column;

} sca_solv_data;


#endif
