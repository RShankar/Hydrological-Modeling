#ifndef _MODEL_H
#define _MODEL_H

SCA_TDF_MODULE(model)
{
    sca_tdf::sca_out<double>  out;

    ~model()
    {
        cout<<"Dealloc"<<endl;
    }

    model( sc_core::sc_module_name nm, 
               sca_core::sca_time Tm_ = sca_core::sca_time(0.125, sc_core::SC_MS))
	{
        
      Tm = Tm_;
    }
  
    void initialize()
    {

    }

	void set_attributes()
	{
		set_timestep(Tm);
     

	}
 
	void processing()
	{
        
		double t = get_time().to_seconds(); // actual time

        //cout<<"t "<<t<<endl;
        out.write(5);

	}
    
  	private:
		sca_core::sca_time Tm; // module time step



};


class rc : public sc_core::sc_module 
{
	public:
    
    sca_eln::sca_terminal storageOut; // node
    sca_eln::sca_terminal xIn;
    sca_eln::sca_terminal yIn;
    sca_eln::sca_terminal zIn;
    
    sca_eln::sca_terminal xOut; // node
    sca_eln::sca_terminal yOut; // node
    sca_eln::sca_terminal zOut; // node
    
    sca_eln::sca_tdf::sca_vsource *vxIn;
    sca_eln::sca_tdf::sca_vsource *vyIn;
    sca_eln::sca_tdf::sca_vsource *vzIn;
    sca_eln::sca_tdf::sca_vsink   *vStorageOut;


    sca_eln::sca_r rx1;
    sca_eln::sca_r rx2;
    
    sca_eln::sca_r ry1;
    sca_eln::sca_r ry2;

    sca_eln::sca_r rz1;
    sca_eln::sca_r rz2;

    sca_eln::sca_c c1;
   
    sca_eln::sca_node_ref gnd; 
    


    RcModelSettings set;


    void gndX()
    {
        rx2.n(gnd);

    }
    
    void gndY()
    {
        ry2.n(gnd);

    }

    void gndZ()
    {
        ry2.n(gnd);
    }


    rc(sc_core::sc_module_name nm, 
        RcModelSettings set):sc_core::sc_module(nm),
        rx1("rx1",set.Kx),
        rx2("rx2",set.Kx),
        ry1("ry1",set.Ky),
        ry2("ry2",set.Ky),
        rz1("rz1",set.Kz),
        rz2("rz2",set.Kz),
        c1("C", set.S, set.initHead)

    {
        rx1.p(xIn);
        rx1.n(storageOut);
    
        rx2.p(storageOut);
        rx2.n(xOut);
        
        ry1.p(yIn); 
        ry1.n(storageOut);
     
        ry2.p(storageOut);
        ry2.n(yOut);

        rz1.p(zIn);
        rz1.n(storageOut);
        
        rz2.p(storageOut); 
        rz2.n(zOut);

        
        c1.p(storageOut); 
        c1.n(gnd);
        

    }
    
   
};
    
class Rc3dMoC: public sc_core::sc_module
{

    public:
        
        sca_eln::sca_r *rxIn;
        sca_eln::sca_r *rxOut;
        
        sca_eln::sca_r *ryIn;
        sca_eln::sca_r *ryOut;
        
        sca_eln::sca_r *rzIn;
        sca_eln::sca_r *rzOut;

		sca_eln::sca_c *c;

        sca_eln::sca_node o;

        sca_eln::sca_node s;

        sca_eln::sca_node_ref gnd;

        RcModelSettings setttings;

