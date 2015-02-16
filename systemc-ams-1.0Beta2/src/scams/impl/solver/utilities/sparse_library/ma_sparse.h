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

 ma_sparse.h - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: ma_sparse.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/



/* /// C Header ///////////////////////////////////////// -*- Mode: C -*- /// */
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
 *  File    :  ma_sparse.h
 *  Purpose :  sparse matrix linear equation solver
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: ma_sparse.h 969 2010-03-01 21:34:37Z karsten $
 *
 */

#ifndef _ma_sparse_h_
#define _ma_sparse_h_

/* /// Headers ////////////////////////////////////////////////////////////// */

#include "ma_typedef.h"


/* /// Defines ////////////////////////////////////////////////////////////// */

#define exportMA_Sparse

/* /// Data Structures ////////////////////////////////////////////////////// */


struct sparse                /* sparse matrix definition using   */
                             /* Compressed Row Storage           */
{     count_far  nd;         /* dimension matrix                 */
      count_far  nel;        /* number of nonzero fill-ins       */
      count_far  nmax;       /* allocated maximum of fill-ins    */
      value      *a;         /* pointer to values                */
      count_near *ja;        /* column position within line      */
      count_far  *ia;        /* pointer to start of new line,
                                nd+1 elements, last el. is nel   */
      count_far  *fa;        /* pointer to next el. in line,
                                -1 in case of end of line        */
      unsigned   full : 1;   /* if 1 then a and nd used for full
                                matrix representation
                                if 0 then sparse representation  */
      unsigned   decomp : 1; /* if 1 then a is decomposed
                                if 0 then a is not decomposed    */
};

struct spcode                /* solution code for sparse matrix  */
{
      count_far  ndec;       /* length code for decomposition    */
      count_far  nsol;       /* length code for solution         */
      count_far  nsoldec;    /* length code for solution, decom-
                                position steps only,
                                without backward substitution    */
      count_far  ndecmax;    /* maximum length of decomp. code   */
      count_far  nsolmax;    /* maximum length of solution code  */
      count_far  sparlength; /* necessary length of sparse matrix*/
      count_far  rank;       /* rank of matrix                   */
      count_far  *dec;       /* decomposition code               */
      count_far  *sol;       /* solution code                    */
      count_far  *isort;     /* vector for sorting solution after
                                within lequsparse_solut          */
      count_far  *zerodemand;/* positions of right hand side after
                                decomposition, which has to be zero
                                in case of a singular matrix, if
                                solutions are possible */
      /* line and column at which singularity occurs */
      count_near  critical_line;
      count_near  critical_column;


      /* parameters */
      value  piv_abs_tol;    /* absolute tol. for finding pivot (egpiva) */
      value  piv_rel_tol;    /* relative tol. for finding pivot (egpivr) */
      value  gener_piv_scope;/* pivot range in order to limit new fill ins (egpivsw) */
      value  decomp_piv_scope;/* range for singul. check during decomp. (egpivl) */
      debug  SparseDebug;    /* debug control */
      char *(*name_of_variable)();
      char *(*name_of_equation)();



      /* statistics */
      count_far  fill_ins;   /* original number of fill ins */
      count_far  fill_ins_after_dec;
      count_far  dec_code_length;
      count_far  sol_code_length;

};



exportMA_Sparse void       MA_InitSparse(struct sparse* sA);      /*MA_matspars.c*/
exportMA_Sparse err_code   MA_ConvertFullToSparse(value* A,count_near n,struct sparse* sA,count_far maxlen); /*MA_generate_sparse.c*/
exportMA_Sparse void       MA_InitCode(struct spcode* code);            /*MA_matspars.c*/
exportMA_Sparse err_code   MA_LequSparseCodegen(struct sparse* sA,struct spcode* code);   /*MA_lequspar.c*/
exportMA_Sparse err_code   MA_ProductSparseVector(struct sparse* sA,value* pb,value* pc); /*MA_matspars.c*/
exportMA_Sparse err_code   MA_LequSparseSolut(struct sparse* sA,struct spcode* code,value* r,value* x);     /*MA_lequspar.c*/
exportMA_Sparse void       MA_FreeSparse(struct sparse* sA);          /*MA_matspars.c*/
exportMA_Sparse err_code   MA_ReallocSparse(struct sparse* sA);      /*MA_matspars.c*/
exportMA_Sparse void       MA_FreeCode(struct spcode  *code);



#endif /* _ma_sparse_h_ */

/* /// end of file ////////////////////////////////////////////////////////// */
