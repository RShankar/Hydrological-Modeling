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

 MA_generate_sparse.c - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: MA_generate_sparse.c 969 2010-03-01 21:34:37Z karsten $

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
 *  File    :  MA_generate_sparse.c
 *  Purpose :  procedures, which generate or use sparse matrix descriptions
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: MA_generate_sparse.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

/* /// Headers ////////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <malloc.h>
#include "ma_sparse.h"

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  conversion of a simple full matrix (column-wise) into sparse representation.
 *
 *  @param      A      original matrix, to change into sparse (value *)
 *  @param      n      dimension of A (count_near)
 *  @param      sA     sparse matrix (struct sparse *)
 *  @param      maxlen if maxlen is defined (>0) it is used
 *                     instead of a prophylactical maximum (count_far)
 *  @return
 *  <ul><li>    0 - o.k.
 *  <li>        1 - dimension erroneous
 *  <li>        2 - no more memory
 *  </ul>
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_generate_sparse.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse err_code MA_ConvertFullToSparse(value* A, count_near n,
		struct sparse* sA, count_far maxlen)
{
	count_far i, j, anz, nn;

	/*----------------------- exceptions  -----------------------------*/
	if (n < 1)
		return 1;

	/*----------------------- conversion  -----------------------------*/
	MA_FreeSparse(sA);
	sA->nd = (count_far) n;
	nn = sA->nd * sA->nd;

	/* number of non-zero fill ins */
	anz = 0;
	for (i = 0; i < nn; i++)
	{
		if (*(A + i) != 0.0)
			anz++;
	}
	sA->nel = anz;
	if (maxlen > 0)
	{
		if (maxlen < anz)
			return 1;
		sA->nmax = maxlen + 2;
	}
	else
	{
		sA->nmax = anz + (count_far) anz * 0.1 + 20; /* prophylactical */
	}

	sA->a = (value *) calloc((unsigned) sA->nmax, (unsigned) sizeof(value));
	sA->ja = (count_near *) calloc((unsigned) sA->nmax,
			(unsigned) sizeof(count_near));
	sA->ia = (count_far *) calloc((unsigned) (n + 2),
			(unsigned) sizeof(count_far));
	if (sA->a == NULL || sA->ja == NULL || sA->ia == NULL)
		return 2;

	sA->ia[0] = anz = 0;
	for (i = 0; i < n; i++) /* line */
	{
		sA->ia[i + 1] = sA->ia[i];
		for (j = 0; j < n; j++) /* column */
		{
			if (*(A + j * n + i) != 0.0)
			{
				sA->a[anz] = *(A + j * n + i);
				sA->ja[anz] = j;
				(sA->ia[i + 1])++;
				anz++;
			}
		}
	}
	/* MA_PrintSparse(sA,stdout); */
	return 0;
}

/* /// end of file ////////////////////////////////////////////////////////// */

