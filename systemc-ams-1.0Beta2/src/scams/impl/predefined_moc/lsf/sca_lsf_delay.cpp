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

  sca_lsf_delay.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 10.01.2010

   SVN Version       :  $Revision: 979 $
   SVN last checkin  :  $Date: 2010-03-07 10:58:17 +0100 (Sun, 07 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_lsf_delay.cpp 979 2010-03-07 09:58:17Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include "systemc-ams"
#include "scams/predefined_moc/lsf/sca_lsf_delay.h"
#include "scams/impl/util/data_types/sca_method_object.h"
#include "scams/impl/util/data_types/sca_method_list.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_lsf
{

sca_delay::sca_delay(sc_core::sc_module_name, sca_core::sca_time delay_, double k_, double y0_):
	x("x"), y("y"), delay("delay", delay_), k("k", k_), y0("y0", y0_)
{
	 delay_buffer = NULL;
	 delay_buffer_size=0;
	 delay_cnt=0;
	 nsample=0;
	 initialized=false;
	 scale_last_sample=0.0;
}

const char* sca_delay::kind() const
{
	return "sca_lsf::sca_delay";
}

void sca_delay::init_delay()
{
	nsample=(unsigned long)(delay.get().value()/get_timestep().value());

    delay_sec=delay.get().to_seconds();
    dt_sec=get_timestep().to_seconds();


	if(nsample==0)
	{
		//linear interpolation - enough ??
		// (x-xlast)/dt * (dt-delay) + xlast =
		// x/dt*(dt-delay)  + xlast*(1-(dt-delay)/dt) = xlast*delay/dt
		B(y,x) = (dt_sec-delay_sec)/dt_sec;
		scale_last_sample=delay_sec/dt_sec;
	}
	else
	{
		B(y,x) = 0.0;

		if(delay_buffer!=NULL)
		{
			if(delay_buffer_size==(nsample+1)) return;
			delete [] delay_buffer;
			delay_buffer=NULL;
			delay_buffer_size=0;
			delay_cnt=0;
		}

		delay_buffer=new double[nsample+1];
		if(delay_buffer==NULL)
		{
			std::ostringstream str;
			str << "Can't allocate memory for delay buffer in: "
			    << name();
			SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
		}

		for(unsigned long i=0;i<=nsample;i++)
		{
			delay_buffer[i]=y0.get();
		}

		delay_buffer_size=nsample+1;
		delay_cnt=nsample+1;

		dt_delay=(nsample+1)*dt_sec - delay_sec;
	}
}

void sca_delay::pre_solve()
{
	if(!initialized)
	{
		initialized=true;
		last_sample=y0.get();
		request_reinit();
	}
}


double sca_delay::q_t()
{
	if(nsample==0)
	{
	   return last_sample*scale_last_sample;
	}
	else
	{
	   double xc=delay_buffer[(delay_cnt-(nsample-1))%delay_buffer_size];
	   double xl=delay_buffer[(delay_cnt-nsample)%delay_buffer_size];

	   double out=(xc-xl)/dt_sec*dt_delay + xl;
	   return  out;
	}
}


void sca_delay::post_solve()
{
	if(nsample==0)
	{
		last_sample=sca_lsf::sca_module::x(x)*k.get();
	}
	else
	{
		delay_cnt++;
		delay_buffer[delay_cnt%delay_buffer_size]=sca_lsf::sca_module::x(x)*k.get();
	}
}

void sca_delay::matrix_stamps()
{

    B(y,y)  = -1.0;  // y = ...

    if(!sca_ac_analysis::sca_ac_is_running())
    {
    	//initialization must be done after setup - otherwise timstep not available
    	if(!initialized)
    	{
    		add_pre_solve_method(SCA_VMPTR(sca_delay::pre_solve));
    	}

    	init_delay();
    	q(y).add_element(SCA_MPTR(sca_delay::q_t),this);
    	add_post_solve_method(SCA_VMPTR(sca_delay::post_solve));

    }
    else
    {
    	double w=sca_ac_analysis::sca_ac_w();
    	delay_sec=delay.get().to_seconds();
    	//e**(-jw*delay) = cos(-w*delay) + j*sin(-w*delay)
    	A(y,x) =   k.get()/w * sin(-w*delay_sec);   // j*sin(w*delay)
    	B(y,x) =   k.get()   * cos(-w*delay_sec);   //   cos(w*delay)
    }

}

} //namespace sca_lsf


