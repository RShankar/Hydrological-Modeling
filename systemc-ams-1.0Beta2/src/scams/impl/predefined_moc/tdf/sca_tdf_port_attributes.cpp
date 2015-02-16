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

 sca_tdf_port_attributes.cpp - description

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 27.08.2009

 SVN Version       :  $Revision: 1039 $
 SVN last checkin  :  $Date: 2010-08-17 11:11:52 +0200 (Tue, 17 Aug 2010) $
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_port_attributes.cpp 1039 2010-08-17 09:11:52Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include <systemc-ams>
#include "scams/impl/predefined_moc/tdf/sca_tdf_port_attributes.h"


namespace sca_tdf
{

namespace sca_implementation
{

const unsigned long& sca_port_attributes::get_rate() const
{
	return rate;
}


const unsigned long& sca_port_attributes::get_delay() const
{
	return delay;
}

const sca_core::sca_time& sca_port_attributes::get_timestep() const
{
	return timestep_calculated;
}

const sca_core::sca_time& sca_port_attributes::get_timeoffset() const
{
	return timeoffset_calculated;
}



void sca_port_attributes::set_rate(unsigned long rate_)
{
	rate=rate_;
}


void sca_port_attributes::set_delay(unsigned long delay_)
{
	delay=delay_;
}

void sca_port_attributes::set_timestep(const sca_core::sca_time& timestep_)
{
	timestep=timestep_;
	timestep_calculated=timestep_;
}


void sca_port_attributes::set_timeoffset(const sca_core::sca_time& timeoffset_)
{
	timeoffset=timeoffset_;
	timeoffset_calculated=timeoffset_;
}

sca_port_attributes::sca_port_attributes()
{
	rate=1;
	delay=0;
	timeoffset=sc_core::SC_ZERO_TIME;
	timestep=sca_core::sca_implementation::NOT_VALID_SCA_TIME();

	timeoffset_calculated=sc_core::SC_ZERO_TIME;
	timestep_calculated=sca_core::sca_implementation::NOT_VALID_SCA_TIME();
}
}
}

