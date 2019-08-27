/*
 
Copyright (C) 2007 Lucas K. Wagner

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 
*/

#ifndef MO_MATRIX_H_INCLUDED
#define MO_MATRIX_H_INCLUDED

#include "Array.h"
#include "Qmc_std.h"
#include "Basis_function.h"
#include "Center_set.h"
#include <algorithm> 

class System;
class Sample_point;

class General_MO_matrix {
 public:
  /*!
    Build several sets of MO's to be evaluated in updateVal and updateLap.
    Each element in occupations should be a list of the MO's that should
    be evaluated.  For example, one can create a list of spin up and spin
    down MO's, and only evaluate up when an up electron is moved.
   */
  virtual void buildLists(Array1 <Array1 <int> > & occupations)=0;
  virtual void buildLists(Array1 <int> & occupations) {
      error("Must pass occupations as Array1<Array1<int> > to indicate different spins.\n Only needed for spinor orbitals"); 
  }

  /*!
    get the number of molecular orbitals
   */
  virtual int getNmo()=0;
  virtual int showinfo(ostream & os)=0;
  virtual int writeinput(string &, ostream &)=0;
  virtual void read(vector <string> & words, unsigned int & startpos, 
                    System * sys)=0;

  virtual ~General_MO_matrix()  {  }

};

//----------------------------------------------------------------------


template <class T> class Templated_MO_matrix: public General_MO_matrix {
protected:
  Center_set centers;
  Array1 <Basis_function *> basis; 
  int nmo;
  doublevar magnification_factor;
  string orbfile;
  int totbasis;
  int maxbasis;
  virtual void init() { } ;

  string oldsofile;

  Array1 <doublevar> kpoint; //!< the k-point of the orbitals in fractional units(1 0 0) is X, etc..
public:

  /*!
    Build several sets of MO's to be evaluated in updateVal and updateLap.
    Each element in occupations should be a list of the MO's that should
    be evaluated.  For example, one can create a list of spin up and spin
    down MO's, and only evaluate up when an up electron is moved.
   */
  virtual void buildLists(Array1 <Array1 <int> > & occupations)=0;

  virtual void setOrbfile(string & x) {
    orbfile=x;
  }
  /*!
    get the number of molecular orbitals
   */
  int getNmo() {
    return nmo;
  }

  virtual int showinfo(ostream & os)=0;

  virtual int writeinput(string &, ostream &)=0;
  virtual void read(vector <string> & words, unsigned int & startpos, 
                    System * sys);

  virtual void writeorb(ostream &, Array2 <doublevar> & rotation, Array1 <int> &)
  { error("writeorb not implemented"); } 

  virtual void updateVal(
    Sample_point * sample,
    int e,
    //!< electron number
    int listnum,
    Array2 <T> & newvals
    //!< The return: in form (MO)
  )=0;
  virtual void updateVal(
    Sample_point * sample,
    int e,
    //!< electron number
    Array2 <T> & newvals
    //!< The return: in form (MO)
  )
  { error("Must pass spin index. This is only used for spinor orbitals"); }

  virtual void updateSpinorComponents(
                         Sample_point * sample,
                         int e,
                         //!< electron number
                         Array2 <dcomplex> & newvals
                         //!< The return: in form (MO)
                         )
  { error("updateSpinorComponents not implemented for this MO type"); }

  virtual void updateLap(
    Sample_point * sample,
    int e,
    //!< electron number
    int listnum,
    //!< Choose the list that was built in buildLists
    Array2 <T> & newvals
    //!< The return: in form (MO,[value gradient lap])
  )=0;

  virtual void updateLap(
    Sample_point * sample,
    int e,
    //!< electron number
    //!< Choose the list that was built in buildLists
    Array2 <T> & newvals
    //!< The return: in form (MO,[value gradient lap])
  )
  { error("Must pass spin index. This is only used for spinor orbitals"); }

  virtual void updateSpinLap(
    Sample_point * sample,
    int e,
    //!< electron number
    //!< Choose the list that was built in buildLists
    Array2 <T> & newvals
    //!< The return: in form (MO,[value gradient lap])
  )
  { error("updateSpinLap not implemented"); }

  virtual void updateHessian(Sample_point * sample,
			     int e,
			     int listnum,
			     Array2 <T>& newvals
			     //!< in form (MO, [value gradient, dxx,dyy,dzz,dxy,dxz,dyz])
			     ) { 
    error("this MO_matrix doesn't support Hessians");
  }
  virtual void updateHessian(Sample_point * sample,
			     int e,
			     Array2 <T>& newvals
			     //!< in form (MO, [value gradient, dxx,dyy,dzz,dxy,dxz,dyz])
			     ) { 
    error("this MO_matrix doesn't support Hessians");
  }

  Templated_MO_matrix()
  {}

  virtual ~Templated_MO_matrix()
  {
    //doublevar totcalls=0;
    //for(int i=0; i< nmo; i++) {
    //  cout << "mo_counter " << mo_counter(i)/n_calls << endl;
    //  totcalls+=mo_counter(i)/n_calls;
    //}
    //cout << " average # of basis functions evaluated " << totcalls/nmo << endl;
    for(int i=0; i< basis.GetDim(0); i++)
      deallocate(basis(i));
  }

};

