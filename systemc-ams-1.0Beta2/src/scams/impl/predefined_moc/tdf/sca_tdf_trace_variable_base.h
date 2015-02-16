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

  sca_tdf_trace_variable_base.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 12.12.2009

   SVN Version       :  $Revision: 1187 $
   SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_tdf_trace_variable_base.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_TDF_TRACE_VARIABLE_BASE_H_
#define SCA_TDF_TRACE_VARIABLE_BASE_H_

namespace sca_tdf
{
namespace sca_implementation
{


class sca_trace_variable_base :  public sc_core::sc_object,
                                 public sca_util::sca_traceable_object
{
public:
    sca_trace_variable_base(const char* nm);
    virtual ~sca_trace_variable_base();

protected:

    unsigned long rate;
    sca_core::sca_time       timeoffset;
    unsigned long calls_per_period;
    sca_core::sca_time       sample_period;
    const sc_dt::int64*    call_counter_ref;
    long          buffer_size;
    std::vector<bool>  write_flags;
    bool          initialized;

    bool initialize();

private:

    sca_tdf::sca_module*   parent_module;

    bool trace_init(sca_util::sca_implementation::sca_trace_object_data& data);
    void trace(long id,sca_util::sca_implementation::sca_trace_buffer& buffer);

    virtual void resize_buffer(long n)           = 0;
    virtual void store_to_last(long n)           = 0;
    virtual void trace_last(long id,sca_core::sca_time& ct,sca_util::sca_implementation::sca_trace_buffer& tr_buffer) = 0;

    virtual void set_type_info(sca_util::sca_implementation::sca_trace_object_data& data)=0;
};


} //namespace sca_implementation

} //namespace sca_util

#endif /* SCA_TDF_TRACE_VARIABLE_BASE_H_ */
