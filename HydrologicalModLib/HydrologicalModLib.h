#ifndef _HYDROLOGICAL_MOD_H
#define _HYDROLOGICAL_MOD_H

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc-ams>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <map>
using namespace std;

typedef enum TYPE 
{
    ELN_MOC, 
    LSF_MOC, 
    TDF_MOC, 
} MOC_TYPE;


typedef int (*inputFunction)(double t, float ***);
typedef int (*mocFunction)(double t, double *,  double *);

template<class A, class B>
void deInitMapList(std::map<A, B> *item)
{

    for (std::map<A, B>::iterator it = (*item).begin(); it!=(*item).end(); ++it) 
    {
       // std::cout << it->first << '\n';
        delete it->second;

    }
}

//template<class A, class B, class C>
template< class A, class B, class C>
int initMapItem(std::map<A, B> &mapList, A name)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new C;
        return 1;
    } 

    return 0;
}

template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B;
        return 1;
    } 

    return 0;
}
template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name, sc_core::sc_module_name objName)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B(objName);
        return 1;
    } 

    return 0;
}
template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name, sc_core::sc_module_name objName, double param1)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B(objName, param1);
        return 1;
    } 

    return 0;
}

template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name, sc_core::sc_module_name objName, double param1, double param2)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B(objName, param1, param2);
        return 1;
    } 

    return 0;
}
template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name, sc_core::sc_module_name objName,  sca_core::sca_time param1, double param2, double param3)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B(objName, param1, param2, param3);
        return 1;
    } 

    return 0;
}
template< class A, class B>
int initMapItem(std::map<A, B*> &mapList, A name, sc_core::sc_module_name objName, double param1, double param2, double param3)
{
    if ( mapList.find(name) == mapList.end() ) 
    {
        mapList[name] = new B(objName, param1, param2, param3);
        return 1;
    } 

    return 0;
}
template<class A, class B>
int connectMoCSignal(A &MoC, string portName, B &signalList, string signalName)
{
    MoC.I[portName](signalList[signalName]);
}

template<class A, class B>
static void connectInput(A &objInput, B &objOutput)
{
    (*objInput).inp(*objOutput);
}

template<class A, class B>
static void connectOutput(A &objInput, B &objOutput)
{
    (*objInput).outp(*objOutput);
}

template<class A, class B>
static void connectPos(A &objInput, B &objOutput)
{
    (*objInput).p(*objOutput);
}

template<class A, class B>
static void connectNeg(A &objInput, B &objOutput)
{
    (*objInput).n(*objOutput);
}

template<class A, class B>
static void connectX(A &objInput, B &objOutput)
{
    (*objInput).x(*objOutput);
}
template<class A, class B>
static void connectX1(A &objInput, B &objOutput)
{
    (*objInput).x2(*objOutput);
}
template<class A, class B>
static void connectX2(A &objInput, B &objOutput)
{
    (*objInput).x2(*objOutput);
}
template<class A, class B>
static void connectY(A &objInput, B &objOutput)
{
    (*objInput).y(*objOutput);
}



#endif

