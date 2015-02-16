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

  sca_matrix_base.h - description

  Original Author: Karsten Einwich Fraunhofer IIS/EAS Dresden

  Created on: 09.10.2009

   SVN Version       :  $Revision: 1107 $
   SVN last checkin  :  $Date: 2011-01-09 12:20:46 +0100 (Sun, 09 Jan 2011) $
   SVN checkin by    :  $Author: karsten $
   SVN Id            :  $Id: sca_matrix_base.h 1107 2011-01-09 11:20:46Z karsten $

 *****************************************************************************/

/*****************************************************************************/

#ifndef SCA_MATRIX_BASE_H_
#define SCA_MATRIX_BASE_H_

namespace sca_core
{
namespace sca_implementation
{
class sca_linear_solver;
class sca_conservative_cluster;
}
}


namespace sca_tdf
{
namespace sca_implementation
{
class sca_ct_ltf_nd_proxy;
class sca_ct_ltf_zp_proxy;
class sca_ct_vector_ss_proxy;
}
}


namespace sca_eln
{
namespace sca_implementation
{
class sca_eln_view;
class lin_eqs_cluster;
}
}

namespace sca_lsf
{
namespace sca_implementation
{
class sca_lsf_view;
class lin_eqs_cluster;
}
}



namespace sca_ac_analysis
{
namespace sca_implementation
{
class sca_ac_domain_entity;
class sca_ac_domain_eq;
class sca_ac_domain_solver;
class solve_linear_complex_eq_system;
}
}


namespace sca_util
{
namespace sca_implementation
{

template<class T>
class sca_function_vector;

template<class T>
class sca_matrix_base;

template<class T>
std::ostream& operator<<  ( std::ostream& os,const sca_matrix_base<T>&);

template<class T>
class sca_matrix_base
{

	friend class sca_tdf::sca_implementation::sca_ct_ltf_nd_proxy;
	friend class sca_tdf::sca_implementation::sca_ct_ltf_zp_proxy;
	friend class sca_tdf::sca_implementation::sca_ct_vector_ss_proxy;

    unsigned long  sizes[2];    // -> dim=1 (vector) and dim=2 supported
    bool           auto_dim, auto_sizable;
    bool           ignore_negative;
    T              dummy;


protected:

    bool                square;
    long                last_val;
    unsigned long       dim;      //currently dimension is set to 2 or 1
    std::valarray<T>    matrix;
    bool                accessed;

protected:

    void reset();
    void remove();

    sca_matrix_base();                                  //default matrix
    sca_matrix_base(unsigned long x);                   //vector not resizable
    sca_matrix_base(unsigned long y, unsigned long x);  //matrix not resizable
    sca_matrix_base(const sca_matrix_base<T>& m);       //copied matrix

    virtual ~sca_matrix_base();

    sca_matrix_base<T>& operator= (const sca_matrix_base<T>& m);

    unsigned long dimx() const;
    unsigned long dimy() const;
    void set_autodim();
    void reset_autodim();
    bool get_autodim() const;
    void set_sizable();
    void reset_sizable();
    T* get_flat();
    void reset_ignore_negative();
    void set_ignore_negative();

    void reset_access_flag();
    bool get_access_flag() const;


    T& operator[] (unsigned long x);
    const T& operator[] (unsigned long x) const;


    T& operator() (long y, long x);
    const T& operator() (long y, long x) const;
    T& operator() (long xi);  //vector
    const T& operator() (long xi) const;  //vector

    void resize(unsigned long xy);

    void resize(unsigned long ys, unsigned long xs);

    friend std::ostream& operator<<  <T>( std::ostream& os,const sca_matrix_base<T>&);

