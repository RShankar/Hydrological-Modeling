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

 MA_lequspar.c - description

 Original Author: Christoph Clauss Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $

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
 *  File    :  MA_lequspar.c
 *  Purpose :  sparse matrix lin. equation solver
 *  Notes   :
 *  Author  :  C.Clauss
 *  Version :  $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

/* /// Headers ////////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <malloc.h>
#include "ma_sparse.h"

/* /// Private Defines ////////////////////////////////////////////////////// */
/* /// Private Data Structures ////////////////////////////////////////////// */
/* /// External Variables /////////////////////////////////////////////////// */
/* /// Prototypes of Static Functions /////////////////////////////////////// */
/* /// Prototypes of Imported Functions ///////////////////////////////////// */
/* /// External Functions /////////////////////////////////////////////////// */
/* /// Static Functions ///////////////////////////////////////////////////// */

static value absol(value v)
{
	if (v < 0.0)
		return -v;
	else
		return v;
}

/* /// External Functions /////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This internal function adds a tripel to decomposition code.
 *
 *  @param   code       sparse code
 *  @param   z1,z2,z3   tripel to add
 *  @return  0 - o.k.
 *           1 - not enough memory
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

static err_code tocodedec(struct spcode* code,count_far z1,count_far z2,count_far z3)
/*-----------------------------------------------------------------*/
{
	count_far i;
	i = code->ndec;
	code->ndec += 3;
	if (code->ndec > code->ndecmax)
	{ /* reallocation */
		code->ndecmax += 1000;
		code->dec = (count_far *) realloc((char *) code->dec,
				(unsigned) (code->ndecmax * sizeof(count_far)));
		if (code->dec == NULL)
			return 1;
	}
	code->dec[i] = z1;
	code->dec[i + 1] = z2;
	code->dec[i + 2] = z3;
	/* printf("dec %8d %8d %8d \n",(int)z1,(int)z2,(int)z3); */
	return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function adds a tripel to the solution code.
 *
 *  @param   code       sparse code
 *  @param   z1,z2,z3   tripel to add
 *  @return  0 - o.k.
 *           1 - not enough memory
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

static err_code tocodesol(struct spcode* code,count_far z1,count_far z2,count_far z3)
{
	count_far i;
	i = code->nsol;
	code->nsol += 3;
	if (code->nsol > code->nsolmax)
	{ /* reallocation */
		code->nsolmax += 300;
		code->sol = (count_far *) realloc((char *) code->sol,
				(unsigned) (code->nsolmax * sizeof(count_far)));
		if (code->sol == NULL)
			return 1;
	}
	code->sol[i] = z1;
	code->sol[i + 1] = z2;
	code->sol[i + 2] = z3;
	/* printf("sol         %8d %8d %8d \n",(int)z1,(int)z2,(int)z3); */
	return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */

/**
 *  This function generates the decomposition code, the sparse matrix is destructed.
 *
 *  @param    sA         sparse matrix
 *  @param    code       sparse decomposition code
 *  @param    debf       channel for debug output
 *  @return   return information
 *  <ul><li>       0 - o.k.
 *  <li>          1 - singular
 *  <li>          2 - dim < 1
 *  <li>          3 - not enough memory
 *  <li>          4 - wrong tolerances
 *  <li>          5 - no sparse matrix
 *  </ul>
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

/*exportMA_Sparse err_code MA_LequSparseCodegen(sA, code, debf)*/
exportMA_Sparse err_code MA_LequSparseCodegen(struct sparse* sA,struct spcode* code)
#ifdef NOT_USED
	FILE *debf; /* channel for debug output          */
#endif
/*-----------------------------------------------------------------*/
/*SparseDebug :
 0 - nothing
 1 - step, line etc. if singular
 2 - matrix after dec., code
 3 - tolerance criteria
 4 - step, pivot candidates
 5 - full information
 */
{
	count_near ret; /* return variable */
	count_far i, i1, j1 = 0, j2, k7, k8, i2, i3, k = 0, kk, j, i6, lvgl = 0,
			mi = 0, i7, m1, line, i4, jlast, coljlast, coli, /* column in pivot line */
			colj, /* column in nonpivot line */
			ipc, /* number of element in pivot column, sparse list */
			n = 0, /* dimension */
			nn, /* dimension - 1 */
			nel, /* number elements */
			jk, /* pivoting steps */
			niz, /* dimension of the matrix to be decomposed */
			pnew, /* pointer to the last element in the sparse list */
			l1, /* index of first el. in pivot line */
			mark, /* Markowitz sum */
			lz, /* pivot line   */
			l, /* pivot number in sparse list */
			ls; /* pivot column */

	count_far *ip, /* line permutation vector */
	*jp, /* column permutation vactor */
	*jz, /* Markowitz sums of pivot proposals in the line */
	*ik, /* indexes in a list of pivot proposals in the line */
	*is; /* numbers of elements in undecomposed matrix part */

	value su, w, rk, schwell, p, pmin, al; /* pivot value */

#ifdef NOT_USED
	Sollte nie auftreten, da unser Code per Definition richtig ist :-)
	if (MA_GetSparseFullDescription(sA))
	{	fprintf(debf,"System matrix used as a full description  \n");
		return 5;
	}
#endif

	sA->decomp = 1; /* note: matrix decomposed */

	ret = 0;
	if (sA->nd < 1 || sA->nel < 1)
	{
		return 2;
	}

	ip = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	jp = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	jz = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	ik = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	is = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	sA->fa = (count_far *) calloc((unsigned) sA->nmax,
			(unsigned) sizeof(count_far));

	if (ip == NULL || jp == NULL || jz == NULL || ik == NULL || is == NULL
			|| sA->fa == NULL)
	{
		ret = 3;
		goto retour;
	}

#ifdef NOT_USED
	brauchen wir nicht
	if (code->SparseDebug > 0)
	{	if (code->SparseDebug == 5)
		{	fprintf(debf,"start lequsparse_codegen  \n");
			MA_PrintSparse(sA, debf);
		}
	}
#endif

	/* Initialization */
	/* ============== */
	n = sA->nd;
	nn = n - 1;
	nel = sA->nel;

	/* Code initialization */
	code->ndecmax = code->nsolmax = 3 * 2 * nel;
	if (code->dec != NULL)
	{
		free(code->dec);
		code->dec = NULL;
	}
	if (code->sol != NULL)
	{
		free(code->sol);
		code->sol = NULL;
	}
	if (code->isort != NULL)
	{
		free(code->isort);
		code->isort = NULL;
	}
	code->dec = (count_far *) calloc((unsigned) (code->ndecmax),
			(unsigned) sizeof(count_far));
	code->sol = (count_far *) calloc((unsigned) (code->nsolmax),
			(unsigned) sizeof(count_far));
	code->isort = (count_far *) calloc((unsigned) (sA->nd + 1),
			(unsigned) sizeof(count_far));
	if (code->sol == NULL || code->dec == NULL || code->isort == NULL)
	{
		ret = 3;
		goto retour;
	}
	code->ndec = code->nsol = 0;
	code->critical_column = code->critical_line = -1; /* default regular */

	k7 = k8 = 1;
	jk = 0; /* pivoting steps */
	niz = n; /* dimension of remaining matrix, still to be decomposed */
	pnew = nel - 1;/* pointer to the last element in the sparse list */

	schwell = 1.0 / code->gener_piv_scope;

	code->fill_ins = nel; /* original number of fill ins before decomposition */

	/* calculation of fa */
	/* sA->fa: pointer to next element within line, last of line is -1; dim nel */
	for (i = 0; i < n; i++) /* line counter */
	{
		j1 = sA->ia[i]; /* pointer to start of line */
		j2 = sA->ia[i + 1] - 2; /* pointer to end of line but one */
		if (j1 <= j2)
		{
			for (i1 = j1; i1 <= j2; i1++)
				sA->fa[i1] = i1 + 1;
		}
		if(j2>-2) sA->fa[j2 + 1] = -1; /* last pointer is -1 */
		/*sA->fa[j2 + 1] = -1; */ /* last pointer is -1 */
	}

	/* ip, jp, is  - initialization */
	for (i = 0; i < n; i++)
	{
		ip[i] = jp[i] = i;
		is[i] = 0;
	}
	for (i = 0; i < nel; i++)
		is[sA->ja[i]]++;

#ifdef NOT_USED
	if (code->SparseDebug > 0)
	{	if (code->SparseDebug == 5)
		{	fprintf(debf,"\n before first step \n");
			MA_PrintSparse(sA, debf);
			for (i = 0; i < n; i++)
			fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
					(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);
		}
	}
#endif

	for (niz = n; niz > 1; niz--)
	{

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{	if (code->SparseDebug == 3 || code->SparseDebug == 4 || code->SparseDebug == 5)
			{	fprintf(debf,"\n\n\n pivot step, niz = %d  \n\n",(int)niz);
			}
		}
#endif

		kk = n - niz; /* number of finished decomposition steps */
		code->rank = kk;

		/* Pivot candidates (stored in ik), Markowitz sums (stored in jz) */
		/* ============================================================== */
		for (j = kk; j < n; j++)
		{
			i1 = ip[j]; /* line number, formerly i1 = iz[ ip[j]]; */
			if (sA->ia[i1] == sA->ia[i1 + 1])
			{ /* line is empty by structure */
				ret = 1; /* singular */

#ifdef NOT_USED
				if (code->SparseDebug > 0)
				{
					if (code->SparseDebug == 1 || code->SparseDebug == 5)
					{
						fprintf(debf,"structural singular in decomposition step %d  \n",(int)kk);
						fprintf(debf,"in line:  %6d  no pivot candidate \n",(int)i1);
						fprintf(debf,"already decomposed:  \n");
						for (i = 0; i < kk; i++)
						{
							fprintf(debf,"line:  %6d  column:  %6d\n",(int)ip[i],(int)jp[i]);
						}
					}
				}
#endif

#ifdef NOT_USED
				if (code->name_of_variable != NULL && code->name_of_equation != NULL)
				fprintf(debf,"structural singular at variable: %s,  equation: %s",
						code->name_of_variable((count_near)jp[j]),
						code->name_of_equation((count_near)ip[j]));
#endif

				code->critical_line = ip[j];
				code->critical_column = jp[j];

				/* if (j == n - 1) goto backward; goto retour; */
				ik[i1] = -1; /* no pivot candidate in this line */
				jz[i1] = 0; /* therefore, markowitz sum is zero */			}
			else
			{ /* there exist elements in the line */
				i3 = l1 = sA->ia[i1]; /* first el. in line */
				su = 0.0; /* maximum of line */
				i2 = 0; /* number of valid el. of line */

				while (i3 > -1) /* search maximum el. of line */
				{
					if (sA->ja[i3] > -1)
					{
						i2++;
						w = absol(sA->a[i3]);
						if (w > su)
						{
							su = w;
							k = i3; /* column of maximum */
						}
					}
					i3 = sA->fa[i3];
				}
				/* fprintf(debf," k  %ld  max:  %e  \n",(int)k,(float)su); */
				if (su < code->piv_abs_tol)
				{
					ret = 1; /* singular */

#ifdef NOT_USED
					if (code->SparseDebug > 0)
					{
						if (code->SparseDebug == 1 || code->SparseDebug == 5)
						{
							fprintf(debf,"numerical singular in decomposition step %d  \n",(int)kk);
							fprintf(debf,"in line:  %6d  no pivot candidate \n",(int)i1);
							fprintf(debf,"already decomposed:  \n");
							for (i = 0; i < kk; i++)
							{
								fprintf(debf,"line:  %6d  column:  %6d\n",(int)ip[i],(int)jp[i]);
							}
						}
					}
#endif

#ifdef NOT_USED
					if (code->name_of_variable != NULL && code->name_of_equation != NULL)
					fprintf(debf,"numerical singular at variable: %s,  equation: %s",
							code->name_of_variable((count_near)jp[j]),
							code->name_of_equation((count_near)ip[j]));
#endif

					code->critical_line = ip[j];
					code->critical_column = jp[j];

					/* if (j == n - 1) goto backward; goto retour; */
					ik[i1] = -1; /* no pivot candidate in this line */
					jz[i1] = 0; /* therefore, markowitz sum is zero */
				}
				else
				{
					if (i2 == 1)
					{ /* there is only one element in the line */
						ik[i1] = k;
						jz[i1] = is[sA->ja[k]];
					}
					else
					{
						pmin = 1.e20;
						i = l1; /* stepping line */
						while (i != -1)
						{ /* fprintf(debf,"element %d \n",i); */
							i6 = sA->ja[i]; /* column */
							if (i6 > -1) /* column not decomposed */
							{
								w = absol(sA->a[i]);
								if (w > code->piv_abs_tol)
								{
									rk = su / w;
								}
								else
								{
									rk = 1.e20;
								}
								if (rk <= schwell)
								{
									mark = (i2 - 1) * (is[i6] - 1);
									/* fprintf(debf,"line %d, %d el., col %d, %d el. \n",
									 (int)i1,(int)i2,(int)i6,(int)is[i6]);
									 fprintf(debf,"candidat %e mark %d \n", w, (int)mark);  */
									i7 = i2 + is[i6];
									p = mark + code->piv_rel_tol * rk; /* fprintf(debf," p %e \n", p); */
									if (p < pmin || (p == pmin && i7 < lvgl))
									{
										pmin = p;
										lvgl = i7;
										mi = mark;
										ik[i1] = i;
									}
								}
							}
							i = sA->fa[i];
						}
						jz[i1] = mi; /* stores markowitz sum */
					}
				}
			}
		}

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{	if (code->SparseDebug == 5)
			{	fprintf(debf,"after finding pivot candidates \n");
				MA_PrintSparse(sA, debf);
				for (i = 0; i < n; i++)
				fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
						(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);
			}
		}
#endif

		/* lu decomposition in actual step */
		/* =============================== */

		/* finding pivot line - minimal Markowitz sum */
		/* ========================================== */

		mark = -1;
		i2 = -1;
		for (j = kk; j < n; j++)
		{
			if (ik[ip[j]] > -1)
			{
				if (mark == -1)
				{ /* first valid pivot candidate */
					mark = jz[ip[j]];
					i2 = j;
				}
				i1 = jz[ip[j]];
				if (mark > i1)
				{ /* search for minimal Markowitz sum */
					mark = i1;
					i2 = j;
				}
			}
		}

		if (i2 == -1)
		{ /* no further pivot element found */
#ifdef NOT_USED
			if (code->SparseDebug > 0)
			{
				if (code->SparseDebug == 5)
				{
					fprintf(debf,"after  %d successfull decomposition steps",(int)kk);
					fprintf(debf," no further pivot element found \n");
				}
			}
#endif
			goto backward;
		}

		lz = ip[i2]; /* pivot line   */
		l = ik[lz]; /* pivot number in sparse list */
		ls = sA->ja[l]; /* pivot column */
		l1 = sA->ia[lz]; /* first el. pivot line in sparse list */
		al = sA->a[l]; /* pivot value */

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{
			if (code->SparseDebug == 5 || code->SparseDebug == 4)
			{
				fprintf(debf,"pivot is: %e in line %d, column %d ",al,(int)lz,(int)ls);
				fprintf(debf,"list number %d \n  first el. in pivot line is %d\n",
						(int)l,(int)l1);
			}
		}
#endif

		/* moving pivot to position kk,kk */
		ip[i2] = ip[kk];
		ip[kk] = lz; /* line */
		i1 = jp[kk];
		jp[kk] = ls;
		for (i = kk + 1; i < n; i++)
		{
			if (jp[i] == ls)
			{
				jp[i] = i1;
				break;
			}
		}

		/* modification pivot line */
		/* ======================= */
		if (absol(al) < code->piv_abs_tol)
		{
			if (al < 0)
			{
				al = -code->piv_abs_tol;
			}
			else
			{
				al = code->piv_abs_tol;
			}
		}
		al = 1.0 / al;
		sA->a[l] = al;
		if (tocodedec(code, l, -1, -1) != 0)
		{
			ret = 3;
			goto retour;
		}
		i = l1; /* stepping through pivot line */
		while (i > -1)
		{
			if (sA->ja[i] > -1 /* neglect decomposed part */
			&& i != l) /* leaving out the pivot element */
			{
				sA->a[i] *= al; /* division by pivot element */
				if (tocodedec(code, i, l, -1) != 0)
				{
					ret = 3;
					goto retour;
				}
			}
			i = sA->fa[i];
		}
		if (tocodesol(code, lz, l, -1) != 0)
		{
			ret = 3;
			goto retour;
		}

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{
			if (code->SparseDebug == 5)
			{	fprintf(debf,"after modifikation pivot line  \n");
				MA_PrintSparse(sA, debf);
				for (i = 0; i < n; i++)
				fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
						(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);
			}
		}
#endif

		/* non pivot lines */
		/* =============== */
		for (m1 = kk + 1; m1 < n; m1++)
		{
			line = ip[m1]; /* actual line */
#ifdef NOT_USED
			if (code->SparseDebug > 0)
			{
				if (code->SparseDebug == 5)
				{
					fprintf(debf, "\n actual nonpivot line %d \n", (int) line);
				}
			}
#endif

			/* stepping through line to watch its elements*/
			/*
			 fprintf(debf,"\n actual nonpivot line %d \n",(int)line);
			 i = sA->ia[line];
			 if (sA->ia[line + 1] == i) fprintf(debf," line empty \n ");
			 else
			 {
			 while(i > -1)
			 {
			 if (sA->ja[i] > -1) fprintf(debf," %e ",(double)sA->a[i]);
			 i = sA->fa[i];
			 }
			 fprintf(debf,"\n");
			 }
			 */

			/* is in line in the pivot column a nonzero element? */
			ipc = -1;
			i = sA->ia[line]; /* stepping through line */
			if (sA->ia[line + 1] != i) /* otherwise line is empty */
			{
				while (i > -1)
				{
					if (sA->ja[i] > -1) /* neglect decomposed part */
					{
						if (sA->ja[i] == ls)
						{
							ipc = i;
							sA->ja[i] = -sA->ja[i] - 1;
							break; /* column el. marked */
						}
					}
					i = sA->fa[i];
				}
			}

			if (ipc > -1)
			{ /* within pivot column of line there is a fill in */
				if (tocodesol(code, line, lz, ipc) != 0)
				{
					ret = 3;
					goto retour;
				}

				/* stepping through pivot line and through nonpivot line */
				/* jumping over pivot element */
#ifdef NOT_USED
				if (code->SparseDebug > 0)
				{
					if (code->SparseDebug == 5)
					{
						fprintf(debf,"line %d has a nonzero in the pivot column\n",
								(int)line);
					}
				}
#endif

				i = l1; /* start pivot line*/
				j = sA->ia[line]; /* start nonpivot line*/
				jlast = -1;
				coljlast = -1; /* last element in line */
				while (i > -1)
				{
					coli = sA->ja[i];
					if (coli > -1 && i != l) /* neglect dec. part and pivot */
					{ /* fprintf(debf,"column in pivot line %d\n",(int)coli); */
						/* first el. in pivot line found */

						colj = sA->ja[j];
						if (colj < 0)
							colj = -colj - 1;
						while (colj < coli)
						{
							jlast = j;
							coljlast = colj;
							j = sA->fa[j]; /* printf(" %d  \n",(int)j); */
							if (j == (count_far) -1)
								break;
							colj = sA->ja[j];
							if (colj < 0)
								colj = -colj - 1;
						}

						/*  fprintf(debf,"\n colj --> after search %d j %d    \n\n",
						 (int)colj,(int)j); */

						if (j == -1)
						{ /* end of nonpivot line */
#ifdef NOT_USED
							if (code->SparseDebug > 0)
							{
								if (code->SparseDebug == 5)
								{
									fprintf(debf," new fill in after last element, col. %d \n",
											(int)coli);
								}
							}
#endif
							pnew = sA->nel;
							sA->nel += 1; /* number nonzeros */
							sA->ia[sA->nd] += 1; /* last pointer ia */
							if (sA->nel > sA->nmax)
							{
								if (MA_ReallocSparse(sA) != 0)
								{
									ret = 3;
									goto retour;
								}
							}
							sA->ja[pnew] = coli;
							sA->fa[pnew] = -1;
							sA->fa[jlast] = pnew;
							j = pnew;
							colj = coli;
							sA->a[pnew] = -sA->a[ipc] * sA->a[i];
							is[coli] += 1; /* updating is */
							i3 = -i - 2;
							if (tocodedec(code, pnew, ipc, i3) != 0)
							{
								ret = 3;
								goto retour;
							}
						}
						else
						{
							colj = sA->ja[j];
							if (colj < 0)
								colj = -colj - 1;
							if (colj == coli)
							{ /* el. in pivot and nonpivot line */
#ifdef NOT_USED
								if (code->SparseDebug > 0)
								{
									if (code->SparseDebug == 5)
									{
										fprintf(debf," el. in piv and nonpiv line, col. %d \n",
												(int)coli);
									}
								}
#endif

								/* fprintf(debf," j %d ipc %d i %d \n",
								 (int)j,(int)ipc,(int)i);
								 */
								sA->a[j] -= sA->a[ipc] * sA->a[i];
								if (tocodedec(code, j, ipc, i) != 0)
								{
									ret = 3;
									goto retour;
								}
							}
							else
							{
								if (colj < coli)
								{
#ifdef NOT_USED
									fprintf(debf,"internal error\n");
#endif
									goto retour;
								}
								/* new fill in necessary */
								if (jlast < 0)
								{
#ifdef NOT_USED
									if (code->SparseDebug > 0)
									{
										if (code->SparseDebug == 5)
										{
											fprintf(debf," new fill in before first element \n");
										}
									}
#endif
									pnew = sA->nel;
									sA->nel += 1; /* number nonzeros */
									sA->ia[sA->nd] += 1; /* last pointer ia */
									if (sA->nel > sA->nmax)
									{
										if (MA_ReallocSparse(sA) != 0)
										{
											ret = 3;
											goto retour;
										}
									}
									sA->ja[pnew] = coli;
									sA->fa[pnew] = sA->ia[line];
									jlast = sA->ia[line] = pnew;
									coljlast = coli;
									sA->a[pnew] = -sA->a[ipc] * sA->a[i];
									is[coli] += 1; /* updating is */
									i3 = -i - 2;
									if (tocodedec(code, pnew, ipc, i3) != 0)
									{
										ret = 3;
										goto retour;
									}
								}
								else
								{
#ifdef NOT_USED
									if (code->SparseDebug > 0)
									{
										if (code->SparseDebug == 5)
										{
											fprintf(debf," new fill after element col %d \n",
													(int)coljlast);
										}
									}
#endif
									pnew = sA->nel;
									sA->nel += 1; /* number nonzeros */
									sA->ia[sA->nd] += 1; /* last pointer ia */
									if (sA->nel > sA->nmax)
									{
										if (MA_ReallocSparse(sA) != 0)
										{
											ret = 3;
											goto retour;
										}
									}
									sA->ja[pnew] = coli;
									sA->fa[pnew] = sA->fa[jlast];
									sA->fa[jlast] = pnew;
									jlast = pnew;
									coljlast = coli;
									sA->a[pnew] = -sA->a[ipc] * sA->a[i];
									is[coli] += 1; /* updating is */
									i3 = -i - 2;
									if (tocodedec(code, pnew, ipc, i3) != 0)
									{
										ret = 3;
										goto retour;
									}
								}
							}
						}
					}
					i = sA->fa[i];
				} /* end case that line has element in pivot column */
			} /* end stepping through nonpivot lines */
		}
		/* stepping through pivot line, marking */
		i = l1;
		while (i > -1)
		{
			if (sA->ja[i] > -1) /* neglect decomposed part */
			{
				is[sA->ja[i]] -= 1; /* updating is */
				sA->ja[i] = -sA->ja[i] - 1; /* marking */
			}
			i = sA->fa[i];
		}

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{
			if (code->SparseDebug == 5)
			{
				fprintf(debf,"after modifikation non pivot lines  \n");
				MA_PrintSparse(sA, debf);
				for (i = 0; i < n; i++)
				fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
						(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);
			}
		}
#endif

	} /* end niz */

	code->rank = sA->nd - niz;

	/* last pivot line */
	/* =============== */
	i = ip[nn];
	j = jp[nn];
	i1 = sA->ia[i];
	while (sA->ja[i1] != j)
	{
		i1 = sA->fa[i1]; /* index to pivot in sparse list */
		if (i1 < 0) /* singular, no pivot element */
		{
			ret = 1;

#ifdef NOT_USED
			if (code->SparseDebug > 0)
			{
				if (code->SparseDebug == 1 || code->SparseDebug == 5)
				{
					fprintf(debf,"structurally singular in last decomposition step  \n");
					fprintf(debf,"line:  %6d  column:  %6d\n",(int)i,(int)j);
				}
			}
#endif

#ifdef NOT_USED
			if (code->name_of_variable != NULL && code->name_of_equation != NULL)
			fprintf(debf,"singular at variable: %s,  equation: %s",
					code->name_of_variable((count_near)jp[nn]),
					code->name_of_equation((count_near)ip[nn]));
#endif

			code->critical_line = ip[nn];
			code->critical_column = jp[nn];

			goto backward;
			/* goto retour; */
		}
	}
	if (absol(sA->a[i1]) < code->piv_abs_tol)
	{
		ret = 1;

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{
			if (code->SparseDebug == 1 || code->SparseDebug == 5)
			{
				fprintf(debf,"numerically singular in last decomposition step  \n");
				fprintf(debf,"line:  %6d  column:  %6d\n",(int)i,(int)j);
			}
		}
#endif

		code->critical_line = i;
		code->critical_column = j;
		goto backward;
		/* goto retour; */
	}
	sA->a[i1] = 1.0 / sA->a[i1];
	sA->ja[j1] = -sA->ja[j1] - 1;
	if (tocodedec(code, i1, -1, -1) != 0)
	{
		ret = 3;
		goto retour;
	}
	if (tocodesol(code, i, i1, -1) != 0)
	{
		ret = 3;
		goto retour;
	}
	code->rank = n;

	/* ====================== */
	/* backward decomposition */
	/* ====================== */
	backward: code->nsoldec = code->nsol;

#ifdef NOT_USED
	if (code->SparseDebug > 0)
	{
		if (code->SparseDebug == 5)
		{
			fprintf(debf,"before start backward decomposition  \n");
			MA_PrintSparse(sA, debf);
			for (i = 0; i < n; i++)
			fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
					(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);
			fprintf(debf,"n  %d \n",(int)n);
		}
	}
#endif

	/* filling zerodemand in singular case */
	if (code->rank < n)
	{
		k = n - code->rank;
		if (code->zerodemand != NULL)
		{
			free(code->zerodemand);
			code->zerodemand = NULL;
		}
		code->zerodemand = (count_far *) calloc((unsigned) k,
				(unsigned) sizeof(count_far));
		if (code->zerodemand == NULL)
		{
			ret = 3;
			goto retour;
		}

		for (i = 0; i < k; i++)
			code->zerodemand[i] = ip[i + code->rank];

#ifdef NOT_USED
		if (code->SparseDebug > 0)
		{
			if (code->SparseDebug == 5)
			{
				fprintf(debf, "after decomposition the following positions ");
				fprintf(debf, "of the solution vector have to be zero  \n");
				for (i = 0; i < n - code->rank; i++)
				fprintf(debf, "%6d  ", (int) code->zerodemand[i]);
				fprintf(debf, " \n");
			}
		}
#endif
	}

	for (k = n - 2; k > -1; k--)
	{
		line = ip[k];
		j = jp[k]; /* line steps through lines of matrix */

		i4 = sA->ia[line]; /* steps through actual line in sparse list */
		do
		{
			i6 = sA->ja[i4]; /* column of actual element i4 in line */
			if (i6 < 0)
				i6 = -i6 - 1; /* all elements are watched at */
			for (i3 = k + 1; i3 < n; i3++) /* i3 steps through ready lines */
			{
				if (i6 == jp[i3]) /* test: is ready pivot in column i6 of el. i4? */
				{
					if (tocodesol(code, line, ip[i3], i4) != 0)
					{
						ret = 3;
						goto retour;
					}
				} /* if it is, in line a backward subst. step is done with the
				 pivot standing on the right hand vector in line ip[i3] */
			}
			i4 = sA->fa[i4];
		} while (i4 > -1);
	}

	code->sparlength = pnew + 1; /* using the code, this sparse length is
	 necessary */

	for (i = 0; i < n; i++)
		code->isort[ip[i]] = jp[i]; /* for back permutation
		 of solution */

	code->dec_code_length = code->ndec;
	code->sol_code_length = code->nsol;
	code->fill_ins_after_dec = pnew + 1;

	/*========================*/
	/*Actions at the very end */
	/*========================*/

	retour:

#ifdef NOT_USED

	if (code->SparseDebug > 0)
	{
		if (code->SparseDebug == 2 || code->SparseDebug == 5)
		{
			fprintf(debf,"matrix at the end of decomposition \n");
			MA_PrintSparse(sA, debf);
			for (i = 0; i < n; i++)
			fprintf(debf,"%6d   ip %6d   jp %6d   is %6d   jz %6d   ik %6d \n",
					(int)i,(int)ip[i],(int)jp[i],(int)is[i],(int)jz[i],(int)ik[i]);

			fprintf(debf,"decomposition code \n");
			for (i = 0; i < code->ndec; i +=3)
			{
				fprintf(debf," %8d %8d %8d \n",
						(int)code->dec[i], (int)code->dec[i+1], (int)code->dec[i+2]);
			}

			fprintf(debf,"solution code \n");
			for (i = 0; i < code->nsol; i +=3)
			{	fprintf(debf," %8d %8d %8d \n",
						(int)code->sol[i], (int)code->sol[i+1], (int)code->sol[i+2]);
			}

			fprintf(debf,"vector for back permutation of solution \n");
			for (i = 0; i < n; i++)
			{
				fprintf(debf," %8d %8d %8d ", (int)code->sol[i], (int)code->sol[i+1], (int)code->sol[i+2]);
			}
			fprintf(debf,"\n end of lequsparse_codegen \n\n");

		}
	}

#endif

	free(ip);
	free(jp);
	free(jz);
	free(ik);
	free(is);
	ip = jp = jz = ik = is = NULL;
	free(sA->fa);
	sA->fa = NULL;
	return ret;
}

/**
 *  This function is for forward/backward substitution (solution).
 *  It destroyes the righthand side r
 *
 *  @param    sA       sparse matrix
 *  @param    code     sparse decomposition code
 *  @param    r        right hand vector
 *  @param    x        solution vector
 *  @return nothing
 *
 *  @author C. Clauss clauss@eas.iis.fhg.de
 *  @version $Id: MA_lequspar.c 969 2010-03-01 21:34:37Z karsten $
 *
 */

exportMA_Sparse err_code MA_LequSparseSolut(struct sparse* sA,struct spcode* code,value* r,value* x)
{
	count_far k, l;
	err_code ret;

	/* printf("\n in Solut:  rank  %d   nsol  %d   nsoldec  %d\n",
	 (int)code->rank, (int)code->nsol, (int)code->nsoldec);
	 */
	ret = 0;

	if (code->rank == sA->nd)
	{ /* solution in regular case, dekomposition and backward subst. together */
		for (l = 0; l < code->nsol; l += 3)
		{
			k = code->sol[l + 2];
			if (k < 0)
			{
				*(r + code->sol[l]) *= sA->a[code->sol[l + 1]];

				/* printf(" rl = %e  a=%e \n",
				 (double)(*(r + code->sol[l])),
				 (double)(sA->a[ code->sol[l+1] ])); */
			}
			else
			{
				*(r + code->sol[l]) -= *(r + code->sol[l + 1]) * sA->a[k];
				/* printf(" rl = %e  rl+1=%e  a=%e \n", *(r + code->sol[l]),
				 *(r + code->sol[l+1]), sA->a[ k ]); */
			}
		}
	}
	else
	{ /* singular case */
		for (l = 0; l < code->nsoldec; l += 3) /* decomposition */
		{
			k = code->sol[l + 2];
			if (k < 0)
			{
				*(r + code->sol[l]) *= sA->a[code->sol[l + 1]];
			}
			else
			{
				*(r + code->sol[l]) -= *(r + code->sol[l + 1]) * sA->a[k];
			}
		}

		/*
		 printf("\n singular, r after decomposition \n");
		 for (l = 0; l< sA->nd; l++)
		 {
		 printf("  %e ", (double)r[l]);
		 }
		 printf("\n");
		 */
		for (l = 0; l < sA->nd - code->rank; l++)
		{
			if (r[code->zerodemand[l]] > code->piv_abs_tol
					|| r[code->zerodemand[l]] < -code->piv_abs_tol)
				ret = 1;
		}

		for (l = code->nsoldec; l < code->nsol; l += 3) /* backward substitution */
		{
			k = code->sol[l + 2];
			if (k < 0)
			{
				*(r + code->sol[l]) *= sA->a[code->sol[l + 1]];
			}
			else
			{
				*(r + code->sol[l]) -= *(r + code->sol[l + 1]) * sA->a[k];
			}
		}
	}

	/* back permutation */
	for (k = 0; k < sA->nd; k++)
	{
		x[code->isort[k]] = r[k];
	}

	return ret;
}

/* /// end of file ////////////////////////////////////////////////////////// */
