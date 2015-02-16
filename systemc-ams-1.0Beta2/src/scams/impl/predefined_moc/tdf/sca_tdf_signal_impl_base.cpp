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

  sca_tdf_signal_impl_base.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 06.08.2009

   SVN Version       :  $Revision: 1187 $
   SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_signal_impl_base.cpp 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#include<systemc-ams>
#include "scams/impl/predefined_moc/tdf/sca_tdf_signal_impl_base.h"
#include "scams/impl/synchronization/sca_synchronization_obj_if.h"
#include "scams/impl/core/sca_solver_base.h"
#include "scams/impl/util/tracing/sca_trace_file_base.h"
#include "scams/impl/analysis/ac/sca_ac_domain_globals.h"




namespace sca_tdf
{
namespace sca_implementation
{


/** Sets sample rate of the port */
void sca_tdf_signal_impl_base::set_rate(unsigned long port,unsigned long rate)
{
    if(rates.size()<=port)
        rates.resize(port+1);
    rates[port]=rate;
}

/** Sets number of sample delay of the port */
void sca_tdf_signal_impl_base::set_delay(unsigned long port,unsigned long samples)
{
    if(delays.size()<=port)
        delays.resize(port+1);
    delays[port]=samples;
}

/** Sets time distance between two samples */
void sca_tdf_signal_impl_base::set_T(unsigned long port,sca_core::sca_time t_period)
{

    if(Ts_setted.size()<=port)
    {
        Ts_setted.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
        Ts_calculated.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
    }
    Ts_setted[port]       = t_period;
    Ts_calculated[port] = t_period;
}


/** sets absolute time of first sample (must be < than T */
void sca_tdf_signal_impl_base::set_t0(unsigned long port,sca_core::sca_time t0)
{
    if(t0s_setted.size()<=port)
    {
        t0s_setted.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
        t0s_calculated.resize(port+1,::sc_core::SC_ZERO_TIME);
    }
    t0s_setted[port]     = t0;
    if(t0!=sca_core::sca_implementation::NOT_VALID_SCA_TIME())
        t0s_calculated[port] = t0;
}

/** Gets the absolute number of samples (from simulation start) samples
    of the current call are not included */
unsigned long sca_tdf_signal_impl_base::get_sample_cnt(unsigned long port)
{
    return *(call_counter_refs[port]) * rates[port];
}

/** Gets the setted sampling rate of the port */
unsigned long& sca_tdf_signal_impl_base::get_rate(unsigned long port)
{
    return rates[port];
}
/** Gets the setted sampling delay of the port */
unsigned long& sca_tdf_signal_impl_base::get_delay(unsigned long port)
{
    return delays[port];
}

/** Gets the calculated sample time distance -> if the time was setted it
    must be equal to the calculated one - if not the scheduler will through
    an exception */
sca_core::sca_time& sca_tdf_signal_impl_base::get_T(unsigned long port)
{
    if(Ts_setted.size()<=port)
    {
        Ts_setted.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
        Ts_calculated.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
    }

    //to support dynamic time steps T is calculated dynamically
    if(cluster_period!=NULL)
        Ts_calculated[port]= (*cluster_period) /
                             (get_rate(port) * calls_per_period[port]);


    return Ts_calculated[port];
}

/** Gets the calculated time offset (time of the first sample) -> the setted one
    will be equal to the calculated one -> if not possible (causality) the
    scheduler will through an exception */
sca_core::sca_time& sca_tdf_signal_impl_base::get_t0(unsigned long port)
{
    if(t0s_setted.size()<=port)
    {
        t0s_setted.resize(port+1,sca_core::sca_implementation::NOT_VALID_SCA_TIME());
        t0s_calculated.resize(port+1,::sc_core::SC_ZERO_TIME);
    }
    return t0s_calculated[port];
}

/** Gets the absolute time (from simulation start) of the first sample of the
    current call */
//	sc_time get_time(unsigned long port, unsigned long sample=0)
//       {
//           long call_cnt_call =(*(call_counter_refs[port]))%calls_per_period[port];
//           sc_time ct=get_synchronization_if()->get_cluster_start_time() +
//                      ((sample+call_cnt_call*get_rate(port))*get_T(port));
//           return ct;
//       }


/** Gets the absolute time (from simulation start) of the first sample of the
  current call */
sca_core::sca_time sca_tdf_signal_impl_base::get_time(unsigned long port, unsigned long sample)
{
	if((calls_per_period==NULL)||(call_counter_refs==NULL))
	{
		return sca_core::sca_implementation::NOT_VALID_SCA_TIME();
	}

    sca_core::sca_time cp=get_synchronization_if()->get_cluster_period();
    //calculate number of samples per cluster period (number of module calls * port rate)
    long sample_period = calls_per_period[port]*get_rate(port);
    //calculate sample number since cluster start (module call since start * port rate)
    long sample_call   =(*(call_counter_refs[port]))%calls_per_period[port] *
                        get_rate(port);
    sca_core::sca_time ct=get_synchronization_if()->get_cluster_start_time() +
               (cp* (sample_call+sample))/sample_period;
    return ct;
}


//prepare channel datastructures
void sca_tdf_signal_impl_base::end_of_elaboration()
{
    unsigned long max_in_delay = 0;

    if(get_samples_per_period()<0)
        return; //not used


    long out_delay = 0;


    unsigned long    nports = get_number_of_connected_ports();
    sca_core::sca_implementation::sca_port_base**  plist  = get_connected_port_list();

    in_delay_buffer_resize(nports);
    //in_delay_buffer.resize(nports,NULL);

    is_inport = new bool[nports];


    //ignore not connected signals
    if(nports>0)
    {
        sca_ac_analysis::sca_implementation::sca_ac_register_arc
								(dynamic_cast<sc_core::sc_interface*>(this));
    }


    buffer_offsets        = new long[nports];
    call_counter_refs     = new const sc_dt::int64* [nports];
    calls_per_period      = new       long [nports];

    sca_core::sca_implementation::sca_port_base* cp;
    for(unsigned long i=0; i<nports; ++i)
    {
        cp = plist[i];
        cp->set_if_id(i); //set port identification

        //sdf ports can be used in all domains due they are sync ports
        call_counter_refs[i]=cp->get_parent_module()->
                             get_sync_domain()->get_call_counter_ref();

        calls_per_period[i]=cp->get_parent_module()->
                            get_sync_domain()->get_calls_per_period();

        buffer_offsets[i] = 0;

        if(cp->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_IN_PORT)
        {
            is_inport[i]=true;
            buffer_offsets[i] = -delays[i];

            if(delays[i]>0)
            {
            	in_delay_buffer_create(i,delays[i]);
                //in_delay_buffer[i]=new T[delays[i]];

                if(max_in_delay<delays[i])
                    max_in_delay = delays[i];
            }

        }
        else if(cp->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_OUT_PORT)
        {
            is_inport[i]=false;
            out_delay = delays[i];
            buffer_offsets[i] = delays[i];
            if(driver_port>=0)
            {
                std::ostringstream str;
                str << "Multiple driver for a sca_sdf_channel found." << std::endl
                << "Channel driven from module: "
                << plist[driver_port]->get_parent_module()->name()
                << " port #" << plist[driver_port]->get_port_number()
                << " and "
                << plist[i]->get_parent_module()->name()
                << " port #" << plist[i]->get_port_number() << "." << std::endl;
                SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
            }
            driver_port=i;
        }

        cluster_period=&(get_synchronization_if()->get_cluster_period());
    }

    buffer_size = get_samples_per_period()+out_delay+max_in_delay;

    create_buffer(buffer_size);
    //buffer = new T[buffer_size];
    //for(unsigned long i=0; i<buffer_size; ++i)	buffer[i] = T();
}

long sca_tdf_signal_impl_base::get_driver_port_id()
{
    return driver_port;
}

const char* sca_tdf_signal_impl_base::kind() const
{
    return "sca_sdf_signal";
}



void sca_tdf_signal_impl_base::construct()
{
    driver_port=-1;
    buffer_offsets=NULL;
    buffer_size=0;
    call_counter_refs=NULL;
    calls_per_period=NULL;
    cluster_period=NULL;
    is_inport=NULL;
}


sca_tdf_signal_impl_base::sca_tdf_signal_impl_base()
{
    construct();
}


sca_tdf_signal_impl_base::sca_tdf_signal_impl_base(const char* name_):
	sca_core::sca_prim_channel(name_)
{
    construct();
}

sca_tdf_signal_impl_base::~sca_tdf_signal_impl_base()
{
    buffer_size = 0;
    delete[] buffer_offsets;
    delete[] call_counter_refs;
    delete[] calls_per_period;
    delete[] is_inport;
}


bool sca_tdf_signal_impl_base::trace_init(sca_util::sca_implementation::sca_trace_object_data& data)
{
    data.type="-";
    data.unit="-";
    //trace will be activated after every complete cluster calculation
    //by the synchronization layer
    if(get_synchronization_if()==NULL)
        return false;

    set_type_info(data);

    get_synchronization_if()->add_cluster_trace(data);

    return true;
}

//is called after cluster was calculated
void sca_tdf_signal_impl_base::trace(long id,sca_util::sca_implementation::sca_trace_buffer& trace_buffer)
{
    sca_core::sca_time ctime;

    ctime=get_synchronization_if()->get_cluster_start_time() +
          get_t0(driver_port);

    sca_core::sca_time time_period=get_T(driver_port);

    unsigned long sample_per_period=calls_per_period[driver_port]*rates[driver_port];

    //call_counter_refs[driver_port] starts at 0 !!!
    sc_dt::int64 sample_cnt;
    sample_cnt=(*(call_counter_refs[driver_port]) + 1 -
                calls_per_period[driver_port] ) * rates[driver_port];

    for(unsigned long i=0; i<sample_per_period; ++i, ++sample_cnt)
    {
        store_trace_buffer_time_stamp(trace_buffer,id,ctime,sample_cnt%buffer_size);
        ctime+=time_period;
    }
}


} //namespace sca_implementation
} //namespace sca_tdf

