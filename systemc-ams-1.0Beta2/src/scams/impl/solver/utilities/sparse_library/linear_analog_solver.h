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

 linear_analog_solver.h - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 22.10.2009

 SVN Version       :  $Revision: 1103 $
 SVN last checkin  :  $Date: 2011-01-08 21:11:02 +0100 (Sat, 08 Jan 2011) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: linear_analog_solver.h 1103 2011-01-08 20:11:02Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef  LINEAR_ANALOG_SOLVER_H
#define  LINEAR_ANALOG_SOLVER_H

extern "C"
{
  class sca_solv_data;

  int ana_init (
            double* A,             /* sqaure matrix A */
            double* B,             /* sqaure matrix B */
            unsigned long size,    /* number of equations -> dim of A and B */
            double  h,             /* constant time step */
            sca_solv_data **sdatap,/* internal solver data */
            int reinit             /* reinit -> hold states and dont allocates*/
                                   /* memory */
               );

  int ana_reinit (
            double* A,             /* sqaure matrix A */
            double* B,             /* sqaure matrix B */
            unsigned long size,    /* number of equations -> dim of A and B */
            double  h,             /* constant time step */
            sca_solv_data **sdatap,/* internal solver data */
            int reinit             /* reinit -> hold states and dont allocates*/
                                   /* memory */
                 );

  void ana_solv (
                    double* q,       /* time dependent vector */
                    double* x,       /* state vector */
                    sca_solv_data* sdata /* internal solver data */
                );

  void ana_get_error_position(sca_solv_data* data,long* row,long* column);
}

#endif
