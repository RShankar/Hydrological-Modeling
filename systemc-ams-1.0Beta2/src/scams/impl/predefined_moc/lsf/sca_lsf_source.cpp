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

  sca_lsf_source.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.01.2010

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_source.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/lsf/sca_lsf_source.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_lsf
{

sca_source::sca_source(
		sc_core::sc_module_name,
		double init_value_,
		double offset_,
		double amplitude_,
		double frequency_,
		double phase_,
		sca_core::sca_time delay_,
		double ac_amplitude_,
		double ac_phase_,
		double ac_noise_amplitude_) :
			y("y"), init_value("init_value", init_value_),
			offset("offset", offset_), amplitude("amplitude", amplitude_),
			frequency("frequency", frequency_), phase("phase", phase_),
			delay("delay", delay_), ac_amplitude("ac_amplitude",
					ac_amplitude_), ac_phase("ac_phase", ac_phase_),
			ac_noise_amplitude("ac_noise_amplitude", ac_noise_amplitude_)
{
}

const char* sca_source::kind() const
{
	return "sca_lsf::sca_source";
}

void sca_source::matrix_stamps()
{
    B(y,y)  =  -1.0;

    if(!sca_ac_analysis::sca_ac_is_running())
    {
    	A(y,y)= 0.0;
    	q(y).add_element(SCA_MPTR(sca_source::value_t),this);
    }
    else
    {
    	if(sca_ac_analysis::sca_ac_noise_is_running())
    	{
    		q(y).set_value(ac_noise_amplitude.get());
    	}
    	else
    	{
    		sca_util::sca_complex phase_cmpl(0.0, (M_PI/180.0) * ac_phase.get());

    		phase_cmpl = exp(-phase_cmpl);

    		B(y, y) =  phase_cmpl.real();
    		A(y, y) =  phase_cmpl.imag()/sca_ac_analysis::sca_ac_w();

    		q(y).set_value(-ac_amplitude.get());
    	}
    }
}

double sca_source::value_t()
{
	if (get_time() < delay)
    		return init_value;

	double tmp;
    	tmp = offset + amplitude * sin(2.0 * M_PI * frequency * (sca_get_time()
    			- delay.get().to_seconds()) + phase);
    return tmp;
}


} //namespace sca_lsf
