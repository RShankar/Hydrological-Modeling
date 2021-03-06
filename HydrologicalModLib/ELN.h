#ifndef ELN_MODEL_H
#define ELN_MODEL_H

template<class dataType>
class ELN : public sc_core::sc_module 
{
	public:

        std::map<string, sca_tdf::sca_in<dataType>  *>  I;
        std::map<string, sca_tdf::sca_out<dataType> *>  Q;
        std::map<string, sca_eln::sca_node *>         n;
        std::map<string, sca_eln::sca_node_ref *>     g;
        std::map<string, sca_eln::sca_r *>            r;
        std::map<string, sca_eln::sca_c *>            c;
        std::map<string, sca_eln::sca_tdf::sca_vsource *>   vSo;
        std::map<string, sca_eln::sca_tdf::sca_vsink *>     vSi;



        ~ELN()
        {
            deInitMapList<string,sca_tdf::sca_in<dataType>*>(&I);
            deInitMapList<string,sca_tdf::sca_out<dataType>*>(&Q);
            deInitMapList<string,sca_eln::sca_node *>(&n);
            deInitMapList<string,sca_eln::sca_node_ref *>(&g);
            deInitMapList<string,sca_eln::sca_tdf::sca_vsource *>(&vSo);
            deInitMapList<string,sca_eln::sca_tdf::sca_vsink *>(&vSi);
        };

        ELN(sc_core::sc_module_name nm):sc_core::sc_module(nm)
        {
            I["I"]              = new sca_tdf::sca_in<dataType>;
            Q["Q"]              = new sca_tdf::sca_out<dataType>;
        };
        /*
        template<class nodeType>
        void createNode(string name)
        {
            eln.g[name]         = new sca_eln::sca_node_ref;
        }
    eln.n["n1"]             = new sca_eln::sca_node;
    eln.n["n2"]             = new sca_eln::sca_node;

     
    eln.vSo["Source"]       = new sca_eln::sca_tdf::sca_vsource("Source");
    eln.vSi["Sink"]         = new sca_eln::sca_tdf::sca_vsink("Sink");

    eln.r["r1"]             = new sca_eln::sca_r("r1",1.0);
    eln.c["c1"]             = new sca_eln::sca_c("c1",1.0,0);*/


};

#endif