        Rc3dMoC(
            sc_core::sc_module_name nm,
            RcModelSettings set,
            sca_util::sca_trace_file* af)
        {   
            char name[100];
            
            setttings = set;

            sprintf_s(name,"rxIn_%s",nm);
            rxIn = new sca_eln::sca_r(name,set.Kx);
            sca_util::sca_trace(af, *rxIn, name);

            sprintf_s(name,"rxOut_%s",nm);
            rxOut = new sca_eln::sca_r(name,set.Kx);
            sca_util::sca_trace(af, *rxOut, name);
            
            sprintf_s(name,"ryIn_%s",nm);
            ryIn = new sca_eln::sca_r(name,set.Ky);
            sca_util::sca_trace(af, *ryIn, name);

            sprintf_s(name,"ryOut_%s",nm);
            ryOut = new sca_eln::sca_r(name,set.Ky);
            sca_util::sca_trace(af, *ryOut, name);
            
            
            sprintf_s(name,"rzIn_%s",nm);
            
            rzIn = new sca_eln::sca_r(name,set.Kz/2);
            sca_util::sca_trace(af, *rzIn, name);

            sprintf_s(name,"rzOut_%s",nm);
            rzOut = new sca_eln::sca_r(name,set.Kz/2);
            sca_util::sca_trace(af, *rzOut, name);

            sprintf_s(name,"c_%s",nm);
   
            c = new sca_eln::sca_c(name,set.S,set.initHead);

            //sca_util::sca_trace(af, *c, name);

            rxIn->n((set.Kx != -1 )?o:s);
            rxOut->p((set.Kx != -1 )?o:s);

            ryIn->n((set.Ky != -1 )?o:s);
            ryOut->p((set.Ky != -1 )?o:s);

            rzIn->n((set.Kz != -1 )?o:s);
            rzOut->p((set.Kz != -1 )?o:s);

            c->p(o);
        }

        ~Rc3dMoC()
        {
            delete rxIn;
            delete rxOut;
            delete ryIn;
            delete ryOut;
  
            delete rzIn;
            delete rzOut;

            delete c;
        }

    private:

};

class gridEln : public sc_core::sc_module 
{
	public:

        sca_tdf::sca_in<double> *xIn; // output port
        sca_tdf::sca_in<double> *yIn; // output port
        sca_tdf::sca_in<double> *zIn; // output port
        sca_tdf::sca_out<double> *storageOut; // output port


		sca_eln::sca_node_ref gnd; 

 
        vector< vector< vector<Rc3dMoC *> > > grid;
        
        sca_eln::sca_node *Q ;
        sca_eln::sca_node *x;
        sca_eln::sca_node *y;
        sca_eln::sca_node *z;
        sca_eln::sca_node *storage; 
        sca_eln::sca_node *open;

        

        char name[100];

        void allocateNodes(FlowSetting set)
        {
            

            cout<<set.xDir<<" "<<set.yDir<<" "<<set.zDir<<endl;

            unsigned int gridSize = (set.xMax * set.yMax * set.zMax);

            Q = new sca_eln::sca_node[3*gridSize];
            open = new sca_eln::sca_node[3*gridSize];

            x = new sca_eln::sca_node[set.xDir];
            y = new sca_eln::sca_node[set.yDir];
            z = new sca_eln::sca_node[set.zDir];
            storage = new sca_eln::sca_node[gridSize]; 
        
        }
        
