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

  sca_solve_ac_linear.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 02.01.2010

   SVN Version       :  $Revision: 1107 $
   SVN last checkin  :  $Date: 2011-01-09 12:20:46 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_solve_ac_linear.h 1107 2011-01-09 11:20:46Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_SOLVE_AC_LINEAR_H_
#define SCA_SOLVE_AC_LINEAR_H_

namespace sca_ac_analysis
{
namespace sca_implementation
{
struct sca_solve_ac_linear_data;
int sca_solve_ac_linear_init( double* A,unsigned long n,sca_solve_ac_linear_data** data);
int sca_solve_ac_linear(double* B, double* x,struct sca_solve_ac_linear_data** data);
void sca_solve_ac_linear_free(struct sca_solve_ac_linear_data** data);
void sca_solve_ac_get_error_position(sca_solve_ac_linear_data* data,long* row,long* column);

} // namespace sca_implementation
} // namespace sca_ac_analysis

#endif /* SCA_SOLVE_AC_LINEAR_H_ */
