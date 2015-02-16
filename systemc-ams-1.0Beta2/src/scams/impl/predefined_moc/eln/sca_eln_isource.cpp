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

  sca_eln_isource.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 12.11.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_eln_isource.cpp 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/eln/sca_eln_isource.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_eln
{

sca_isource::sca_isource(sc_core::sc_module_name, double init_value_ ,
		double offset_ , double amplitude_ , double frequency_ , double phase_, sca_core::sca_time delay_ , double ac_amplitude_ ,
		double ac_phase_ , double ac_noise_amplitude_ ) :
	p("p"), n("n"), init_value("init_value", init_value_), offset("offset",
			offset_), amplitude("amplitude", amplitude_), frequency(
			"frequency", frequency_), phase("phase",phase_), delay("delay", delay_), ac_amplitude(
			"ac_amplitude", ac_amplitude_),
			ac_phase("ac_phase", ac_phase_), ac_noise_amplitude(
					"ac_noise_amplitude", ac_noise_amplitude_)
{
    through_value_available = true;
    through_value_type      = "I";
    through_value_unit      = "A";

    i_value=0.0;
}

const char* sca_isource::kind() const
{
	return "sca_eln::sca_tdf::sca_isource";
}



double sca_isource::i_t()
{
    if (get_time() < delay)
    {
    	i_value=init_value;
    	return i_value;
    }

    i_value = offset + amplitude * sin(2.0 * M_PI * frequency * (sca_get_time()
    			- delay.get().to_seconds()) + phase);
    return i_value;
}


void sca_isource::matrix_stamps()
{
    nadd = add_equation();

    B(p, nadd) =  1.0;
    B(n, nadd) = -1.0;

    if( sca_ac_analysis::sca_ac_is_running() )
    {
    	if(sca_ac_analysis::sca_ac_noise_is_running())
    	{
            B(nadd, nadd) =  1.0;
            A(nadd, nadd) =  0.0;

            q(nadd).set_value(-ac_noise_amplitude.get());
    	}
    	else
    	{
    		sca_util::sca_complex phase_cmpl(0, (M_PI/180) * ac_phase.get());

    		phase_cmpl = exp(-phase_cmpl);

    		B(nadd, nadd) =  phase_cmpl.real();
    		A(nadd, nadd) =  phase_cmpl.imag()/sca_ac_analysis::sca_ac_w();

    		q(nadd).set_value(-ac_amplitude.get());
    	}
    }
    else
    {
        B(nadd, nadd) =  1.0;
        A(nadd, nadd) =  0.0;

        q(nadd).sub_element(SCA_MPTR(sca_isource::i_t), this);
    }

}



bool sca_isource::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type=through_value_type;
    data.unit=through_value_unit;

    //trace will be activated after every complete cluster calculation
    //by the synchronization layer
    get_sync_domain()->add_solver_trace(data);
    return true;
}

void sca_isource::trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer)
{
    sca_core::sca_time ctime = sca_eln::sca_module::get_time();

    double through_value = i_value;
    buffer.store_time_stamp(id,ctime,through_value);
}

sca_util::sca_complex sca_isource::calculate_ac_result(sca_util::sca_complex* res_vec)
{
	return std::polar(ac_amplitude.get(),(M_PI/180.0) * ac_phase.get());
}



} //namespace sca_eln