        void allocateGrid( FlowSetting set, int (*initFunc)(unsigned int * , double *))
        {
        
            Rc3dMoC *cell = NULL;
            vector< vector< vector<Rc3dMoC *> > > grid1(set.xMax, vector< vector<Rc3dMoC *> > (set.yMax, vector<Rc3dMoC *>(set.zMax,cell)) );
            grid = grid1;
            
            unsigned int kk=0;
            unsigned int jj=0;
            unsigned int ii=0;
            double params[5]={0};
            RcModelSettings rcSet;

            int l = 0;

            sca_util::sca_trace_file* af = sca_util::sca_create_tabular_trace_file("c:\\fw\\ac1.dat");


            for( kk=0; kk < set.zMax; kk++)
            {

                for( jj=0; jj < set.yMax; jj++)
                {
                    for( ii=0; ii < set.xMax; ii++)
                    {       
                        unsigned int coord[3] ={ii, jj, kk};

                        initFunc(coord, params);

                        sprintf_s(name,"cell[%d,%d,%d]",ii,jj,kk);
                      


                        rcSet.Kx        =  params[0];
                        rcSet.Ky        =  params[1];
                        rcSet.Kz        =  params[2];
                        rcSet.S         =  params[3];
                        rcSet.initHead  =  params[4];

                        rcSet.maxRow = set.xMax;
                        rcSet.maxCol = set.yMax;
                        rcSet.maxLayer = set.zMax;

                        rcSet.i = ii;
                        rcSet.j = jj;
                        rcSet.k = kk;
  
                        cout<<name<<endl;

                        cell = new Rc3dMoC(name,rcSet,af);

                        grid[ii][jj][kk] = cell;
                    }
                }
            }
        }
        
        
        void connectGrid( FlowSetting set)
        {
         
            unsigned int kk=0;
            unsigned int jj=0;
            unsigned int ii=0;
            unsigned int qn = 0;

            unsigned int sn = 0;

            unsigned int nextXNode, nextYNode, nextZNode;

            for( kk=0; kk < set.zMax; kk++)
            {
                for( jj=0; jj < set.yMax; jj++)
                {
                    for( ii=0; ii < set.xMax; ii++)
                    {
 
                        nextXNode = (ii < set.xMax) ? ( ii + 1) %set.xMax : set.xMax ;
                        nextYNode = (jj < set.yMax) ? ( jj + 1) %set.yMax : set.yMax;
                        nextZNode = (kk < set.zMax) ? ( kk + 1) %set.zMax : set.zMax;
                        
                        
                        cout<<"nextXNode="<<nextXNode<<endl;

                        
                        
                        if(nextXNode) 
                        {
                            grid[ii][jj][kk]->rxOut->n(Q[qn]);
                            grid[nextXNode][jj][kk]->rxIn->p(Q[qn]);
                        }
                        else
                        {
                             grid[ii][jj][kk]->rxOut->n(gnd);
                        }

                       
                        
                        if(nextYNode) 
                        {
                            grid[ii][jj][kk]->ryOut->n(Q[qn + 1]);
                            grid[ii][nextYNode][kk]->ryIn->p(Q[qn + 1]);
                        }
                        else
                        {
                             grid[ii][jj][kk]->ryOut->n(gnd);
                        }

                        
                        
                        if(nextZNode) 
                        {
                            
                            grid[ii][jj][kk]->rzOut->n(Q[qn + 2]);
                            grid[ii][jj][nextZNode]->rzIn->p(Q[qn + 2]);
                        
                        }
                        else
                        {
                            grid[ii][jj][kk]->rzOut->n(gnd);
                        }

                        grid[ii][jj][kk]->c->n(gnd);

                        sca_eln::sca_tdf::sca_vsink *vStorageOut;

                        sprintf_s(name,"sto[%d]",sn);
                        vStorageOut = new sca_eln::sca_tdf::sca_vsink(name);
                        vStorageOut->outp(storageOut[sn]);

                        vStorageOut->p(grid[ii][jj][kk]->o);
                        vStorageOut->n(gnd);

                        sn++;

                        qn+=3;


                    }
                }
            }
        
        }

        void connectInputOutput( FlowSetting set)
        {
         
            unsigned int kk=0;
            unsigned int jj=0;
            unsigned int ii=0;
            unsigned int sn = 0;
            unsigned int xn = 0;
            unsigned int yn = 0;
            unsigned int zn = 0;


            char name[100];



            for( xn = 0; xn < set.xDir ; xn++)
            {
                sprintf_s(name,"xin[%d]",xn);
                sca_eln::sca_tdf::sca_vsource *vxIn = new sca_eln::sca_tdf::sca_vsource(name);
                vxIn->inp(xIn[xn]);
                vxIn->p(x[xn]);
                vxIn->n(gnd);
            }

            for( yn = 0; yn < set.yDir ; yn++) 
            {
                sprintf_s(name,"yin[%d]",yn);
                sca_eln::sca_tdf::sca_vsource *vyIn = new sca_eln::sca_tdf::sca_vsource(name);
                vyIn->inp(yIn[yn]);
                vyIn->p(y[yn]);
                vyIn->n(gnd);
            }
 
            for( zn = 0; zn < set.zDir ; zn++) 
            { 

                sprintf_s(name,"zin[%d]",zn);
                sca_eln::sca_tdf::sca_vsource *vzIn = new sca_eln::sca_tdf::sca_vsource(name);
                vzIn->inp(zIn[zn]);
                vzIn->p(z[zn]);
                vzIn->n(gnd);
            }

            xn = 0;
            yn = 0;
            zn = 0;

            for( kk=0; kk < set.zMax; kk++)
            {
                for( jj=0; jj < set.yMax; jj++)
                { 
                    grid[0][jj][kk]->rxIn->p(x[xn]);
                    cout<<0<<" "<<jj<<" "<<kk<<endl;
                    xn++; 
                }
            }
        
            for( kk=0; kk < set.zMax; kk++)
            {
                for( ii=0; ii < set.xMax; ii++)
                {
                    
                    grid[ii][0][kk]->ryIn->p(y[yn]);
                    yn++; 
                }
            }

            for( jj=0; jj < set.yMax; jj++)
            {
                for( ii=0; ii < set.xMax; ii++)
                {
                    grid[ii][jj][0]->rzIn->p(z[zn]);
                    zn++; 
                }
            }

        }

