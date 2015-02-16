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

 MA_matspars.c - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/

/* /// C Module ///////////////////////////////////////// -*- Mode: C -*- /// */
/*
 *  Copyright (C) 1999 by Fraunhofer-Gesellschaft. All rights reserved.
 *  Fraunhofer Institute for Integrated Circuits, Design Automation Department
 *  Zeunerstrasse 38, D-01069 Dresden, Germany (http://www.eas.iis.fhg.de)
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
 *  File    :  MA_matspars.c
 *  Purpose :  procedures for sparse matrix technique,  lin. equation solver
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

/*#define NOT_USED*/

/* /// Headers ////////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <malloc.h>
#include "ma_sparse.h"


/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function initializes dimensions and pointers of struct sparse elements to zero.
 *
 *  @param     sA      sparse matrix (struct sparse)
 *  @return    nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse void MA_InitSparse(struct sparse* sA)
{  sA->nd     = sA->nel = sA->nmax = 0;
 sA->a      = NULL;
 sA->ja     = NULL;
 sA->ia     = NULL;
 sA->fa     = NULL;
 sA->full   = 0;
 sA->decomp = 0;
}

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function  destructs a sparse matrix, sets pointers to zero.
 *
 *  @param      sA    sparse matrix  (struct sparse)
 *  @return     nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse void MA_FreeSparse(struct sparse* sA)
{  sA->nd = sA->nel = sA->nmax = 0;
 if (sA->a != NULL)
 { free( sA->a); sA->a = NULL;
 }
 if (sA->ja != NULL)
 { free( sA->ja); sA->ja = NULL;
 }
 if (sA->ia != NULL)
 { free( sA->ia); sA->ia = NULL;
 }
 if (sA->fa != NULL)
 { free( sA->fa); sA->fa = NULL;
 }
 sA->full = 0;
 sA->decomp = 0;
}


/**
 *  This function reallocs a sparse matrix.
 *
 *  @param     sA     sparse matrix (struct sparse)
 *  @return
 *  <ul><li>   0 -  o.k.
 *  <li>       1 - reallocation of NULL pointer
 *  <li>       2 - no more memory
 *  </ul>
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse err_code MA_ReallocSparse(struct sparse* sA)
{  sA->nmax += sA->nd * 2 + 10;

 if (sA->a != NULL)
 {  sA->a = (value *)realloc((char *)sA->a,
                             (unsigned)(sA->nmax * sizeof(value)) );
 if (sA->a == NULL) return 2;
 }
 else
 {  return 1;
 }

 if (sA->ja != NULL)
 {  sA->ja = (count_near *)realloc((char *)sA->ja,
                                   (unsigned)(sA->nmax * sizeof(count_near)) );
 if (sA->ja == NULL) return 2;
 }
 else
 {  return 1;
 }

 if (sA->ia != NULL)
 {  sA->ia = (count_far *)realloc((char *)sA->ia,
                                  (unsigned)(sA->nmax * sizeof(count_far)) );
 if (sA->ia == NULL) return 2;
 }
 else
 {  return 1;
 }

 if (sA->fa != NULL)
 {  sA->fa = (count_far *)realloc((char *)sA->fa,
                                  (unsigned)(sA->nmax * sizeof(count_far)) );
 if (sA->fa == NULL) return 2;
 }
 else
 {  return 1;
 }
 /* printf("  - reallocation finished  \n"); */
 return 0;
}



/**
 *  This function initializes dimensions and pointers of struct code elements to zero.
 *
 *  @param    code    sparse code   (struct spcode)
 *  @return   nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */


exportMA_Sparse void MA_InitCode(struct spcode* code)
{
   code->ndec  = code->nsol = code->nsoldec = code->rank = 0;
   code->dec                                   = NULL;
   code->sol                                   = NULL;
   code->isort                                 = NULL;
   code->zerodemand                            = NULL;
   code->critical_line = code->critical_column = -1; /* in regular case */

   /* default values */
   code->piv_abs_tol      = 1.e-15;
   code->piv_rel_tol      = 1.e-10;
   code->gener_piv_scope  = 0.1;
   code->decomp_piv_scope = 1.e-4;
   code->SparseDebug      = 0;
   code->name_of_variable = NULL;
   code->name_of_equation = NULL;

   code->fill_ins = 0;
   code->fill_ins_after_dec = 0;
   code->dec_code_length = 0;
   code->sol_code_length = 0;

}



/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  Multiplication sparse matrix with vector c := sA * b.
 *
 *  @param      sA    sparse matrix (struct sparse)
 *  @param      pb    vector b (value *)
 *  @param      pc    result vector  sA * b  (value *)
 *  @return
 *  <ul><li>    1  error
 *  <li>        0  o.k.
 *  </ul>
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse err_code MA_ProductSparseVector(struct sparse* sA,value* pb,value* pc)
{  count_far li,i,k,j;
/*----------------------- exceptions  -----------------------------*/
 if (sA->nd < 1) return 1;
 if (sA->nmax < 1) return 1;
 /*----------------------- multiplication --------------------------*/
 i = 0;                               /* pointer into sparse list */
 for (li = 0; li < sA->nd; li++)
 {  *(pc + li) = 0.0;                 /* initialization c */
 k = sA->ia[li + 1] - sA->ia[li];  /* number of elements of line */
 for (j = 0; j < k; j++)
 {  *(pc + li) += sA->a[i] * *(pb + sA->ja[i]);
 i++;
 }
 }
 return 0;
}




/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function  destructs the sparse matrix decomposition code.
 *
 *  @param      code    sparse matrix decomposition code (struct spcode)
 *  @return     nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse void MA_FreeCode(struct spcode  *code)
{  code->ndec = code->nsol = code->nsoldec = code->rank = 0;
 if (code->dec != NULL)
 { free( code->dec); code->dec = NULL;
 }
 if (code->sol != NULL)
 { free( code->sol); code->sol = NULL;
 }
 if (code->isort != NULL)
 { free( code->isort); code->isort = NULL;
 }
 if (code->zerodemand != NULL)
 { free( code->zerodemand); code->zerodemand = NULL;
 }
}

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  Composition of a square antisymmetric sparse matrix from two square sparse
 *  matrices X, Y. The resulting matrix R consists of: fx*X (first line, first row),
 *  -fy*Y (first line, second row), fy*Y (second line, first row), fx*X (second
 *  line, second row).
 *
 *
 *  @param      sR    resulting sparse matrix (struct sparse)
 *  @param      sX    partial sparse matrix (struct sparse)
 *  @param      sY    partial sparse matrix (struct sparse)
 *  @param      fx    factor for matrix sX (value)
 *  @param      fy    factor for matrix sY (value)
 *  @return
 *  <ul><li>    1  error
 *  <li>        0  o.k.
 *  </ul>
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_matspars.c 969 2010-03-01 21:34:37Z karsten $
 *
 */


/* /// end of file ////////////////////////////////////////////////////////// */

