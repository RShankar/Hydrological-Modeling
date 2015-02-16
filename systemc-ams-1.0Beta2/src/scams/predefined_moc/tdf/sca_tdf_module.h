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

 sca_tdf_module.h - base class for tdf modules

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 1187 $
 SVN last checkin  :  $Date: 2011-05-05 18:45:03 +0200 (Thu, 05 May 2011) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_module.h 1187 2011-05-05 16:45:03Z karsten $

 *****************************************************************************/
/*

 LRM clause 4.1.1.1.
 The class sca_tdf::sca_module shall define the base class for all
 TDF primitive modules.

 */

/*****************************************************************************/

#ifndef SCA_TDF_MODULE_H_
#define SCA_TDF_MODULE_H_

namespace sca_tdf
{

//begin implementation specific
namespace sca_implementation
{
class sca_tdf_view;
class sca_tdf_solver;
class sca_ct_ltf_nd_proxy;
class sca_ct_vector_ss_proxy;
}

//end implementation specific

//class sca_module : public implementation-derived-from sca_core::sca_module
class sca_module: public sca_core::sca_module
{
public:
	virtual const char* kind() const;

protected:
	typedef void (sca_tdf::sca_module::*sca_module_method)();

	virtual void set_attributes();
	virtual void initialize();
	virtual void processing();
	virtual void ac_processing();

	void register_processing(sca_tdf::sca_module::sca_module_method);
	void register_ac_processing(sca_tdf::sca_module::sca_module_method);

	sca_core::sca_time get_time() const;

	explicit sca_module(sc_core::sc_module_name);
	sca_module();

	virtual ~sca_module();


// begin implementation specific section

private:

	friend class sca_tdf::sca_implementation::sca_tdf_solver;
	friend class sca_tdf::sca_implementation::sca_tdf_view;
	friend class sca_tdf::sca_implementation::sca_ct_ltf_nd_proxy;
	friend class sca_tdf::sca_implementation::sca_ct_vector_ss_proxy;

	  /**
	    Method for registering the attribute method. The registered method will
	    will be called from the view layer before datastructure set-up
	  */
	  void register_attr_method(sca_module_method funcp);

	  /**
	    Method for registering the init method. The registered method will
	    will be called from the solver layer immedately before simulation start,
	    after all simulation datastructures set-up.
	  */
	  void register_init_method(sca_module_method funcp);


	  /**
	    Method for registering the post processing method. The registered method
	    will be called from the solver layer before simulation finished (before leaving
	    sc_main).
	  */
	  void register_post_method(sca_module_method funcp);


	  /** Default method for post processing see @ref #void register_post_method */
	  virtual void post_proc() {}


	  //reference to counter with number of module calls
	  const sc_dt::int64** call_counter;

	  //the view class is allowed to access the assigned methods
	  friend class sca_tdf_view;

	  sca_module_method attr_method;
	  sca_module_method init_method;
	  sca_module_method sig_proc_method;
	  sca_module_method ac_processing_method;
	  sca_module_method post_method;

	  bool reg_attr_method;
	  bool reg_init_method;
	  bool reg_processing_method;
	  bool reg_ac_processing_method;
	  bool reg_post_method;

	  bool no_default_processing;
	  bool no_default_ac_processing;
	  bool check_for_processing;

	  void construct();

	  //overload sca_core::sca_module method
	  void elaborate();

// end implementation specific section


};

#define SCA_TDF_MODULE(name) struct name : sca_tdf::sca_module

} // namespace sca_tdf

#endif /* SCA_TDF_MODULE_H_ */
