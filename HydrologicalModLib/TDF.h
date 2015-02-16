#ifndef TDF_MODEL_H
#define TDF_MODEL_H

template<class dataType>
SCA_TDF_MODULE(TDF)
{
	public:

        typedef int (*MoC)(double t, dataType *, dataType *);

        std::map<string, sca_tdf::sca_in<dataType>  *>  I;
        std::map<string, sca_tdf::sca_out<dataType> *>  Q;

        ~TDF()
        {
            deInitMapList<string,sca_tdf::sca_in<dataType>*>(&I);
            deInitMapList<string,sca_tdf::sca_out<dataType>*>(&Q);
        };

        TDF(sc_core::sc_module_name nm,
            MoC _moc,
            sca_core::sca_time Tm_ = sca_core::sca_time(1.0, sc_core::SC_SEC))
        {
 
            I["I"]              = new sca_tdf::sca_in<dataType>;
            Q["Q"]              = new sca_tdf::sca_out<dataType>;
            IBuffer             = new dataType;
            QBuffer             = new dataType;
            Tm = Tm_;
            moc = _moc;
        };
      
    void initialize()
    {
       dataType t = get_time().to_seconds(); // actual time
        
    }

	void set_attributes()
	{
		set_timestep(Tm);
	}

	void processing()
	{
        double t = get_time().to_seconds(); // actual time

        IBuffer[0] = (*I["I"]).read();

        if( moc != NULL && 
            moc(t, IBuffer, QBuffer) )
        {
            (*Q["Q"]).write(QBuffer[0]);
        } 

	}

	private:
		sca_core::sca_time Tm; // module time step
        MoC moc;
        dataType *IBuffer;
        dataType *QBuffer;

};

#endif
