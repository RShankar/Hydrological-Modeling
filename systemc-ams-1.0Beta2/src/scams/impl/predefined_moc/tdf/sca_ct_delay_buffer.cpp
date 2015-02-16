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

  sca_ct_delay_buffer.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: Feb 20, 2010

   SVN Version       :  $Revision$
   SVN last checkin  :  $Date$
   SVN checkin by    :  $Author$
   SVN Id            :  $Id$

 *****************************************************************************/

/*****************************************************************************/

#include"sca_ct_delay_buffer.h"

namespace sca_tdf
{
namespace sca_implementation
{


sca_ct_delay_buffer_base::sca_ct_delay_buffer_base() : time_buffer(NULL)
{
	buffer_size=0;
	buffer_used=0;
	buffer_write=0;
	buffer_first=0;
	first_index=0;
	second_index=1;
}

sca_ct_delay_buffer_base::~sca_ct_delay_buffer_base()
{
	if(time_buffer!=NULL) delete[] time_buffer;
	time_buffer=NULL;
}

void sca_ct_delay_buffer_base::set_max_delay(sca_core::sca_time mtime)
{
	max_time=mtime;
}

///////////////////

void sca_ct_delay_buffer_base::set_time_reached(sca_core::sca_time rtime)
{
	time_reached=rtime;
}

///////////////////

void  sca_ct_delay_buffer_base::resize(unsigned long size)
{
	sca_core::sca_time* tmp_buf=new sca_core::sca_time[size];

	for(unsigned long i=0;i<buffer_used;i++)
	{
		tmp_buf[i]=time_buffer[(buffer_first+i)%buffer_size];
	}

	resize_value_buffer(size);

	buffer_first=0;
	buffer_write=buffer_used;
	buffer_size=size;

	if(time_buffer!=NULL) delete[] time_buffer;

	time_buffer=tmp_buf;
}

/////////////////////////

void sca_ct_delay_buffer_base::store_value(sca_core::sca_time vtime)
{
	if(buffer_size<=buffer_used)
	{
		//we remove samples older than max_time and older
		//than last_read_time - we assume, that the buffer is read continuously
		if(time_reached>max_time)
		{
			sca_core::sca_time delayed_time;
			if(last_read_time>time_reached-max_time)
			{
				delayed_time=time_reached-max_time;
			}
			else
			{
				delayed_time=last_read_time;
			}

			//the second sample must be less than delayed_time
			while(time_buffer[(buffer_first+1)%buffer_size]<delayed_time)
			{
				if(buffer_used<=2) break; //at least one sample must be left
				buffer_first++;           //remove obsolete sample
				buffer_used--;
			}
		}

		//if required resize buffer
		if(buffer_size<=buffer_used) resize(buffer_size+10);
	}

	//store value
	time_buffer[buffer_write%buffer_size]=vtime;
	buffer_write++;
	buffer_used++;
}

////////////////////////////////

int sca_ct_delay_buffer_base::get_time_interval(sca_core::sca_time vtime)
{
	//usually the next requested value is after the previous one
	// - if not reset index values
	if( (first_index<(buffer_write-buffer_used))  ||  //outside of avail. buffer
	    (time_buffer[first_index%buffer_size]>=vtime))
	{
		first_index=buffer_write-buffer_used; //index to first avail. value
		second_index=first_index+1;

		if(time_buffer[first_index%buffer_size]>vtime) return 0;
	}

	//search time interval
	for(unsigned long i=second_index;i<buffer_write;i++)
	{
		if(time_buffer[i%buffer_size]>=vtime)
		{
			second_index=i;
			first_index=i-1;
			return 1;
		}
	}

	if(time_buffer[first_index%buffer_size]==vtime)
	{
		second_index=first_index;
		return 2;  //no time intervall, however first value
	}

	return 0; //error - time interval not found
}

bool  sca_ct_delay_buffer_base::get_next_time_after(sca_core::sca_time ntime)
{
	last_read_time=ntime;

	//usually the next requested value is after the previous one
	// - if not reset index values
	if( (first_index<(buffer_write-buffer_used))  ||  //outside of avail. buffer
	    (time_buffer[first_index%buffer_size]>=ntime))
	{
		first_index=buffer_write-buffer_used; //index to first avail. value
		second_index=first_index+1;
	}

	//search time point
	for(unsigned long i=first_index;i<buffer_write;i++)
	{
		if(time_buffer[i%buffer_size]>ntime)
		{
			first_index=i;
			second_index=i+1;
			return false;
		}
	}
	return true; //error - time interval not found
}


bool sca_ct_delay_buffer_base::get_timepoint(sca_core::sca_time ntime)
{
	last_read_time=ntime;

	//usually the next requested value is after the previous one
	// - if not reset index values
	if( (first_index<(buffer_write-buffer_used))  ||  //outside of avail. buffer
	    (time_buffer[first_index%buffer_size]>=ntime))
	{
		first_index=buffer_write-buffer_used; //index to first avail. value
		second_index=first_index+1;
	}

	//search time point
	for(unsigned long i=first_index;i<buffer_write;i++)
	{
		if(time_buffer[i%buffer_size]==ntime)
		{
			first_index=i;
			second_index=i+1;
			return false;
		}
	}
	return true; //error - time interval not found
}

//////////////////////////////////////////
/////////////////////////////////////////

///////////////////

sca_ct_delay_buffer_double::sca_ct_delay_buffer_double() : value_buffer(NULL)
{
	resize(2);
}

sca_ct_delay_buffer_double::~sca_ct_delay_buffer_double()
{
	if(value_buffer!=NULL) delete[] value_buffer;
	value_buffer=NULL;
}

void  sca_ct_delay_buffer_double::resize_value_buffer(unsigned long size)
{
	double* tmp_buf=new double[size];

	for(unsigned long i=0;i<buffer_used;i++)
	{
		tmp_buf[i]=value_buffer[(buffer_first+i)%buffer_size];
	}

	for(unsigned long i=buffer_used;i<size;i++)
	{
		tmp_buf[i]=0.0;
	}

	if(value_buffer!=NULL) delete[] value_buffer;
	value_buffer=tmp_buf;
}


void sca_ct_delay_buffer_double::store_value(sca_core::sca_time vtime,double value)
{
	sca_ct_delay_buffer_base::store_value(vtime);
	value_buffer[(buffer_write-1)%buffer_size]=value;
}

bool sca_ct_delay_buffer_double::get_value(sca_core::sca_time vtime, double& value)
{
	int res=get_time_interval(vtime);

	if(res==0) return true;

	if(res==2)
	{
		value = value_buffer[first_index%buffer_size];
		return false;
	}

	unsigned long first=first_index%buffer_size;
	unsigned long second=second_index%buffer_size;

	value=(value_buffer[second]-value_buffer[first]) /
	       (time_buffer[second]-time_buffer[first]).to_seconds() *
	       (vtime-time_buffer[first]).to_seconds() + value_buffer[first];

	return false;
}

bool sca_ct_delay_buffer_double::get_next_value_after
		(sca_core::sca_time& atime, double& value,sca_core::sca_time ntime)
{
	if(get_next_time_after(ntime))
	{
		return true;
	}

	atime=time_buffer[first_index%buffer_size];
	value=value_buffer[first_index%buffer_size];

	return false;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

sca_ct_delay_buffer_vector::sca_ct_delay_buffer_vector() : value_buffer(NULL)
{
	resize(2);
}

sca_ct_delay_buffer_vector::~sca_ct_delay_buffer_vector()
{
	if(value_buffer!=NULL) delete[] value_buffer;
	value_buffer=NULL;
}

void  sca_ct_delay_buffer_vector::resize_value_buffer(unsigned long size)
{
	sca_util::sca_vector<double>* tmp_buf=new sca_util::sca_vector<double>[size];

	for(unsigned long i=0;i<buffer_used;i++)
	{
		tmp_buf[i]=value_buffer[(buffer_first+i)%buffer_size];
	}


	if(value_buffer!=NULL) delete[] value_buffer;
	value_buffer=tmp_buf;
}


void sca_ct_delay_buffer_vector::store_value(sca_core::sca_time vtime,
		const sca_util::sca_vector<double>& values)
{
	sca_ct_delay_buffer_base::store_value(vtime);
	//buffer_write was increased in base class store_value
	value_buffer[(buffer_write-1)%buffer_size]=values;
}

bool sca_ct_delay_buffer_vector::get_value(sca_core::sca_time vtime,
		sca_util::sca_vector<double>& values)
{
	int res=get_time_interval(vtime);

	if(res==0) return true;

	if(res==2)
	{
		values = value_buffer[first_index%buffer_size];
		return false;
	}

	unsigned long first=first_index%buffer_size;
	unsigned long second=second_index%buffer_size;

	sca_util::sca_vector<double>& fv(value_buffer[first]);
	sca_util::sca_vector<double>& sv(value_buffer[second]);

	if(fv.length()!=sv.length())
	{
		SC_REPORT_ERROR("SystemC-AMS","Internal Error due a Bug");
	}

	values.resize(fv.length());
	for(unsigned long i=0;i<fv.length();i++)
	{
		values(i) = (sv(i)-fv(i)) /
		            (time_buffer[second]-time_buffer[first]).to_seconds() *
		            (vtime-time_buffer[first]).to_seconds() + fv(i);
	}

	return false;
}

bool sca_ct_delay_buffer_vector::get_next_value_after
		(sca_core::sca_time& atime, sca_util::sca_vector<double>& values,sca_core::sca_time ntime)
{
	if(get_next_time_after(ntime))
	{
		return true;
	}

	atime=time_buffer[first_index%buffer_size];
	values=value_buffer[first_index%buffer_size];

	return false;
}



} //namespace sca_implementation
} //namespace sca_tdf
