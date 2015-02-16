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

  sca_ct_delay_buffer.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: Feb 20, 2010

   SVN Version       :  $Revision$
   SVN last checkin  :  $Date$
   SVN checkin by    :  $Author$
   SVN Id            :  $Id$

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_CT_DELAY_BUFFER_H_
#define SCA_CT_DELAY_BUFFER_H_

#include "systemc-ams"

namespace sca_tdf
{
namespace sca_implementation
{

class sca_ct_delay_buffer_base
{
public:

	void set_max_delay(sca_core::sca_time mtime);
	void set_time_reached(sca_core::sca_time rtime);


protected:

	void store_value(sca_core::sca_time vtime);
	int get_time_interval(sca_core::sca_time vtime);
	//sets first_index to time point after ntime (if not available return true)
	bool get_next_time_after(sca_core::sca_time ntime);
	bool get_timepoint(sca_core::sca_time ntime);

	sca_ct_delay_buffer_base();
	virtual ~sca_ct_delay_buffer_base();

	virtual void resize_value_buffer(unsigned long size)=0;


	void resize(unsigned long size);

	sca_core::sca_time max_time;

	sca_core::sca_time* time_buffer;
	unsigned long buffer_size;
	unsigned long buffer_used;
	unsigned long buffer_write;
	unsigned long buffer_first;

	unsigned long first_index;
	unsigned long second_index;

	sca_core::sca_time last_read_time;

	sca_core::sca_time time_reached;

};

class sca_ct_delay_buffer_double : public sca_ct_delay_buffer_base
{
public:
	void store_value(sca_core::sca_time vtime,double value);
	bool get_value(sca_core::sca_time vtime, double& value);

	bool get_next_value_after(sca_core::sca_time& atime, double& value,sca_core::sca_time ntime);

	sca_ct_delay_buffer_double();
	~sca_ct_delay_buffer_double();

private:

	double* value_buffer;

	void resize_value_buffer(unsigned long size);

};


class sca_ct_delay_buffer_vector : public sca_ct_delay_buffer_base
{
public:
	void store_value(sca_core::sca_time vtime,const sca_util::sca_vector<double>& values);
	bool get_value(sca_core::sca_time vtime, sca_util::sca_vector<double>& values);

	bool get_next_value_after(sca_core::sca_time& atime, sca_util::sca_vector<double>& value,sca_core::sca_time ntime);

	sca_ct_delay_buffer_vector();
	~sca_ct_delay_buffer_vector();

private:

	sca_util::sca_vector<double>* value_buffer;

	void resize_value_buffer(unsigned long size);

};


}
}


#endif /* SCA_CT_DELAY_BUFFER_H_ */