    friend class sca_core::sca_implementation::sca_linear_solver;
    friend class sca_core::sca_implementation::sca_conservative_cluster;
    friend class sca_util::sca_implementation::sca_function_vector<T>;
    friend class sca_eln::sca_implementation::sca_eln_view;
    friend class sca_lsf::sca_implementation::sca_lsf_view;
    friend class sca_lsf::sca_implementation::lin_eqs_cluster;
    friend class sca_eln::sca_implementation::lin_eqs_cluster;
    friend class sca_ac_analysis::sca_implementation::sca_ac_domain_entity;
    friend class sca_ac_analysis::sca_implementation::sca_ac_domain_eq;
    friend class sca_ac_analysis::sca_implementation::sca_ac_domain_solver;
    friend class sca_ac_analysis::sca_implementation::solve_linear_complex_eq_system;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

template<class T>
inline void sca_matrix_base<T>::reset()
{
    matrix.resize(matrix.size());
}


template<class T>
inline void sca_matrix_base<T>::remove()
{
    if(matrix.size())
    {
        matrix.resize(0);
        for(unsigned long i=0;i<dim;i++)
        {
            sizes[i] =0;
        }
    }
}

////////

template<class T>
inline sca_matrix_base<T>::sca_matrix_base()       //default matrix
{
    dim      = 0;
    sizes[0] = 0;
    sizes[1] = 0;
    square   = 0;
    auto_dim = 1;
    auto_sizable=0;
    last_val = -1;
    accessed = false;
    ignore_negative=1;
}


template<class T>
inline sca_matrix_base<T>::sca_matrix_base(unsigned long x)       //vector not resizable
{
    dim      = 1;
    sizes[0] = 0;
    sizes[1] = 0;
    square   = 0;
    auto_dim = 0;
    auto_sizable=0;
    last_val = x-1;
    resize(1,x);
    ignore_negative=1;
}



template<class T>
inline sca_matrix_base<T>::sca_matrix_base(unsigned long y, unsigned long x)       //matrix not resizable
{
    dim      = 2;
    sizes[0] = 0;
    sizes[1] = 0;
    square   = 0;
    auto_dim = 0;
    resize(y,x);
    ignore_negative=1;
}

template<class T>
inline sca_matrix_base<T>::~sca_matrix_base()
{
}

template<class T>
inline sca_matrix_base<T>::sca_matrix_base(const sca_matrix_base<T>& m) : matrix(m.matrix)     //copied matrix
{
    dim      = m.dim;
    sizes[0] = m.sizes[0];
    sizes[1] = m.sizes[1];
    square   = m.square;
    auto_dim = m.auto_dim;
    auto_sizable=m.auto_sizable;
    last_val = m.last_val;
    accessed = true;
    ignore_negative=m.ignore_negative;
}


template<class T>
inline sca_matrix_base<T>& sca_matrix_base<T>::operator= (const sca_matrix_base<T>& m)
{
    dim      = m.dim;
    sizes[0] = m.sizes[0];
    sizes[1] = m.sizes[1];
    square   = m.square;
    auto_dim = m.auto_dim;
    accessed = true;
    ignore_negative=m.ignore_negative;
    matrix.resize(m.matrix.size());
    matrix = m.matrix;
    return *this;
}


template<class T>
inline unsigned long sca_matrix_base<T>::dimx() const
{
	return(sizes[0]);
}

template<class T>
inline unsigned long sca_matrix_base<T>::dimy() const
{
	return(sizes[1]);
}

template<class T>
inline void sca_matrix_base<T>::set_autodim()
{
	auto_dim=1;
}


template<class T>
inline void sca_matrix_base<T>::reset_autodim()
{
	auto_dim=0;
}

template<class T>
inline bool sca_matrix_base<T>::get_autodim() const
{
	return auto_dim;
}


template<class T>
inline void sca_matrix_base<T>::set_sizable()
{
	auto_sizable=1;
}


template<class T>
inline void sca_matrix_base<T>::reset_sizable()
{
	auto_sizable=0;
}


template<class T>
inline T* sca_matrix_base<T>::get_flat()
{
	return &matrix[0];
}


template<class T>
inline void sca_matrix_base<T>::reset_ignore_negative()
{
	ignore_negative=0;
}


template<class T>
inline void sca_matrix_base<T>::set_ignore_negative()
{
	ignore_negative=1;
}

template<class T>
inline void sca_matrix_base<T>::reset_access_flag()
{
	accessed=false;
}


template<class T>
inline bool sca_matrix_base<T>::get_access_flag() const
{
	return accessed;
}


template<class T>
inline T& sca_matrix_base<T>::operator[] (unsigned long x)
{
	accessed=true;
	return matrix[x];
}

template<class T>
inline const T& sca_matrix_base<T>::operator[] (unsigned long x) const
{
    //we must use the operator T& valarray::operator[](size_t) instead
    // T valarray::operator[](size_t) const
    return( const_cast<std::valarray<T>*>(&matrix)->operator[] (x) );
}


template<class T>
inline T& sca_matrix_base<T>::operator() (long y, long x) //matrix
{
    bool flag=false;
    unsigned long xr, yr;

    accessed=true;

    if((x<0)||(y<0))
    {
        if(ignore_negative)
        {
            memset(&dummy, 0, sizeof(T));
            return(dummy);
        }
        else
        {
            SC_REPORT_ERROR("SystemC-AMS", "Matrix access with negative indices is not allowed.\n");
        }
    }

    if(dim==0) dim=2;
    if((dim!=2)&&(y>0))  //exception handling has to be included
    {
        SC_REPORT_ERROR("SystemC-AMS", "Wrong matrix access (internal error).\n");
    }

    if((unsigned long)(x)>=sizes[0])
    {
        flag=1;
        xr  =x+1;
    }
    else
    {
        xr=sizes[0];
    }


    if((unsigned long)(y)>=sizes[1])
    {
        flag=1;
        yr  =y+1;
    }
    else
    {
        yr=sizes[1];
    }

    if(square)
    {
        if(yr>xr) xr=yr;
        else yr=xr;
    }

    if(flag)
    {
        if(auto_dim)
        {
        	resize(yr,xr);
            //cout << "resize matrix " << xr << " " << yr << endl;

        }
        else  //exception handling has to be included  !!!!!!!
        {
            SC_REPORT_ERROR("SystemC-AMS", "Not allowed matrix access (index is out of bounds).\n");
        }
    }

    return( matrix[x*sizes[1]+y] );
}




template<class T>
inline const T& sca_matrix_base<T>::operator() (long y, long x) const //matrix
{
    bool flag=false;
    unsigned long xr, yr;

    if((x<0)||(y<0))
    {
        if(ignore_negative)
        {
            memset(const_cast<T*>(&dummy), 0, sizeof(T));
            return(dummy);
        }
        else
        {
            SC_REPORT_ERROR("SystemC-AMS", "Matrix access with negative indices is not allowed.\n");
        }
    }

    if(dim==0) *const_cast<unsigned long*>(&dim)=2;
    if((dim!=2)&&(y>0))  //exception handling has to be included
    {
        SC_REPORT_ERROR("SystemC-AMS", "Wrong matrix access (internal error).\n");
    }

    if((unsigned long)(x)>=sizes[0])
    {
        flag=1;
        xr  =x+1;
    }
    else
    {
        xr=sizes[0];
    }


    if((unsigned long)(y)>=sizes[1])
    {
        flag=1;
        yr  =y+1;
    }
    else
    {
        yr=sizes[1];
    }

    if(square)
    {
        if(yr>xr) xr=yr;
        else yr=xr;
    }

    if(flag)
    {
        if(auto_dim)
        {
            const_cast<sca_matrix_base<T>*>(this)->resize(yr,xr);

        }
        else  //exception handling has to be included  !!!!!!!
        {
            SC_REPORT_ERROR("SystemC-AMS", "Not allowed matrix access (index is out of bounds).\n");
        }
    }

    //we must use the operator T& valarray::operator[](size_t) instead
    // T valarray::operator[](size_t) const
    return( const_cast<std::valarray<T>&>(matrix)[x*sizes[1]+y] );
}




template<class T>
inline T& sca_matrix_base<T>::operator() (long xi)  //vector
{
    accessed=true;

    if(xi<0)
    {
        if(ignore_negative)
        {
        	memset(&dummy, 0, sizeof(T));
            return(dummy);
        }
        else
        {
            SC_REPORT_ERROR("SystemC-AMS", "Matrix access with negative indices is not allowed.\n");
        }
    }

    unsigned long x=(unsigned long) xi;
    unsigned long xr;

    if(dim==0) dim=1;


    if(dim!=1) //exception handling has to be included
    {
        SC_REPORT_ERROR("SystemC-AMS", "Wrong vector access (internal error).\n");
    }

    if(x>=sizes[0])
    {
        if(auto_dim)
        {
            if(auto_sizable)
                for(xr=(sizes[0])? sizes[0]:(x+1); xr<=x; xr*=2);
            else xr=x+1;

            resize(1,xr);

        }
        else  //exception handling has to be included  !!!!!!!
        {
            SC_REPORT_ERROR("SystemC-AMS", "Not allowed vector access (index is out of bounds).\n");
        }
    }

    if(xi>last_val) last_val=xi;

    return( matrix[x] );
}


template<class T>
inline const T& sca_matrix_base<T>::operator() (long xi)  const //vector
{

    if(xi<0)
    {
        if(ignore_negative)
        {
        	memset(const_cast<T*>(&dummy), 0, sizeof(T));
            return(dummy);
        }
        else
        {
            SC_REPORT_ERROR("SystemC-AMS", "Matrix access with negative indices is not allowed.\n");
        }
    }

    unsigned long x=(unsigned long) xi;
    unsigned long xr;

    if(dim==0) *(const_cast<unsigned long*>(&dim))=1;


    if(dim!=1) //exception handling has to be included
    {
        SC_REPORT_ERROR("SystemC-AMS", "Wrong vector access (internal error).\n");
    }

    if(x>=sizes[0])
    {
        if(auto_dim)
        {
            if(auto_sizable)
                for(xr=(sizes[0])? sizes[0]:(x+1); xr<=x; xr*=2);
            else xr=x+1;

            const_cast<sca_matrix_base<T>*>(this)->resize(1,xr);

        }
        else  //exception handling has to be included  !!!!!!!
        {
            SC_REPORT_ERROR("SystemC-AMS", "Not allowed vector access (index is out of bounds).\n");
        }
    }

    if(xi>last_val) *const_cast<long*>(&last_val)=xi;

    return( const_cast<std::valarray<T>&>(matrix)[x] );
}




template<class T>
inline void sca_matrix_base<T>::resize(unsigned long xy)
{
    if(square) resize(xy,xy);
    else
    {
        resize(1,xy);
        if((long)xy<=last_val) last_val=xy-1;
    }
}



template<class T>
inline void sca_matrix_base<T>::resize(unsigned long ys, unsigned long xs)
{

    if(!matrix.size())
    {
        matrix.resize((size_t)(xs*ys));
    }

    else
    {
        std::valarray<size_t> blocklengths(2), subsizes(2);

        blocklengths[0]=(size_t)sizes[1];
        blocklengths[1]=1;
        subsizes[0]=(size_t)((sizes[0]>xs)? xs : sizes[0]);
        subsizes[1]=(size_t)((sizes[1]>ys)? ys : sizes[1]);

        std::gslice dim_b(0, subsizes, blocklengths);

        //this is not working for gcc 2.96 on Red Hat
        //valarray<T>     backup (matrix[dim_b]);
        std::valarray<T>     backup (subsizes[0]*subsizes[1]);
        backup=matrix[dim_b];

        matrix.resize((size_t)(xs*ys));

        blocklengths[0]=(size_t)ys;
        std::gslice dim_c(0, subsizes, blocklengths);
        matrix[dim_c] = backup;
    }

    sizes[0]=xs;
    sizes[1]=ys;

    accessed=true;
}



template<class T>
inline std::ostream& operator<< ( std::ostream& os,const sca_matrix_base<T>& matrix)
{
    for(unsigned long iy=0;iy<matrix.dimy();iy++)
    {
        for(unsigned long ix=0;ix<matrix.dimx();ix++)
        {
            os << std::setw(20) << matrix(iy,ix) << " ";
        }
        os << std::endl;
    }
    return(os);
}





}
}


#endif /* SCA_MATRIX_BASE_H_ */