typedef  Templated_MO_matrix<doublevar> MO_matrix;
typedef  Templated_MO_matrix<dcomplex> Complex_MO_matrix;
//----------------------------------------------------------------------------


int allocate(vector <string> & words, System * sys, MO_matrix *& moptr);
int allocate(vector <string> & words, System * sys, 
             Complex_MO_matrix *& moptr);

void rotate_orb(istream & orbin, ostream & orbout,
                Array2 <doublevar> & rotation,
                Array1 <int>  & moList, int nfunctions);
void rotate_Corb(istream & orbin, ostream & orbout,
		 Array2 <doublevar> & rotation,
		 Array1 <int>  & moList, int nfunctions);


//------------------------------------------------------------------------------------------
#ifdef USE_MPI
inline void overloaded_broadcast(Array1 <doublevar> & v) { 
  MPI_Bcast(v.v,v.GetDim(0), MPI_DOUBLE,0,MPI_Comm_grp);
}
inline void overloaded_broadcast(Array1 <dcomplex> & v) { 
  MPI_Bcast(v.v,v.GetDim(0), MPI_DOUBLE_COMPLEX,0,MPI_Comm_grp);
}
#endif

template <class T> int readorb(istream & input, Center_set & centers, 
                                  int nmo, int maxbasis, Array1 <doublevar> & kpoint,
                                  Array3 <int> & coeffmat, Array1 <T> & coeff) {
  int nmo_read=0;
  int maxlabel=0; 
  coeffmat.clear(); //important to do this so that we know exactly how big the array v will be
                    //This enables us to use relatively fast Bcast() operations.
  coeff.clear();
  if(mpi_info.node==0) { 
    string dummy;
    vector <int> mo,center,basis,label;
    while(true) { 
      input >> dummy;
      if(dummy=="COEFFICIENTS") break;
      int currmo=atoi(dummy.c_str())-1;
      if(currmo > nmo) { 
        input.ignore(300,'\n');
      }
      else { 
        mo.push_back(currmo);
        input >> dummy;
        basis.push_back(atoi(dummy.c_str())-1);
        if(basis.back() >= maxbasis) 
          error("Basis function greater than maxbasis requested:",basis.back()+1);
        else if(basis.back() < 0) 
          error("Basis function cannot be less than 1:",basis.back()+1);
        input >> dummy;
        center.push_back(atoi(dummy.c_str())-1);
        if(center.back() > centers.equiv_centers.GetDim(0) )
          error("Center number in orb file greater than maximum number ", 
                 centers.equiv_centers.GetDim(0));
        
        input >> dummy;
        label.push_back(atoi(dummy.c_str())-1);
      }
      if(!input) 
        error("Unexpected end of file; did not find COEFFICIENTS while reading orbitals");
    }
    nmo_read=*std::max_element(mo.begin(),mo.end())+1;
    coeffmat.Resize(nmo_read, centers.size(), maxbasis);
    coeffmat=-1;
    {
      vector<int>::iterator m=mo.begin(),
        c=center.begin(),
        b=basis.begin(),
        l=label.begin();

      for(  ; m!=mo.end() && c!=center.end() && b!=basis.end() && l!=label.end();
          m++,c++,b++,l++) { 
//        coeffmat(*m,*c,*b)=*l;
        for(int c_eq=0; c_eq < centers.ncenters_atom(*c); c_eq++) {
          int cen2=centers.equiv_centers(*c, c_eq);
          coeffmat(*m, cen2,*b)=*l;
        }
        
      }
    }

    maxlabel=*std::max_element(label.begin(),label.end())+1;
    coeff.Resize(maxlabel);
    for(int i=0; i< maxlabel; i++) { 
      if(! (input >> coeff(i) ) )
        error("unexpected end of file when reading orbital coefficients");
    }
  }
#ifdef USE_MPI
  MPI_Bcast(&nmo_read,1,MPI_INT,0,MPI_Comm_grp);
  MPI_Bcast(&maxlabel,1,MPI_INT,0,MPI_Comm_grp);
  int coeffmatsize;
  
  if(mpi_info.node!=0) { 
    coeffmat.Resize(nmo_read,centers.size(),maxbasis);
    coeff.Resize(maxlabel);
  }
  MPI_Bcast(coeffmat.v,coeffmat.size,MPI_INT,0,MPI_Comm_grp);
  overloaded_broadcast(coeff);
#endif
  return nmo_read;
}


