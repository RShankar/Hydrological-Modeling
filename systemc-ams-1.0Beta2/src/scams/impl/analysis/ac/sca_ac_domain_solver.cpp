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

  sca_ac_domain_solver.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 02.01.2010

   SVN Version       :  $Revision: 1108 $
   SVN last checkin  :  $Date: 2011-01-09 12:31:33 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_ac_domain_solver.cpp 1108 2011-01-09 11:31:33Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/analysis/ac/sca_ac_domain_solver.h"
#include "scams/impl/core/sca_simcontext.h"
#include "scams/impl/analysis/ac/sca_solve_ac_linear.h"
#include "scams/impl/analysis/ac/sca_ac_domain_db.h"


namespace sca_ac_analysis
{
namespace sca_implementation
{




int solve_linear_complex_eq_system::solve(sca_util::sca_matrix<sca_util::sca_complex >& Ac,
        sca_util::sca_vector<sca_util::sca_complex >& Bc,
        sca_util::sca_vector<sca_util::sca_complex >& Xc)
{
	critical_row=-1;
	critical_column=-1;


    unsigned long dima=Ac.n_cols();
    sca_util::sca_matrix<double> a(2*dima,2*dima);

    //converting to real valued equation system
    for(unsigned long i=0; i<dima; ++i)
        for(unsigned long j=0; j<dima; ++j)
        {
            a(j,i)          = Ac(i,j).real(); // --                   --
            a(j,dima+i)     =-Ac(i,j).imag(); // |  Real(A)  -Imag(A)  |
            // |                     |
            a(dima+j,i)     = Ac(i,j).imag(); // |  Imag(A)   Real(A)  |
            a(dima+j,dima+i)= Ac(i,j).real(); // --                   --
        }

    unsigned long dimb=Bc.length();
    sca_util::sca_vector<double> b(2*dimb);

    for(unsigned long i=0; i<dimb; ++i)
    {
        b(i)     =Bc(i).real();      // |  Real(B) |
        b(dimb+i)=Bc(i).imag();      // |  Imag(B) |
    }

    //   cout << "a: " << a << endl << endl;
    //   cout << "b: " << b << endl << endl;

    sca_util::sca_vector<double> sigs(2*dimb);
    sca_solve_ac_linear_data* sdata = NULL;
    int errc=sca_solve_ac_linear_init(a.get_flat(), 2*dima,&sdata);

    if(errc!=0)
    {
    	if(errc==2)
    	{
    		long row=-1, column=-1;
    		sca_solve_ac_get_error_position(sdata,&row,&column);

    		//if error in imaginary part -> normalize
    		if(row>=long(dimb))    row-=dimb;
    		if(column>=long(dimb)) column-=dimb;

    		critical_row=row;
    		critical_column=column;
    	}


        sca_solve_ac_linear_free(&sdata);
    	return errc;
    }



    //solving the real equation system
    errc=sca_solve_ac_linear(b.get_flat(), sigs.get_flat(),&sdata);

    if(errc!=0)
    {
    	sca_solve_ac_linear_free(&sdata);
        return 3;
    }



    //converting result to complex
    for(unsigned long i=0; i<dimb; ++i)
    {
        Xc(i)=sca_util::sca_complex(sigs(i),sigs(dimb+i));
    }

    sca_solve_ac_linear_free(&sdata);
    return errc;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//constructor
sca_ac_domain_solver::sca_ac_domain_solver(sca_ac_domain_db& ac_db) :
        ac_data(ac_db),
        equations(ac_db)
{
    ac_data.ac_domain    = true;
    ac_data.initialized  = false;

    if(!(sca_core::sca_implementation::sca_get_curr_simcontext()->initialized()))
    {
        init_systemc();
    }

    equations.initialize_equation_system();

    ac_data.ac_domain    = false;

    ac_data.initialized=true;
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::solve_complex_eq_system(
    sca_util::sca_matrix<sca_util::sca_complex >& Ac,
    sca_util::sca_vector<sca_util::sca_complex >& Bc,
    sca_util::sca_vector<sca_util::sca_complex >& Xc
)
{
	solve_linear_complex_eq_system solver;

	int err=solver.solve(Ac,Bc,Xc);

    if(err!=0)
    {
        std::ostringstream str;
        if(err==1)
        {
            str << " Creation of sparse matrix failed (can't allocate memory) for frequency: "
            << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;
        }
        else if(err==2)
        {
        	//currently only for signals and nodes
        	sc_core::sc_object* row_obj=this->ac_data.find_object_for_id(solver.critical_row);
        	sc_core::sc_object* col_obj=this->ac_data.find_object_for_id(solver.critical_column);

            str << " Creation of SparseCode failed for frequency: "
            << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;

            if((row_obj!=NULL) || (col_obj!=NULL))
            {
            	str << "The error is may be near: " << std::endl << "\t\t";
            	if(row_obj!=NULL)
            	{
            		str << row_obj->name();

            		if( (col_obj!=NULL)&&(row_obj!=col_obj) )
            		{
            			str << " and "  << std::endl << "\t\t";
            		}
            	}

            	if( (col_obj!=NULL)&&(row_obj!=col_obj) )
            	{
            		str << col_obj->name();
            	}
            	str << std::endl;
            }
        }
        else
        {
        	  std::ostringstream str;
        	  str << "Equation solver failed for frequency: "
        	      << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;
        }
        SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::ac_noise_solver(
    sca_util::sca_matrix<sca_util::sca_complex >& Ac,
    sca_util::sca_vector<sca_util::sca_complex >& Bc,
    std::vector<bool>&                 noise_src_flags,
    sca_util::sca_matrix<sca_util::sca_complex >& result
)
{
    //noise analysis - assumption all b!=0 are independent noise sources
    //for those sources there contribution has to be calculated seperately
    //and the contribution has to be added arithmeticly

    unsigned long dima=Ac.n_cols();
    unsigned long n_equations=dima;
    sca_util::sca_matrix<double> a(2*n_equations,2*n_equations);

    //reset overall noise vector
    for(unsigned long i=0;i<n_equations;i++)
        result(0,i)=0.0;

    //converting to real valued equation system
    for(unsigned long i=0; i<dima; ++i)
        for(unsigned long j=0; j<dima; ++j)
        {
            a(j,i)          = Ac(i,j).real(); // --                   --
            a(j,dima+i)     =-Ac(i,j).imag(); // |  Real(A)  -Imag(A)  |
            // |                     |
            a(dima+j,i)     = Ac(i,j).imag(); // |  Imag(A)   Real(A)  |
            a(dima+j,dima+i)= Ac(i,j).real(); // --                   --
        }


    sca_solve_ac_linear_data* sdata = NULL;
    int errc=sca_solve_ac_linear_init(a.get_flat(), 2*dima,&sdata);
    if(errc!=0)
    {
        std::ostringstream str;
        if(errc==1)
        {
            str << " Creation of sparse matrix failed (can't allocate memory) for frequency: "
            << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;
        }
        else
        {
            str << " Creation of SparseCode failed for frequency: "
            << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;

        	long row=-1, column=-1;
        	sca_solve_ac_get_error_position(sdata,&row,&column);

        	//if error in imaginary part -> normalize
        	if(row>=long(dima))    row-=dima;
        	if(column>=long(dima)) column-=dima;

        	//currently only for signals and nodes
        	sc_core::sc_object* row_obj=this->ac_data.find_object_for_id(row);
        	sc_core::sc_object* col_obj=this->ac_data.find_object_for_id(column);

            str << " Creation of SparseCode failed for frequency: "
            << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;

            if((row_obj!=NULL) || (col_obj!=NULL))
            {
            	str << "The error is may be near: " << std::endl << "\t\t";
            	if(row_obj!=NULL)
            	{
            		str << row_obj->name();

            		if( (col_obj!=NULL)&&(row_obj!=col_obj) )
            		{
            			str << " and "  << std::endl << "\t\t";
            		}
            	}

            	if( (col_obj!=NULL)&&(row_obj!=col_obj) )
            	{
            		str << col_obj->name();
            	}
            	str << std::endl;
            }
        }
        SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());
    }
    else
    {

        unsigned long dimb=n_equations;
        sca_util::sca_vector<double> b;
        sca_util::sca_vector<double> xnoise;


        b.resize(2*n_equations);
        xnoise.resize(2*n_equations);

        long n_src=1;
        for(unsigned long i=0; i<n_equations; ++i)
        {
            if((noise_src_flags[i]) )
            {
                b.reset(); //sca_solvelinearc destroys the right-hand-vector !!!!
                b(i)      = Bc(i).real();
                b(dimb+i) = Bc(i).imag();

                errc=sca_solve_ac_linear(b.get_flat(), xnoise.get_flat(),&sdata);

                if(errc!=0)
                {
                    std::ostringstream str;
                    str << "Equation solver failed for frequency: "
                    << sca_ac_analysis::sca_ac_f() << " Hz"<< std::endl;
                    SC_REPORT_WARNING("SystemC-AMS",str.str().c_str());
                }



                for(unsigned long k=0; k<n_equations; ++k)
                {
                    sca_util::sca_complex res(xnoise(k),xnoise(dimb+k));
                    result(k,0)+=norm(res);
                    result(k,n_src)=res;
                }
                n_src++;
            }
        }

        //square root of arithmetic sum
        for(unsigned long j=0; j<n_equations; ++j)
        {
            result(j,0)=sqrt(result(j,0));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::calculate(std::vector<double> omegas)
{
    ac_data.ac_domain    = true;
    ac_data.noise_domain = false;

    sca_core::sca_implementation::sca_get_curr_simcontext()->set_no_time_domain_simulation();

    sca_util::sca_vector<sca_util::sca_complex >& result(ac_data.get_result_vector_ref());
    result.reset();

    trace_init();

    for(std::vector<double>::iterator wi=omegas.begin(); wi<omegas.end(); ++wi)
    {
        equations.setup_equations(*wi);

#ifdef SCA_IMPLEMENTATION_DEBUG

        std::cout << "Ag: " << equations.get_Ag().dimx() << std::endl
        << equations.get_Ag() << endl << endl;
        std::cout << "Bg: " << equations.get_Bg().dimx() << std::endl
        << equations.get_Bg() << std::endl << std::endl;
#endif

        solve_complex_eq_system(equations.get_Ag(),equations.get_Bg(),result);

#ifdef SCA_IMPLEMENTATION_DEBUG

        std::cout << "result: " << result.dimx() << std::endl << result << std::endl << std::endl;
#endif

        //////////////// tracing ////////////////////
        trace(*wi,result);
    }

    sca_core::sca_implementation::sca_get_curr_simcontext()->set_time_domain_simulation();

    ac_data.ac_domain = false;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::calculate_noise(std::vector<double> omegas)
{
    ac_data.ac_domain    = true;
    ac_data.noise_domain = true;
    ac_data.equations=&equations; //set reference to be able to access
    //from global routines->required for noise

    sca_core::sca_implementation::sca_get_curr_simcontext()->set_no_time_domain_simulation();

    sca_util::sca_vector<sca_util::sca_complex >& result(ac_data.get_result_vector_ref());
    result.reset();

    if(omegas.size()>0)
        equations.setup_equations(omegas[0]); //get noise sources

    //detect noise_src, store names
    sca_util::sca_vector<std::string> src_names;
    std::vector<bool>   noise_src_flags(equations.get_noise_src_names().length());
    long n_src=0;
    for(unsigned int i=0;i<equations.get_noise_src_names().length();i++)
    {
        if(equations.get_noise_src_names()(i)!="")
        {
            src_names(n_src++)=equations.get_noise_src_names()(i);
            noise_src_flags[i]=true;
        }
        else
        {
            noise_src_flags[i]=false;
        }
    }

    trace_init_noise(src_names);


    sca_util::sca_matrix<sca_util::sca_complex > results;
    results.resize(n_src+1,result.length());
    for(std::vector<double>::iterator wi=omegas.begin(); wi<omegas.end(); ++wi)
    {
        equations.get_Bgnoise().reset();
        equations.setup_equations(*wi);

        ac_noise_solver(equations.get_Ag(),equations.get_Bgnoise(),noise_src_flags,results);

        trace_noise(*wi,results);
    }

    sca_core::sca_implementation::sca_get_curr_simcontext()->set_time_domain_simulation();

    ac_data.ac_domain    = false;
    ac_data.noise_domain = false;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::trace_init_noise(sca_util::sca_vector<std::string>& src_names)
{
    ac_data.init_arc_map_for_add_eqs();

    std::vector<sca_util::sca_implementation::sca_trace_file_base*>* tr_list=
    	sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();

    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>::iterator tr_it  = tr_list->
            begin();
            tr_it != tr_list->end();
            ++tr_it )
    {
        if((*tr_it)->trace_disabled())
            continue;
        (*tr_it)->write_ac_noise_domain_init(src_names);
    }

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::trace_init()
{
    ac_data.init_arc_map_for_add_eqs();

    std::vector<sca_util::sca_implementation::sca_trace_file_base*>* tr_list=
    	sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();

    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>::iterator tr_it  = tr_list->
            begin();
            tr_it != tr_list->end();
            ++tr_it )
    {
        if((*tr_it)->trace_disabled())
            continue;
        (*tr_it)->write_ac_domain_init();
    }

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::trace_noise(double w, sca_util::sca_matrix<sca_util::sca_complex >& result)
{
    std::vector<sca_util::sca_implementation::sca_trace_file_base*>* tr_list=
    	sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();
    result.unset_auto_resizable();

    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>::iterator tr_it  = tr_list->
            begin();
            tr_it != tr_list->end();
            ++tr_it )
    {
        if((*tr_it)->trace_disabled())
            continue;

        long n_src=result.n_rows();
        sca_util::sca_matrix<sca_util::sca_complex > tr_matrix(n_src,(*tr_it)->traces.size());
        tr_matrix.unset_auto_resizable();

        sc_core::sc_interface* sca_if;

        long npoint=0;
        for(std::vector<sca_util::sca_implementation::sca_trace_object_data>::iterator
                tr_objit =  (*tr_it)->
                            traces.begin();
                tr_objit != (*tr_it)->traces.end();
                ++tr_objit
           )
        {
        	sca_util::sca_traceable_object* tobj = (*tr_objit).trace_object;

            if(tobj)
            {
                sc_core::sc_port_base* port = dynamic_cast<sc_core::sc_port_base*>(tobj);
                if(port)
                    sca_if  = dynamic_cast<sc_core::sc_interface*>(port->get_interface());
                else
                    sca_if  = dynamic_cast<sc_core::sc_interface*>(tobj);

                if(sca_if)
                {
                    long id = ac_data.get_arc_id(sca_if);
                    if(id<0)
                        tr_matrix(0,npoint)=0.0;
                    else
                        for(long i=0;i<n_src;i++)
                            tr_matrix(i,npoint)=result(i,id);
                }
                else //tracing for modules (may current tracing for networks)
                {
                    sca_core::sca_module* mod;
                    mod = dynamic_cast<sca_core::sca_module*>(tobj);
                    if(mod==NULL)
                    {
                        tr_matrix(0,npoint)=0.0;
                        continue;
                    }

                    sca_core::sca_implementation::sca_solver_base* solv=mod->get_sync_domain();

                    if(solv==NULL)
                    {
                        tr_matrix(0,npoint)=0.0;
                        continue;
                    }

                    long adr_start=ac_data.get_start_of_add_equations(solv);
                    if(adr_start>=0)
                    {
                    	for(long i=0;i<n_src;i++)
                    	{
                        	sca_util::sca_complex* res=&(result(i,0)) + adr_start;
                        	tr_matrix(i,npoint)=tobj->calculate_ac_result(res);
                    	}
                    }
                    else
                    {
                        tr_matrix(0,npoint)=0.0;
                    }

                }
                npoint++;
            }
        }
        (*tr_it)->write_ac_noise_domain_stamp(w,tr_matrix);
    }
}

/////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::trace(double w, sca_util::sca_vector<sca_util::sca_complex >& result)
{
    std::vector<sca_util::sca_implementation::sca_trace_file_base*>* tr_list=
    	sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();

    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>::iterator tr_it  =
    	tr_list->begin();
            tr_it != tr_list->end();
            ++tr_it )
    {
        if((*tr_it)->trace_disabled())
            continue;
        std::vector<sca_util::sca_complex > tr_vec;
        sc_core::sc_interface* sca_if;

        for(std::vector<sca_util::sca_implementation::sca_trace_object_data>::iterator
                tr_objit =  (*tr_it)->
                            traces.begin();
                tr_objit != (*tr_it)->traces.end();
                ++tr_objit
           )
        {
            sca_util::sca_traceable_object* tobj = (*tr_objit).trace_object;

            if(tobj)
            {
                sc_core::sc_port_base* port = dynamic_cast<sc_core::sc_port_base*>(tobj);
                if(port)
                    sca_if  = dynamic_cast<sc_core::sc_interface*>(port->get_interface());
                else
                    sca_if  = dynamic_cast<sc_core::sc_interface*>(tobj);

                if(sca_if)
                {
                    long id = ac_data.get_arc_id(sca_if);
                    if(id<0)
                        tr_vec.push_back(0.0);
                    else
                        tr_vec.push_back(result(id));
                }
                else //tracing for modules (may current tracing for networks)
                {
                    sca_core::sca_module* mod;
                    mod = dynamic_cast<sca_core::sca_module*>(tobj);
                    if(mod==NULL)
                    {
                        tr_vec.push_back(0.0);
                        continue;
                    }

                    sca_core::sca_implementation::sca_solver_base* solv=mod->get_sync_domain();

                    if(solv==NULL)
                    {
                        tr_vec.push_back(0.0);
                        continue;
                    }

                    long adr_start=ac_data.get_start_of_add_equations(solv);
                    if(adr_start>=0)
                    {
                        sca_util::sca_complex* res=result.get_flat() + adr_start;
                        tr_vec.push_back(tobj->calculate_ac_result(res));
                    }
                    else
                    {
                        tr_vec.push_back(0.0);
                    }

                }
            }
        }
        (*tr_it)->write_ac_domain_stamp(w,tr_vec);
    }
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_solver::init_systemc()
{

    std::vector<sca_util::sca_implementation::sca_trace_file_base*>* tr_list=
    	sca_core::sca_implementation::sca_get_curr_simcontext()->get_trace_list();

    std::vector<bool> en_list(tr_list->size());
    std::vector<bool>::iterator enit=en_list.begin();

    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>::iterator
    		tr_it  = tr_list->begin();
            tr_it != tr_list->end();
            ++tr_it, ++enit )
    {
        if((*tr_it)->trace_disabled())
        {
            (*enit)=false;
        }
        else
        {
            (*tr_it)->disable();
            (*enit)=true;
        }
    }


    sc_core::sc_start(sc_core::SC_ZERO_TIME); //unfourtunately all initial deltas will be performed - also
    //the first call of the df-cluster

    enit=en_list.begin();
    for(std::vector<sca_util::sca_implementation::sca_trace_file_base*>
            ::iterator tr_it  = tr_list->begin();
            tr_it != tr_list->end();
            ++tr_it, ++enit )
    {
        if(*enit)
            (*tr_it)->enable_not_init();
    }

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


} // namespace sca_implementation
} // namespace sca_ac_analysis

