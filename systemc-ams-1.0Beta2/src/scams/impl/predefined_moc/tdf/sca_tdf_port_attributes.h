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

  sca_tdf_port_attributes.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 27.08.2009

   SVN Version       :  $Revision: 969 $
   SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_port_attributes.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_TDF_PORT_ATTRIBUTES_H_
#define SCA_TDF_PORT_ATTRIBUTES_H_


namespace sca_tdf
{
namespace sca_implementation
{

class sca_port_attributes
{
public:

	const unsigned long& get_rate() const;
	const unsigned long& get_delay() const;
	const sca_core::sca_time& get_timestep() const;
	const sca_core::sca_time& get_timeoffset() const;

protected:

	void set_rate(unsigned long rate_);
	void set_delay(unsigned long delay_);
	void set_timestep(const sca_core::sca_time& timestep_);
	void set_timeoffset(const sca_core::sca_time& timeoffset_);

	sca_port_attributes();


private:

	unsigned long rate;
	unsigned long delay;
	sca_core::sca_time timestep;
	sca_core::sca_time timeoffset;

	sca_core::sca_time timestep_calculated;
	sca_core::sca_time timeoffset_calculated;

	friend class sca_tdf::sca_implementation::sca_tdf_view;
	friend class sca_eln::sca_implementation::sca_eln_view;
	friend class sca_lsf::sca_implementation::sca_lsf_view;


};


}
}


#endif /* SCA_TDF_PORT_ATTRIBUTES_H_ */