template <class T> int readorb_noexpand(istream & input, Center_set & centers, 
                                  int nmo, int maxbasis, Array1 <doublevar> & kpoint,
                                  Array3 <int> & coeffmat, Array1 <T> & coeff) {
  int nmo_read=0;
  int maxlabel=0; 
  coeffmat.clear(); //important to do this so that we know exactly how big the array v will be
                    //This enables us to use relatively fast Bcast() operations.
  coeff.clear();
  if(mpi_info.node==0) { 
    string dummy;
    vector <int> mo,center,basis,label;
    while(true) { 
      input >> dummy;
      if(dummy=="COEFFICIENTS") break;
      int currmo=atoi(dummy.c_str())-1;
      if(currmo > nmo) { 
        input.ignore(300,'\n');
      }
      else { 
        mo.push_back(currmo);
        input >> dummy;
        basis.push_back(atoi(dummy.c_str())-1);
        if(basis.back() >= maxbasis) 
          error("Basis function greater than maxbasis requested:",basis.back()+1);
        else if(basis.back() < 0) 
          error("Basis function cannot be less than 1:",basis.back()+1);
        input >> dummy;
        center.push_back(atoi(dummy.c_str())-1);
        if(center.back() > centers.equiv_centers.GetDim(0) )
          error("Center number in orb file greater than maximum number ", 
                 centers.equiv_centers.GetDim(0));
        
        input >> dummy;
        label.push_back(atoi(dummy.c_str())-1);
      }
      if(!input) 
        error("Unexpected end of file; did not find COEFFICIENTS while reading orbitals");
    }
    nmo_read=*std::max_element(mo.begin(),mo.end())+1;
    coeffmat.Resize(nmo_read, centers.equiv_centers.GetDim(0), maxbasis);
    coeffmat=-1;
    {
      vector<int>::iterator m=mo.begin(),
        c=center.begin(),
        b=basis.begin(),
        l=label.begin();

      for(  ; m!=mo.end() && c!=center.end() && b!=basis.end() && l!=label.end();
          m++,c++,b++,l++) { 
        coeffmat(*m,*c,*b)=*l;
        
      }
    }

    maxlabel=*std::max_element(label.begin(),label.end())+1;
    coeff.Resize(maxlabel);
    for(int i=0; i< maxlabel; i++) { 
      if(! (input >> coeff(i) ) )
        error("unexpected end of file when reading orbital coefficients");
    }
  }
#ifdef USE_MPI
  MPI_Bcast(&nmo_read,1,MPI_INT,0,MPI_Comm_grp);
  MPI_Bcast(&maxlabel,1,MPI_INT,0,MPI_Comm_grp);
  int coeffmatsize;
  
  if(mpi_info.node!=0) { 
    coeffmat.Resize(nmo_read,centers.equiv_centers.GetDim(0),maxbasis);
    coeff.Resize(maxlabel);
  }
  MPI_Bcast(coeffmat.v,coeffmat.size,MPI_INT,0,MPI_Comm_grp);
  overloaded_broadcast(coeff);
#endif
  return nmo_read;
}

//----------------------------------------------------------------------
//A simple templated function to evaluate the k-point when it is real versus
//when it is complex. 
template <class T> inline T eval_kpoint_fac(doublevar & dot) {
  error("Not a general class.");
}

template <> inline doublevar eval_kpoint_fac<doublevar>(doublevar & dot) { 
  return cos(dot*pi);
}
template <> inline  dcomplex eval_kpoint_fac<dcomplex>(doublevar & dot) { 
  return exp(dcomplex(0.0,1.0)*dot*pi);
}

