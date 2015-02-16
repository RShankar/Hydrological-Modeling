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

  sca_solve_ac_linear.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 02.01.2010

   SVN Version       :  $Revision: 1107 $
   SVN last checkin  :  $Date: 2011-01-09 12:20:46 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_solve_ac_linear.cpp 1107 2011-01-09 11:20:46Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "scams/impl/analysis/ac/sca_solve_ac_linear.h"

#include <stdio.h>
#include <malloc.h>

extern "C"
{
#include "scams/impl/solver/utilities/sparse_library/ma_typedef.h"
#include "scams/impl/solver/utilities/sparse_library/ma_sparse.h"
}

namespace sca_ac_analysis
{
namespace sca_implementation
{

struct sca_solve_ac_linear_data
{
    struct sparse Spa;
    struct spcode Cod;

    long critical_row;
    long critical_column;

};

void sca_solve_ac_get_error_position(sca_solve_ac_linear_data* data,long* row,long* column)
{
	(*row)=data->critical_row;
	(*column)=data->critical_column;
}

int sca_solve_ac_linear_init(  double* A,
                               unsigned long n,
                               sca_solve_ac_linear_data** data
                           )
{
    if((*data)==NULL)
    {
        (*data)=new sca_solve_ac_linear_data;

        MA_InitSparse(&((*data)->Spa));
        MA_InitCode(&((*data)->Cod));
    }
    else
    {
        MA_FreeSparse(&((*data)->Spa));
        MA_FreeCode(&((*data)->Cod));

        (*data)->critical_column=-1;
        (*data)->critical_row=-1;
    }

    struct sca_solve_ac_linear_data* sdata=(*data);

    if (MA_ConvertFullToSparse(A, n, &(sdata->Spa), (count_far)0) != 0)
    {
        //printf("solveLSG reports: Error in creating sparse matrix!\n");
        return 1;
    }
    if (MA_LequSparseCodegen(&(sdata->Spa), &(sdata->Cod)) != 0)
    {
        //printf("solveLSG reports: Error in creating SparseCode!\n");

    	(*data)->critical_column=sdata->Cod.critical_column;
    	(*data)->critical_row=sdata->Cod.critical_line;


        return 2;
    }

    return 0;

}

int sca_solve_ac_linear(  double* B,
                       double* x,
                       struct sca_solve_ac_linear_data** data
                    )
{
    return MA_LequSparseSolut(&((*data)->Spa), &((*data)->Cod), B, x);
}


void sca_solve_ac_linear_free(struct sca_solve_ac_linear_data** data)
{
    if((*data)!=NULL)
    {
        MA_FreeSparse(&((*data)->Spa));
        MA_FreeCode(&((*data)->Cod));
    }
    delete (*data);
    (*data)=NULL;
}



} // namespace sca_implemantation
} // namespace sca_ac_analysis

