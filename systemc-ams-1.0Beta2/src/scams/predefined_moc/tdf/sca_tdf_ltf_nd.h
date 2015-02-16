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

 sca_tdf_ltf_nd.h - linear transfer function in numerator/denumerator form

 Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

 Created on: 04.03.2009

 SVN Version       :  $Revision: 969 $
 SVN last checkin  :  $Date: 2010-03-01 22:34:37 +0100 (Mon, 01 Mar 2010) $ (UTC)
 SVN checkin by    :  $Author: karsten $
 SVN Id            :  $Id: sca_tdf_ltf_nd.h 969 2010-03-01 21:34:37Z karsten $

 *****************************************************************************/
/*
 LRM clause 4.1.4.2.
 The class sca_tdf::sca_ltf_nd shall implement a scaled continuous-time
 linear transfer function of the Laplace-domain variable s
 in the numerator-denominator form:
 H(s)=k * (num[0] + num[1]*s + ... num[M-1]*s**(M-1) ) /
          (den[0] + den[1]*s + ... den[N-1]*s**(N-1) )
 where k is the constant gain of the transfer function, M and N are the number
 of numerator and denominator coefficients, respectively, num[i] and den[i]
 are real-valued coefficients of the numerator and denominator,respectively.
 */

/*****************************************************************************/

#ifndef SCA_TDF_LTF_ND_H_
#define SCA_TDF_LTF_ND_H_

namespace sca_tdf
{

//begin implementation specific
namespace sca_implementation
{
class sca_ct_ltf_nd_proxy;
}

//end implementation specific


//class sca_ltf_nd : public implementation-derived-from sc_core::sc_object
class sca_ltf_nd: public ::sc_core::sc_object
{
public:
	sca_ltf_nd();
	explicit sca_ltf_nd(const char*);

	virtual const char* kind() const;

	void set_max_delay(sca_core::sca_time ct_delay);
	void set_max_delay(double ct_delay, sc_core::sc_time_unit unit);

	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_util::sca_vector<
					double>& state, double input, double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME);

	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state, double input, double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME);

	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_util::sca_vector<
					double>& state, const sca_util::sca_vector<double>& input,
			double k = 1.0, sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME);

	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay, sca_util::sca_vector<
					double>& state, const sca_util::sca_vector<double>& input,
			double k = 1.0, sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME);


	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_util::sca_vector<
					double>& state, const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);

	sca_tdf::sca_ct_proxy& calculate(const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay, sca_util::sca_vector<
					double>& state, const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);

	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& calculate( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);



	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_util::sca_vector<double>& state,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_util::sca_vector<double>& state,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den, sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			sca_util::sca_vector<double>& state,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			double input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);

	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_util::sca_vector<double>& input,
			double k = 1.0,
			sca_core::sca_time tstep = ::sc_core::SC_ZERO_TIME
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);

	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_in<double>& input,
			double k = 1.0
	);


	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);

	sca_tdf::sca_ct_proxy& operator() ( const sca_util::sca_vector<double>& num,
			const sca_util::sca_vector<double>& den,
			sca_core::sca_time ct_delay,
			const sca_tdf::sca_de::sca_in<double>& input,
			double k = 1.0
	);

//begin implementation specific

private:

	sca_tdf::sca_implementation::sca_ct_ltf_nd_proxy* proxy;

//end implementation specific



};

} // namespace sca_tdf


#endif /* SCA_TDF_LTF_ND_H_ */
