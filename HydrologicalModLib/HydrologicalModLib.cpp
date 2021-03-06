#define DLLEXPORT extern "C" __declspec(dllexport)

#include <iostream>
#include <vector>
#include <list>

#include "HydrologicalModLib.h"
#include "ELN.H"
#include "LSF.H"
#include "TDF.H"

using namespace std;
typedef class object
{
    public:
        object(){};
        ~object()
        {
            switch(type)
            {
                case ELN_MOC:
                    
                    delete (ELN<double> *)(p);
                    break;
                case LSF_MOC:
                    
                    delete (LSF<double> *)(p);
                    break;
                case TDF_MOC:
                    
                     delete (TDF<double> *)(p);
                    break;
            }
        };

        void *p;
        MOC_TYPE type;
} object;


std::map<string, object*> MoC;
std::map<string, sca_tdf::sca_signal<double> *> signalList;
std::map<string, sca_util::sca_trace_file*> traceHandle;

DLLEXPORT int startSim(double duration)
{
    sc_core::sc_report_handler::initialize();
    sc_core::sc_report_handler::set_log_file_name("hydrologicalSimLog.txt");
    sc_core::sc_report_handler::set_actions (sc_core::SC_INFO,    sc_core::SC_LOG);
    sc_core::sc_report_handler::set_actions (sc_core::SC_WARNING, sc_core::SC_LOG);
    sc_core::sc_report_handler::set_actions (sc_core::SC_ERROR,   sc_core::SC_LOG);

    cout<<"Start Simulation"<<endl;

    sc_core::sc_start(duration, sc_core::SC_SEC);
    sc_core::sc_report_close_default_log();
    
    deInitMapList<string, sca_tdf::sca_signal<double> *>(&signalList);
    deInitMapList<string, object*>(&MoC);    
    std::map<string, sca_util::sca_trace_file*> *item = &traceHandle;

    for (std::map<string, sca_util::sca_trace_file*>::iterator it = (*item).begin(); it!=(*item).end(); ++it) 
    {
        std::cout << "Closing trace file " <<it->first << '\n';
        sca_util::sca_close_tabular_trace_file(it->second);
        delete it->second;
    }

    return 1;
}

DLLEXPORT int createSignal( char *signalName )
{  
    return initMapItem<string, sca_tdf::sca_signal<double>>(signalList, string(signalName));
}

template<class A>
static void connectInputMoCSignal(A &obj, string port, string signalName)
{
    (*obj.I[port])(*signalList[signalName]);
}

template<class A>
static void connectOutputMoCSignal(A &obj, string port, string signalName)
{
    (*obj.Q[port])(*signalList[signalName]);
}

template<class A>
static int _connectMoCTemplate( char *mocName, char *portInput, char *sigInput, char *portOutput, char *sigOutput)
{  
   
    A *obj = (A *)MoC[string(mocName)]->p;
   connectInputMoCSignal<A>((*obj),  string(portInput), string(sigInput));
   connectOutputMoCSignal<A>((*obj), string(portOutput), string(sigOutput));
   return 1;
}

static int _connectMoC( char *mocName, char *portInput, char *sigInput, char *portOutput, char *sigOutput)
{  
   
    switch(MoC[string(mocName)]->type)
    {
        case ELN_MOC:
            
            return _connectMoCTemplate<ELN<double>>(mocName, portInput, sigInput, portOutput, sigOutput);
            break;
        case LSF_MOC:
            
            return _connectMoCTemplate<LSF<double>>(mocName, portInput, sigInput, portOutput, sigOutput);
            break;
        case TDF_MOC:
            
            return _connectMoCTemplate<TDF<double>>(mocName, portInput, sigInput, portOutput, sigOutput);
            break;
    }

   return 1;

}

DLLEXPORT int connectMoC( char *mocName, char *portInput, char *sigInput, char *portOutput, char *sigOutput)
{  
   return _connectMoC(mocName, portInput, sigInput, portOutput, sigOutput);
}

DLLEXPORT int createTDFMoC( char *mocName, double flowTimeStep, mocFunction processingFunc)
{  
    
    MoC[string(mocName)] = new object;
    MoC[string(mocName)]->p = (void*) (new TDF<double>( mocName, processingFunc, sca_core::sca_time( flowTimeStep, sc_core::SC_SEC)) );
    MoC[string(mocName)]->type = TDF_MOC;
    
    return 1;
}

DLLEXPORT int createLSFMoC( char *mocName, char *lsfConfigFile)
{  
    /*Not implemented*/
    return 1;
}

DLLEXPORT int createELNMoC( char *mocName)
{  
   //Not Implemented
    return 1;
}

DLLEXPORT int createTraceFile( char *fileName)
{
    traceHandle[string(fileName)] = sca_util::sca_create_tabular_trace_file(fileName);
    return 1;
}

DLLEXPORT int traceSignal( char *fileName, char *signalName)
{ 
    sca_util::sca_trace(traceHandle [string(fileName)], (*signalList[string(signalName)]),   signalName);
    return 1;
}