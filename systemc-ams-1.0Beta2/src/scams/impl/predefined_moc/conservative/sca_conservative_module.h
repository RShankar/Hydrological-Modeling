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

  sca_conservative_module.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 04.11.2009

   SVN Version       :  $Revision: 1157 $
   SVN last checkin  :  $Date: 2011-02-09 23:08:11 +0100 (Wed, 09 Feb 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_conservative_module.h 1157 2011-02-09 22:08:11Z karsten $

 *****************************************************************************/

/*****************************************************************************/


#ifndef SCA_CONSERVATIVE_MODULE_H_
#define SCA_CONSERVATIVE_MODULE_H_

namespace sca_core
{
namespace sca_implementation
{

class sca_conservative_view;

/**
   Module base class for primitive modules
   of conservative networks.
*/
class sca_conservative_module : public sca_core::sca_module
{

	friend class sca_conservative_view;

  protected:

    sca_conservative_module();
    explicit sca_conservative_module(sc_core::sc_module_name);
    ~sca_conservative_module();

    virtual const char* kind() const;

    long cluster_id;

	/**
	 * reinitialization of equation system is forced
	 */
	void request_reinit(int mode=1);
	void request_reinit(int mode,double new_val);
	void request_reinit(double new_val);


	sc_dt::uint64* call_id_p;
	long* reint_request_i;

  public:

    std::vector<long> add_equations;

	sc_dt::uint64 call_id_request_init;
	double        new_value;
	bool          value_set;

	unsigned long reinit_request_cnt;
};



} // namespace sca_core
} // namespace sca_implementation

#endif /* SCA_CONSERVATIVE_MODULE_H_ */