        gridEln( sc_core::sc_module_name nm, FlowSetting set,int (*initFunc)(unsigned int * , double *) = NULL )
        {
        
            unsigned int gridSize = (set.xMax * set.yMax * set.zMax);
  
            xIn = new sca_tdf::sca_in<double>[set.xDir];
            yIn = new sca_tdf::sca_in<double>[set.yDir];
            zIn = new sca_tdf::sca_in<double>[set.zDir];
            storageOut  = new sca_tdf::sca_out<double>[gridSize];

            allocateNodes(set);
            allocateGrid(set, initFunc);
            connectGrid(set);
            connectInputOutput(set);



        }

};





#define M (4)

class rc_2d : public sc_core::sc_module 
{
	public:
		sca_tdf::sca_in <double > in;  
		
		sca_eln::sca_node xNode[3*M];
        sca_eln::sca_node yNode[3*M];
        sca_eln::sca_node zNode[3*M];

        sca_eln::sca_node openNode[3*M];
		sca_eln::sca_node_ref gnd; 

		sca_eln::sca_tdf::sca_vsource vin; 
		
        sca_eln::sca_r *r;
		sca_eln::sca_c *c;

		rc_2d(sc_core::sc_module_name nm):
        sc_core::sc_module(nm),
        in("in"),
        gnd("gnd"),
		vin("vin")
		{

            sca_util::sca_trace_file* af = sca_util::sca_create_tabular_trace_file("c:\\fw\\ac.dat");

			vin.inp(in);
			vin.p(xNode[0]); 
            vin.n(gnd);
    
            RcModelSettings set;

            set.Kx = 2.0;

            set.Ky = -1;

            set.Kz = -1;

            set.initHead = 0;

            set.S = 0.0;


            char name[100];

            int i =0;

            Rc3dMoC *elem;

            for( i = 0 ; i < M ; i++)
            {
                sprintf_s(name,"l%d",i);
                
                elem = new Rc3dMoC(name,set,af);
                
                elem->rxIn->p((set.Kx!=-1)?xNode[i]:openNode[i]);    
                elem->ryIn->p((set.Ky!=-1)?yNode[i]:openNode[i]);
                elem->rzIn->p((set.Kz!=-1)?zNode[i]:openNode[i]);

                elem->rxOut->n((i!=(M-1) || (set.Kx ==-1))?xNode[i+1]:gnd);
                elem->ryOut->n((i!=(M-1) || (set.Ky ==-1) )?yNode[i+1]:gnd);
                elem->rzOut->n((i!=(M-1) || (set.Kz ==-1) )?zNode[i+1]:gnd);

                elem->c->n(gnd);
 

            }
           

             
		}
};
/*
class rc_2d : public sc_core::sc_module 
{
	public:
		sca_tdf::sca_in <double > in;  
		
		sca_eln::sca_node n1, n2; 
		sca_eln::sca_node_ref gnd; 

		sca_eln::sca_tdf::sca_vsource vin; 
		sca_eln::sca_r r1;
        sca_eln::sca_r r2;
		sca_eln::sca_c c1;


		rc_2d(sc_core::sc_module_name nm):
        in("in"),
        n1("n1"), 
        n2("n2"), 
        gnd("gnd"),
		vin("vin"),
        r1("r1",1.0),
        r2("r2",1.0),
        c1("c1",1.0,0.0)
		{

            sca_util::sca_trace_file* af = sca_util::sca_create_tabular_trace_file("c:\\fw\\ac.dat");

			vin.inp(in);
			vin.p(n1); 
            vin.n(gnd);
   
            r1.p(n1);
            r1.n(n2);
            
            c1.p(n2);
            c1.n(gnd);
            
            r2.p(n2);
            r2.n(gnd);
	
            sca_util::sca_trace(af, r1, "r1");
            sca_util::sca_trace(af, r2, "r2");
             
		}
};*/

#endif