template <class T> inline void eval_kpoint_deriv(Array1 <doublevar> & kpoint,
    doublevar kr,
    T & val, Array1 <T> & grad, Array2 <T> & hess)  { 
  error("kpoint_deriv not implemented in general");
}

template <> inline void eval_kpoint_deriv<dcomplex>(Array1 <doublevar> & kpoint,
    doublevar kr,
    dcomplex & val, Array1 <dcomplex> & grad, Array2 <dcomplex> & hess)  { 
  int ndim=grad.GetDim(0);
  assert(ndim==hess.GetDim(0));
  assert(ndim==hess.GetDim(1));
  const dcomplex I(0,1.0);
  dcomplex eikr=eval_kpoint_fac<dcomplex>(kr);
  for(int d1=0; d1 < ndim; d1++) 
    for(int d2=0; d2< ndim; d2++) 
      hess(d1,d2)=eikr*(hess(d1,d2)
          +I*pi*kpoint(d1)*grad(d2)
          +I*pi*kpoint(d2)*grad(d1)
          -pi*pi*kpoint(d1)*kpoint(d2)*val);
 for (int d=0; d< ndim; d++) 
   grad(d)=eikr*(I*pi*kpoint(d)*val+grad(d));
 val*=eikr;
}

template <> inline void eval_kpoint_deriv<doublevar>(Array1 <doublevar> & kpoint,
    doublevar kr,
    doublevar & val, Array1 <doublevar> & grad, Array2 <doublevar> & hess)  { 
  //still not clear how to do this exactly, so we'll ignore it for now.
  assert(abs(kpoint(0)) < 1e-14);
  assert(abs(kpoint(1)) < 1e-14);
  assert(abs(kpoint(2)) < 1e-14);
}



//----------------------------------------------------------------------------
#include "qmc_io.h"

template<class T> inline void Templated_MO_matrix<T>::read(vector <string> & words,
                     unsigned int & startpos,
                     System * sys)
{


  unsigned int pos=startpos;

  if(!readvalue(words, pos, nmo, "NMO")) {
    error("Need NMO in molecular orbital section");
  }

  if(nmo > 40000) 
    error("You have entered more than 40,000 for NMO.  This seems a bit big; we most likely"
        " can't handle it.");


  pos=0;
  if(!readvalue(words, pos, magnification_factor, "MAGNIFY")) {
    magnification_factor=1;
  }


  //Basis functions
  vector <vector <string> > basistext;
  vector < string > basissec;
  pos=0;
  while( readsection(words, pos, basissec, "BASIS") != 0) {
    basistext.insert(basistext.end(), basissec);
  }
  basis.Resize(basistext.size());
  basis=NULL;

  if(basistext.size() == 0 )
    error("Didn't find a BASIS section");
  for(unsigned int i=0; i<basistext.size(); i++) {
    allocate(basistext[i], basis(i));
  }
  
  sys->kpoint(kpoint);
  //------------------------------Centers
  vector <string> centertext;
  pos=startpos;
  if(!readsection(words, pos, centertext, "CENTERS")) { 
    single_write(cout, "Defaulting to using the atoms as centers\n");
    string temp="USEATOMS";
    centertext.push_back(temp);
  }


  unsigned int newpos=0;
  centers.read(centertext, newpos, sys);
  centers.assignBasis(basis);

  //cout << "number of centers " << centers.size() << endl;
  totbasis=0;
  maxbasis=0;
  for(int i=0; i< centers.size(); i++)
  {
    int basiscent=0;
    for(int j=0; j< centers.nbasis(i); j++) {
      basiscent+=basis(centers.basis(i,j))->nfunc();
      //cout << "basiscent " << basiscent << endl;
    }
    totbasis+=basiscent;
    if(maxbasis < basiscent)
      maxbasis=basiscent;
  }

  //cout << "maxbasis " << maxbasis << endl;
  //single_write(cout, nmo, " molecular orbitals requested.\n");

  pos=0;
  if(! readvalue(words, pos, orbfile, "ORBFILE"))
  {
    error("Must specify ORBFILE for MO matrix");
  }
  init();

}
//----------------------------------------------------------------------------

#endif // MO_MATRIX_H_INCLUDED

//----------------------------------------------------------------------------
