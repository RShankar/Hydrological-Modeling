#ifndef LSF_MODEL_H
#define LSF_MODEL_H
template<class dataType>
class LSF : public sc_core::sc_module 
{
	public:
        std::map<string, sca_tdf::sca_in<dataType>  *>  I;
        std::map<string, sca_tdf::sca_out<dataType> *>  Q;
        std::map<string, sca_lsf::sca_signal *>        sig;
        std::map<string, sca_lsf::sca_tdf_sink *>      vSi;
        std::map<string, sca_lsf::sca_tdf_source *>    vSo;
        std::map<string, sca_lsf::sca_add    *>  add;     //Weighted addition of two LSF signals.
        std::map<string, sca_lsf::sca_sub    *>  sub;     //Weighted subtraction of two LSF signals.
        std::map<string, sca_lsf::sca_gain   *>  gain;    //Multiplication of an LSF signal by a constant gain.
        std::map<string, sca_lsf::sca_dot    *>  dot;     //Scaled first-order time derivative of an LSF signal.
        std::map<string, sca_lsf::sca_integ  *>  integ;   //Scaled time-domain integration of an LSF signal.
        std::map<string, sca_lsf::sca_delay  *>  delay;   //Scaled time-delayed version of an LSF signal.
        std::map<string, sca_lsf::sca_source *>  source;  //LSF source.
        std::map<string, sca_lsf::sca_ltf_nd *>  ltf_nd;  //Scaled Laplace transfer function in the time-domain in the numerator-denominator form.
        std::map<string, sca_lsf::sca_ltf_zp *>  ltf_zp;  //Scaled Laplace transfer function in the time-domain in the zero-pole form.
        std::map<string, sca_lsf::sca_ss     *>  ss;      //Single-input single-output state-space equation.

        ~LSF()
        {
            deInitMapList<string, sca_tdf::sca_in<dataType>*>(&I);
            deInitMapList<string, sca_tdf::sca_out<dataType>*>(&Q);
            deInitMapList<string, sca_lsf::sca_signal *> (&sig);
            deInitMapList<string, sca_lsf::sca_tdf_sink *>(&vSi);
            deInitMapList<string, sca_lsf::sca_tdf_source *>(&vSo);
            deInitMapList<string, sca_lsf::sca_add    *>(&add); 
            deInitMapList<string, sca_lsf::sca_sub    *>(&sub);
            deInitMapList<string, sca_lsf::sca_gain   *>(&gain);
            deInitMapList<string, sca_lsf::sca_dot    *>(&dot);
            deInitMapList<string, sca_lsf::sca_integ  *>(&integ);
            deInitMapList<string, sca_lsf::sca_delay  *>(&delay);
            deInitMapList<string, sca_lsf::sca_source *>(&source);
            deInitMapList<string, sca_lsf::sca_ltf_nd *>(&ltf_nd);
            deInitMapList<string, sca_lsf::sca_ltf_zp *>(&ltf_zp);
            deInitMapList<string, sca_lsf::sca_ss     *>(&ss);
        };

        LSF(sc_core::sc_module_name nm):sc_core::sc_module(nm)
        {
            I["I"]              = new sca_tdf::sca_in<dataType>;
            Q["Q"]              = new sca_tdf::sca_out<dataType>;
        };      
};

typedef void (*lsfFunction)(LSF<double> &, string, string, double *);

void newSourceLSF(LSF<double> &lsf, string name, string Port, double * param)
{
    initMapItem<string, sca_lsf::sca_tdf_source>(lsf.vSo, name, name.c_str());
}

void newSinkLSF(LSF<double> &lsf, string name, string Port, double * param)
{
    initMapItem<string, sca_lsf::sca_tdf_sink>(lsf.vSi, name, name.c_str());
}

void newSignalLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_signal>(lsf.sig, name, name.c_str());
}

void newGainLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_gain>(lsf.gain,  name, name.c_str(), param[0]);
}

void newAddLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_add>(lsf.add,  name, name.c_str(), param[0], param[1]);
}

void newSubLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_sub>(lsf.sub,  name, name.c_str(), param[0], param[1]);
}

void newDotLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_dot>(lsf.dot,  name, name.c_str(), param[0]);
}

void newIntegLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   initMapItem<string, sca_lsf::sca_integ>(lsf.integ,  name, name.c_str(), param[0], param[1]);
}

void newDelayLSF(LSF<double> &lsf, string name, string Port, double * param)
{
    initMapItem<string, sca_lsf::sca_delay>(lsf.delay,  name, name.c_str(), sca_core::sca_time(param[0],sc_core::SC_SEC), param[1], param[2]);
}


void connectSourceLSF(LSF<double> &lsf, string name, string Port, double * param)
{
    connectInput<>(lsf.vSo[name], lsf.I[Port]);
}

void connectSinkLSF(LSF<double> &lsf, string name, string Port, double * param)
{
   connectOutput<>(lsf.vSi[name], lsf.Q[Port]);
}

void connectSourceY(LSF<double> &lsf, string name, string sigName, double * param)
{
     connectY<>(lsf.vSo[name], lsf.sig[sigName]);
}
void connectSinkX(LSF<double> &lsf, string name, string sigName, double * param)
{
       connectX<>(lsf.vSi[name], lsf.sig[sigName]);
}

void connectGainX(LSF<double> &lsf, string name, string sigName, double * param)
{
        connectX<>(lsf.gain[name], lsf.sig[sigName]);
}
void connectGainY(LSF<double> &lsf, string name, string sigName, double * param)
{
       connectY<>(lsf.gain[name], lsf.sig[sigName]);
}

void connectAddX1(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectX1<>(lsf.add[name], lsf.sig[sigName]);
}

void connectAddX2(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectX2<>(lsf.add[name], lsf.sig[sigName]);
}

void connectAddY(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectY<>(lsf.add[name], lsf.sig[sigName]);
}

void connectSubX1(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectX1<>(lsf.sub[name], lsf.sig[sigName]);
}

void connectSubX2(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectX2<>(lsf.sub[name], lsf.sig[sigName]);
}

void connectSubY(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectY<>(lsf.sub[name], lsf.sig[sigName]);
}

void connectDotX(LSF<double> &lsf, string name, string sigName, double * param)
{
   connectX<>(lsf.dot[name], lsf.sig[sigName]);
}


void connectDotY(LSF<double> &lsf, string name, string sigName, double * param)
{
   // lsf.dot[name]

   connectY<>(lsf.dot[name], lsf.sig[sigName]);
}

std::map<string, lsfFunction> lsfFuncList;

void lsfInitTable()
{
    lsfFuncList[string("New") +string("Source")] =   newSourceLSF;
    lsfFuncList[string("New") +string("Sink")]   =   newSinkLSF;
    lsfFuncList[string("New") +string("Signal")] =   newSignalLSF;
    lsfFuncList[string("New") +string("Gain")]   =   newGainLSF;
    lsfFuncList[string("New") +string("Add")]    =   newAddLSF;
    lsfFuncList[string("New") +string("Sub")]    =   newSubLSF;

    lsfFuncList[string("Source") +string("I")] = connectSourceLSF;
    lsfFuncList[string("Sink")   +string("Q")] = connectSinkLSF;
    lsfFuncList[string("Source") +string("Y")] = connectSourceY;
    lsfFuncList[string("Sink")   +string("X")] = connectSinkX;
    lsfFuncList[string("Gain")   +string("X")] = connectGainX;
    lsfFuncList[string("Gain")   +string("Y")] = connectGainY;
    
    lsfFuncList[string("Add")   +string("X1")] = connectAddX1;
    lsfFuncList[string("Add")   +string("X2")] = connectAddX2;
    lsfFuncList[string("Add")   +string("Y")]  = connectAddY;
    
    lsfFuncList[string("Sub")   +string("X1")] = connectAddX1;
    lsfFuncList[string("Sub")   +string("X2")] = connectAddX2;
    lsfFuncList[string("Sub")   +string("Y")]  = connectAddY;


}

#endif
