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

  sca_ac_domain_entity.cpp - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 02.01.2010

   SVN Version       :  $Revision: 1104 $
   SVN last checkin  :  $Date: 2011-01-09 11:45:25 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_ac_domain_entity.cpp 1104 2011-01-09 10:45:25Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#include "systemc-ams"
#include "scams/impl/analysis/ac/sca_ac_domain_entity.h"
#include "scams/impl/analysis/ac/sca_ac_domain_db.h"
#include "scams/impl/core/sca_solver_base.h"

namespace sca_ac_analysis
{
namespace sca_implementation
{

///////////////////////////////////////////////////////////////////////////////

unsigned long sca_ac_domain_entity::init_additional_equations(unsigned long eq_start)
{
    if( call_add_eq_method() )
    {
        number_of_add_eqs = Ae->n_cols();
    }

    start_of_add_eqs = eq_start;
    return number_of_add_eqs;
}

///////////////////////////////////////////////////////////////////////////////

sca_ac_domain_entity::sca_ac_domain_entity(sc_core::sc_module* mod) :
        ac_domain_method( NULL ),
        add_eq_method( NULL ),
        calc_add_eq_cons_method( NULL ),
        noise_src( false ),
        number_of_add_eqs( 0 ),
        start_of_add_eqs( 0 ),
        Ae( NULL ),
        Be( NULL ),
        qe( NULL ),
        module( mod ),
        solver( NULL ),
        obj( mod )
{}

///////////////////////////////////////////////////////////////////////////////

sca_ac_domain_entity::sca_ac_domain_entity(sca_core::sca_implementation::sca_solver_base* solv) :
        ac_domain_method( NULL ),
        add_eq_method( NULL ),
        calc_add_eq_cons_method( NULL ),
        noise_src( false ),
        number_of_add_eqs( 0 ),
        start_of_add_eqs( 0 ),
        Ae( NULL ),
        Be( NULL ),
        qe( NULL ),
        module( NULL ),
        solver( solv ),
        obj( solv )
{}


///////////////////////////////////////////////////////////////////////////////

sca_ac_domain_entity::~sca_ac_domain_entity()
{
    Ae     = NULL;
    Be     = NULL;
    qe     = NULL;
    module = NULL;
    solver = NULL;
    obj    = NULL;
    ac_domain_method        = NULL;
    add_eq_method           = NULL;
    calc_add_eq_cons_method = NULL;
}

///////////////////////////////////////////////////////////////////////////////

inline bool sca_ac_domain_entity::call_ac_domain_method()
{
    if( obj && ac_domain_method )
    {
    	ac_db->active_object=obj;
        (obj->*ac_domain_method)();
        ac_db->active_object=NULL;
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

inline bool sca_ac_domain_entity::call_add_eq_method()
{
    if( obj && add_eq_method )
    {
    	ac_db->active_object=obj;
        (obj->*add_eq_method)(Ae, Be, qe);
        ac_db->active_object=NULL;
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

inline void sca_ac_domain_entity::determine_ac_module_ports()
{
    sc_core::sc_interface* sca_if;
    std::vector<sc_core::sc_object*>&
    childs(const_cast<std::vector<sc_core::sc_object*>&>(module->get_child_objects()));

    //for all ports
    for(std::vector<sc_core::sc_object*>
            ::iterator  it  = childs.begin();
            it != childs.end();
            it++
       )
    {
        sc_core::sc_port_base* port_sc=dynamic_cast<sc_core::sc_port_base*>(*it);
        if(port_sc==NULL)
            continue;


        sca_core::sca_implementation::sca_port_base* portp=
        	dynamic_cast<sca_core::sca_implementation::sca_port_base*>(port_sc);
        if( (portp!=NULL) &&
                ( ( portp->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_SC_OUT_PORT ) |
                  ( portp->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_SC_IN_PORT ) ) )
        {
            sca_if = portp->sc_get_interface();
        }
        else
        {
            sca_if = port_sc->get_interface();
        }


        if(sca_if==NULL)
            continue;


        if(ac_db->arcs.find(sca_if) != ac_db->arcs.end()) //ignore non ac arcs
        {
            sca_core::sca_implementation::sca_port_base* port=
            	dynamic_cast<sca_core::sca_implementation::sca_port_base*>(port_sc);
            bool is_inport=false;
            if(port!=NULL)
            {
                if( ( port->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_IN_PORT ) |
                        ( port->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_SC_IN_PORT ) )
                {
                    is_inport=true;
                }
                else if( ( port->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_OUT_PORT ) |
                         ( port->get_port_type()==sca_core::sca_implementation::sca_port_base::SCA_SC_OUT_PORT ) )
                {
                    is_inport=false;
                }
                else
                {
                    std::ostringstream str;
                    str << "Ac-domain arcs can be connected to in- or "
                    << "outports only error in module: "
                    << module->name()
                    << std::endl;
                    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
                }
            }
            else
            {   //is there a better solution to find out/define the port
                //direction in general ???? - this should work for sc_in/sc_out
                std::string pkind(port_sc->kind());
                if(pkind=="sc_in")
                    is_inport=true;
                else if(pkind=="sc_out")
                    is_inport=false;
                else
                {
                    std::ostringstream str;
                    str << "Ac-domain arcs can be connected to in- or "
                    << "outports only error in module: "
                    << module->name()
                    << std::endl;
                    SC_REPORT_ERROR("SystemC-AMS",str.str().c_str());
                }
            }

            //if not in the map yet -> insert it
            if(ac_db->arc_map.find(sca_if) == ac_db->arc_map.end())
            {
                register long id = ac_db->arc_map.size();
                ac_db->arc_map[sca_if] = id;
            }


            if( is_inport)
            {
                //store access id for port
                ac_db->inport_map[port_sc] = inport_arcs.size();
                inport_arcs.push_back(ac_db->arc_map[sca_if]);
            }
            else
            {
                //store access id for port
                ac_db->outport_map[port_sc] = outport_arcs.size();
                outport_arcs.push_back(ac_db->arc_map[sca_if]);
            }
        }  //if ac arc
    } //for all ports

#ifdef SCA_IMPLEMENTATION_DEBUG
    std::cout << "Found module ports for: " << module->name() << ":" << std::endl
    << "\t" << inport_arcs.size() << " inports connected to: ";
    for(size_t i=0;
            i<inport_arcs.size()
            ;
            ++i) std::cout << inport_arcs[i] << " ";
    std::cout << "\n\t" << outport_arcs.size() << " outports connected to: ";
    for(size_t i=0;
            i<outport_arcs.size()
            ;
            ++i) std::cout << outport_arcs[i] << " ";
    std::cout << "\n\toverall number of arcs: " << ac_db->arc_map.size() << std::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////////

inline void sca_ac_domain_entity::determine_ac_solver_ports()
{
    sc_core::sc_interface* sca_if;

    sca_core::sca_implementation::sca_synchronization_obj_if* solv=solver;

    for(long i = 0,
            nin = solv->get_number_of_sync_ports(
            		sca_core::sca_implementation::sca_synchronization_obj_if::FROM_ANALOG);
            i < nin;
            ++i)
    {
        sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port* sync_port =
            solv->get_sync_port(
            		sca_core::sca_implementation::sca_synchronization_obj_if::FROM_ANALOG,i);
        sca_if = dynamic_cast<sc_core::sc_interface*>(sync_port->channel);

        //ignore non ac arcs
        if(ac_db->arcs.find(sca_if) == ac_db->arcs.end())
            continue;

        //if not in the map yet -> insert it
        if(ac_db->arc_map.find(sca_if) == ac_db->arc_map.end())
        {
            register long id = ac_db->arc_map.size();
            ac_db->arc_map[sca_if] = id;
        }

        sc_core::sc_port_base* port_sc=dynamic_cast<sc_core::sc_port_base*>(sync_port->port);
        ac_db->inport_map[port_sc] = inport_arcs.size();
        inport_arcs.push_back(ac_db->arc_map[sca_if]);	//store id
    }

    //do the same for ports from SystemC (I do not know whether it makes sense)
    for(long i = 0,
            nin = solv->get_number_of_sync_ports(
            		sca_core::sca_implementation::sca_synchronization_obj_if::FROM_SYSTEMC);
            i < nin;
            ++i)
    {
    	sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port* sync_port =
            solv->get_sync_port(
            		sca_core::sca_implementation::sca_synchronization_obj_if::FROM_SYSTEMC,i);
        sca_if = dynamic_cast<sc_core::sc_interface*>(sync_port->channel);

        //ignore non ac arcs
        if(ac_db->arcs.find(sca_if) == ac_db->arcs.end())
            continue;

        //if not in the map yet -> insert it
        if(ac_db->arc_map.find(sca_if) == ac_db->arc_map.end())
        {
            register long id = ac_db->arc_map.size();
            ac_db->arc_map[sca_if] = id;
        }

        sc_core::sc_port_base* port_sc=dynamic_cast<sc_core::sc_port_base*>(sync_port->port);
        ac_db->inport_map[port_sc] = inport_arcs.size();
        inport_arcs.push_back(ac_db->arc_map[sca_if]);	//store id
    }

    //now the same for outports
    for(long i = 0,
            nout = solv->get_number_of_sync_ports(
            		sca_core::sca_implementation::sca_synchronization_obj_if::TO_ANALOG);
            i < nout;
            ++i)
    {
    	sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port* sync_port =
            solv->get_sync_port(
            		sca_core::sca_implementation::sca_synchronization_obj_if::TO_ANALOG,i);
        sca_if = dynamic_cast<sc_core::sc_interface*>(sync_port->channel);

        //ignore non ac arcs
        if(ac_db->arcs.find(sca_if) == ac_db->arcs.end())
            continue;

        //if not in the map yet -> insert it
        if(ac_db->arc_map.find(sca_if) == ac_db->arc_map.end())
        {
            register long id = ac_db->arc_map.size();
            ac_db->arc_map[sca_if] = id;
        }

        sc_core::sc_port_base* port_sc=dynamic_cast<sc_core::sc_port_base*>(sync_port->port);
        ac_db->outport_map[port_sc] = outport_arcs.size();
        outport_arcs.push_back(ac_db->arc_map[sca_if]);	//store id
    }

    for(long i = 0,
            nout = solv->get_number_of_sync_ports(
            		sca_core::sca_implementation::sca_synchronization_obj_if::TO_SYSTEMC);
            i < nout;
            ++i)
    {
    	sca_core::sca_implementation::sca_synchronization_obj_if::sca_sync_port* sync_port =
            solv->get_sync_port(
            		sca_core::sca_implementation::sca_synchronization_obj_if::TO_SYSTEMC,i);
        sca_if = dynamic_cast<sc_core::sc_interface*>(sync_port->channel);

        //ignore non ac arcs
        if(ac_db->arcs.find(sca_if) == ac_db->arcs.end())
            continue;

        //if not in the map yet -> insert it
        if(ac_db->arc_map.find(sca_if) == ac_db->arc_map.end())
        {
            register long id = ac_db->arc_map.size();
            ac_db->arc_map[sca_if] = id;
        }

        sc_core::sc_port_base* port_sc=dynamic_cast<sc_core::sc_port_base*>(sync_port->port);
        ac_db->outport_map[port_sc] = outport_arcs.size();
        outport_arcs.push_back(ac_db->arc_map[sca_if]);	//store id
    }
}

//////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_entity::initialize()
{
    //determine ports for modules -> create arc mapping table
    //is performed only if module!=NULL
    if( module!=NULL )
        determine_ac_module_ports();
    //do the same for a solver
    if( solver!=NULL )
        determine_ac_solver_ports();

#ifdef SCA_IMPLEMENTATION_DEBUG

    std::cout << "Number of in arcs: " << inport_arcs.size()
    << " number of out arcs: " << outport_arcs.size() << std::endl;
#endif

    A.resize(inport_arcs.size(),outport_arcs.size());
    B.resize(outport_arcs.size());
}

///////////////////////////////////////////////////////////////////////////////

inline bool sca_ac_domain_entity::call_add_eq_outp_con_method
( sca_util::sca_matrix<sca_util::sca_complex >*& con_matr,
		sca_util::sca_vector<sca_util::sca_complex >& y)
{
    if( obj && calc_add_eq_cons_method )
    {
        (obj->*calc_add_eq_cons_method)(con_matr, y);
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

inline void sca_ac_domain_entity::computeAB(double w)
{
    //modules without outports can be ignored
    if(outport_arcs.size()==0)
        return;

    sca_util::sca_vector<sca_util::sca_complex > x, y;
    x.resize(inport_arcs.size());
    y.resize(outport_arcs.size());

    //input vector x is initialized to zero
    // -> computation of B
    ac_db->set_current_node_data(&x, &B, w,true);
    call_ac_domain_method();

    //computation of A by setting the inports sequentially to 1
    ac_db->set_current_node_data(&x, &y, w, false);
    for(unsigned long ipc=0; ipc<inport_arcs.size(); ++ipc)
    {
        x[ipc] = 1.0;
        call_ac_domain_method();
        x[ipc] = 0.0;

        for(unsigned long opc=0; opc<outport_arcs.size(); ++opc)
            A(ipc,opc) = y[opc] - B[opc];
    }
}

///////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_entity::computeAB_insert( sca_util::sca_matrix<sca_util::sca_complex >& Ag,
        sca_util::sca_vector<sca_util::sca_complex >& Bg,
        double w )
{
    if( ac_domain_method )
    {
        //compute small instance matrices y = A * x + B
        computeAB(w);

        //insert values into the global equation system
        for(unsigned long opc=0; opc<outport_arcs.size(); ++opc)
        {
            long outport_eq = outport_arcs[opc];

            Ag(outport_eq, outport_eq) = -1.0;	//outport "variable"

            for(unsigned long ipc=0; ipc<inport_arcs.size(); ++ipc)
                Ag(inport_arcs[ipc],outport_eq) += A(ipc,opc);

            Bg(outport_eq) = -B[opc];
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void sca_ac_domain_entity::setup_add_eqs(
		sca_util::sca_matrix<sca_util::sca_complex >& Ag,
		sca_util::sca_vector<sca_util::sca_complex >& Bg,
        double w )
{
	sca_util::sca_matrix<sca_util::sca_complex > *out_con_matrix = NULL;

    //if no additional equations are available
    if(add_eq_method==0)
        return;

    sca_util::sca_vector<sca_util::sca_complex>  x_in;
    x_in.resize(inport_arcs.size());

    ac_db->set_current_node_data(&x_in, &B, w,true);

    //first call with x_in initialized to 0
    // -> computation of Ae, Be, qe, and out_con_matrix
    call_add_eq_method();
    call_add_eq_outp_con_method(out_con_matrix, B);

#ifdef SCA_IMPLEMENTATION_DEBUG

    std::cout << "Additional eq-system: " << std::endl;
    std::cout << "Ae : " << std::endl << *Ae << std::endl
    << "Be : " << std::endl << *Be << std::endl
    << "qe : " << std::endl << *qe << std::endl;
    if(out_con_matrix)
        std::cout << "out_con_matrix : " << std::endl << *out_con_matrix << std::endl;
#endif

    sca_util::sca_complex *cb = qe->get_flat();

    for (unsigned long i=0, eqnr=start_of_add_eqs; i<number_of_add_eqs; ++i, ++eqnr)
    {
        for(unsigned long j=0; j<number_of_add_eqs; ++j)
        {   // attention on order of dimensions!
            Ag(start_of_add_eqs+j,eqnr) =
            	sca_util::sca_complex((*Be)(i,j),w * (*Ae)(i,j));
        }
        Bg(eqnr) = -cb[i];	// only real input data! (real qe)
    }


    ////// connectivity to other domains ///////
    ac_db->set_current_node_data(&x_in, &B, w,false);
    //inports
    for(unsigned long i=0; i<inport_arcs.size(); ++i)
    {
        //get number of signals
        unsigned long inport_eq = inport_arcs[i];

        //compute contribution of inport i
        x_in[i] = 1.0;
        call_add_eq_method();
        cb = qe->get_flat();
        x_in[i] = 0.0;

        //ignore contribution of -B
        for(unsigned long j=0, eqnr=start_of_add_eqs; j<number_of_add_eqs; ++j, ++eqnr)
            Ag(inport_eq,eqnr) += Bg(eqnr) + cb[j];
    }

    //outports
    for(unsigned long i=0; i<outport_arcs.size(); ++i)
    {
        unsigned long outport_eq = outport_arcs[i];

        Ag(outport_eq,outport_eq) = -1.0;	//outport "variable"

        if(out_con_matrix)
            for(unsigned long j=0; j<number_of_add_eqs; ++j)
                Ag(start_of_add_eqs+j,outport_eq) = (*out_con_matrix)(j,i);
    }

    if(solver!=NULL) ac_db->add_eq_start_map[solver]=start_of_add_eqs;
}

///////////////////////////////////////////////////////////////////////////////


} // namespace sca_implementation
} // namespace sca_ac_analysis


