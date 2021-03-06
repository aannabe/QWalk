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

#include "Jastrow2_wf.h"
#include "qmc_io.h"
#include "System.h"
#include "Sample_point.h"
#include "Jastrow2_three.h"
#include "Jastrow2_one.h"

//--------------------------------------------------------  

int Jastrow_threebody_piece::make_default_list() { 
  klm.Resize(12,3);
  klm(0,0)=1; klm(0,1)=1; klm(0,2)=0;
  klm(1,0)=1; klm(1,1)=0; klm(1,2)=1;
  klm(2,0)=1; klm(2,1)=1; klm(2,2)=1;
  klm(3,0)=2; klm(3,1)=2; klm(3,2)=0;
  klm(4,0)=2; klm(4,1)=0; klm(4,2)=1;
  klm(5,0)=2; klm(5,1)=0; klm(5,2)=2;
  klm(6,0)=2; klm(6,1)=2; klm(6,2)=2;
  klm(7,0)=3; klm(7,1)=3; klm(7,2)=0;
  klm(8,0)=3; klm(8,1)=0; klm(8,2)=2;
  klm(9,0)=3; klm(9,1)=3; klm(9,2)=2;
  klm(10,0)=1; klm(10,1)=2; klm(10,2)=2;
  klm(11,0)=2; klm(11,1)=3; klm(11,2)=2;
  return 12;

}

void Jastrow_threebody_piece::set_up(vector <string> & words, 
                                   System * sys) {
                                   
  vector<string> atomnames;
  sys->getAtomicLabels(atomnames);
  get_onebody_parms(words, atomnames, unique_parameters,
		    _nparms, parm_labels,
		    linear_parms, parm_centers);
  unsigned int pos=0;
  
  int maxnparms=0;
  vector <string> klm_list;
  if(readsection(words,pos=0,klm_list,"SM_TERMS")) { 
    if(klm_list.size()%3 !=0) 
      error("SM_TERMS must contain a multiple of 3 elements");
    maxnparms=klm_list.size()/3;
    int counter=0;
    klm.Resize(maxnparms,3);
    for(int i=0; i< maxnparms; i++) { 
      for(int j=0; j< 3; j++) {
        klm(i,j)=atoi(klm_list[counter++].c_str());
      }
    }
  }
  else maxnparms=make_default_list();

  
  


  int natoms=atomnames.size();
  eebasis_max=0;
  eibasis_max.Resize(natoms);
  eibasis_max=0;
  for(int at=0; at< natoms; at++) {
    if(nparms(at) > maxnparms) 
      error("too many three-body parameters on ", atomnames[at]);
    for(int p=0; p < nparms(at); p++) { 
      if(eibasis_max(at) <= klm(p,0)) eibasis_max(at)=klm(p,0)+1;
      if(eibasis_max(at) <= klm(p,1)) eibasis_max(at)=klm(p,1)+1;
      if(eebasis_max <= klm(p,2)) eebasis_max=klm(p,2)+1;
    }
  }

  freeze=haskeyword(words, pos=0, "FREEZE");

}
//--------------------------------------------------------------------------

int Jastrow_threebody_piece::writeinput(string & indent, ostream & os) {
  if(freeze) os << indent << "FREEZE" << endl;
  for(int i=0; i< unique_parameters.GetDim(0); i++) {
    os << indent << "COEFFICIENTS { " << parm_labels[i] << "  ";
    for(int j=0; j< _nparms(i); j++)
      os << unique_parameters(i,j) << "  ";
    os << " } " << endl;
  }

  os << indent << "SM_TERMS {\n";
  for(int i=0; i< klm.GetDim(0); i++) {
    os << indent << "  ";
    for(int j=0; j< klm.GetDim(1); j++) { 
      os << klm(i,j) << "  ";
    }
    os << endl;
  }
  os << indent << "}\n";

  return 1;

}

//--------------------------------------------------------------------------

int Jastrow_threebody_piece::showinfo(string & indent, ostream & os) {
  os << indent << "Atom   Coefficients " << endl;
  for(int i=0; i< unique_parameters.GetDim(0); i++) {
    os << indent << parm_labels[i] << "  ";
    for(int j=0; j< _nparms(i); j++)
      os << unique_parameters(i,j) << "  ";
    os << endl;
  }
  return 1;
  
}


//--------------------------------------------------------------------------


inline void eval_threebody_derivative(doublevar parm,
    double * eiek, double * eiel, double * eijk, double * eijl, 
    double * ee, double sign,
    double * lap0, double * lap1) { 
  /*
  doublevar vkl=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,0)
      +eibasis(j,at,k,0)*eibasis(e,at,el,0));


  for(int d=1; d< 5; d++) {
    vkl_e[d]=parm*(eibasis(e,at,k,d)*eibasis(j,at,el,0)
        +eibasis(j,at,k,0)*eibasis(e,at,el,d));
    vkl_j[d]=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,d)
        +eibasis(j,at,k,d)*eibasis(e,at,el,0));
  } 
  lap(0,j,0)+=vkl*eebasis(j,m,0);
  doublevar dot_e=0, dot_j=0;
  for(int d=1; d< 4; d++) {
    lap(0,j,d)+=vkl_e[d]*eebasis(j,m,0);
    lap(0,j,d)-=vkl*eebasis(j,m,d);
    lap(1,j,d)+=vkl_j[d]*eebasis(j,m,0);
    lap(1,j,d)+=vkl*eebasis(j,m,d);

    dot_e+=vkl_e[d]*eebasis(j,m,d);
    dot_j+=vkl_j[d]*eebasis(j,m,d);
  }


  lap(0,j,4)+=vkl*eebasis(j,m,4);
  lap(1,j,4)+=vkl*eebasis(j,m,4);

  lap(0,j,4)+=vkl_e[4]*eebasis(j,m,0);
  lap(1,j,4)+=vkl_j[4]*eebasis(j,m,0);

  lap(0,j,4)-=2*dot_e;
  lap(1,j,4)+=2*dot_j;
  */
  doublevar vkl=parm*(eiek[0]*eijl[0]+eiel[0]*eijk[0]);
  doublevar  vkl_e[5],vkl_j[5];
  for(int d=1; d< 5; d++) {
    vkl_e[d]=parm*(eiek[d]*eijl[0]+eijk[0]*eiel[d]);
    vkl_j[d]=parm*(eiek[0]*eijl[d]+eijk[d]*eiel[0]);
  }
  lap0[0]+=vkl*ee[0];
  doublevar dot_e=0,dot_j=0;
  for(int d=1; d< 4; d++) { 
    lap0[d]+=vkl_e[d]*ee[0];
    lap0[d]-=sign*vkl*ee[d];
    lap1[d]+=vkl_j[d]*ee[0];
    lap1[d]+=sign*vkl*ee[d];
    dot_e+=vkl_e[d]*ee[d];
    dot_j+=vkl_j[d]*ee[d];
  }
  lap0[4]+=vkl*ee[4];
  lap1[4]+=vkl*ee[4];
  lap0[4]+=vkl_e[4]*ee[0];
  lap1[4]+=vkl_j[4]*ee[0];
  lap0[4]-=2*sign*dot_e;
  lap1[4]+=2*sign*dot_j;
    
} 


void Jastrow_threebody_piece::updateLap(int e,
                 const Array4 <doublevar> & eibasis,
                 const Array3 <doublevar> & eebasis,
                 Array3 <doublevar> & lap) {
  //cout << "Jastrow_threebody_piece::updateLap" << endl;
  assert(lap.GetDim(2) >= 5);
  assert(lap.GetDim(0) >=2);
  
  assert(eibasis.GetDim(1) >= parm_centers.GetDim(0));
  int natoms=parm_centers.GetDim(0);
  int nelectrons=eebasis.GetDim(0);
  assert(lap.GetDim(1) >= nelectrons);
  Array1 <doublevar> vkl_e(5); //derivative wrt e of a_e a_j + a_j a_e
  Array1 <doublevar> vkl_j(5); //wrt j

  const doublevar tiny=1e-14;
  //cout << "updateLap " << endl;

  //Find electrons with non-zero basis
  int nelectrons_nonzero=0;
  Array1 <int> electrons_nonzero(nelectrons);
  for(int j=0; j< nelectrons; j++) { 
    bool in_range=false;
    for(int m=0; m < eebasis.GetDim(1); m++) { 
      if(fabs(eebasis(j,m,0)) > tiny) {
        in_range=true;
      }
    }
    if(in_range and e!=j) {
      electrons_nonzero(nelectrons_nonzero++)=j;
    }
  }
  //cout << nelectrons << " " << nelectrons_nonzero << endl;


  int ei_nb=eibasis.GetDim(2);
  int ee_nb=eebasis.GetDim(1);
  int nd=5;
  //Scaling factors for the electron-ion basis
  int ei_s1=eibasis.GetDim(1)*eibasis.GetDim(2)*eibasis.GetDim(3);
  int ei_s2=eibasis.GetDim(2)*eibasis.GetDim(3);
  int ei_s3=eibasis.GetDim(3);

  for(int at=0; at < natoms; at++) {
    int p=parm_centers(at);

    for(int i=0; i< _nparms(p); i++) {
      doublevar parm=unique_parameters(p,i);
      int k=klm(i,0), el=klm(i,1), m=klm(i,2);
      if(fabs(eibasis(e,at,k,0)) > tiny
          || fabs(eibasis(e,at,el,0)) > tiny) { 
        
        //lap=0;
        
        for(int jj=0; jj< nelectrons_nonzero; jj++) {
          int j=electrons_nonzero(jj);
          double sign=1;
          if( j> e) sign=-1;
          eval_threebody_derivative(parm,
              eibasis.v+ e*ei_s1 + at*ei_s2 + k *ei_s3,
              eibasis.v+ e*ei_s1 + at*ei_s2 + el*ei_s3,
              eibasis.v+ j*ei_s1 + at*ei_s2 + k *ei_s3,
              eibasis.v+ j*ei_s1 + at*ei_s2 + el*ei_s3,
              eebasis.v+ j*ee_nb*nd + m*nd,
              sign,
              lap.v+j*nd,
              lap.v+nelectrons*nd+j*nd);
        }
        
        

        //for(int j=0; j < nelectrons; j++) { 
        //  cout << "new lap ";
        //  for(int d=0; d< 5; d++) 
        //    cout << lap(0,j,d) << " ";
        //  cout << endl;
        //}
        
          
//------------        
      //lap=0;
      
        /*
      for(int j=0; j< e; j++) { 
        doublevar vkl=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,0)
            +eibasis(j,at,k,0)*eibasis(e,at,el,0));


          for(int d=1; d< 5; d++) {
            vkl_e[d]=parm*(eibasis(e,at,k,d)*eibasis(j,at,el,0)
                +eibasis(j,at,k,0)*eibasis(e,at,el,d));
            vkl_j[d]=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,d)
                +eibasis(j,at,k,d)*eibasis(e,at,el,0));
          } 
          lap(0,j,0)+=vkl*eebasis(j,m,0);
          doublevar dot_e=0, dot_j=0;
          for(int d=1; d< 4; d++) {
            lap(0,j,d)+=vkl_e[d]*eebasis(j,m,0);
            lap(0,j,d)-=vkl*eebasis(j,m,d);
            lap(1,j,d)+=vkl_j[d]*eebasis(j,m,0);
            lap(1,j,d)+=vkl*eebasis(j,m,d);

            dot_e+=vkl_e[d]*eebasis(j,m,d);
            dot_j+=vkl_j[d]*eebasis(j,m,d);
          }

          //cout << "old: vkl " << vkl << endl;
          //cout << "old: dot_e " << dot_e <<  "dot_j " << dot_j << endl;
          

          lap(0,j,4)+=vkl*eebasis(j,m,4);
          lap(1,j,4)+=vkl*eebasis(j,m,4);

          lap(0,j,4)+=vkl_e[4]*eebasis(j,m,0);
          lap(1,j,4)+=vkl_j[4]*eebasis(j,m,0);

          lap(0,j,4)-=2*dot_e;
          lap(1,j,4)+=2*dot_j;
        }
        
        
        for(int j=e+1; j< nelectrons; j++) {
          doublevar vkl=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,0)
              +eibasis(j,at,k,0)*eibasis(e,at,el,0));
          for(int d=1; d< 5; d++) {
            vkl_e[d]=parm*(eibasis(e,at,k,d)*eibasis(j,at,el,0)
                +eibasis(j,at,k,0)*eibasis(e,at,el,d));
            vkl_j[d]=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,d)
                +eibasis(j,at,k,d)*eibasis(e,at,el,0));
          } 

          lap(0,j,0)+=vkl*eebasis(j,m,0);
          doublevar dot_e=0, dot_j=0;
          for(int d=1; d< 4; d++) {
            lap(0,j,d)+=vkl_e[d]*eebasis(j,m,0);
            lap(0,j,d)+=vkl*eebasis(j,m,d);
            lap(1,j,d)+=vkl_j[d]*eebasis(j,m,0);
            lap(1,j,d)-=vkl*eebasis(j,m,d);

            dot_e+=vkl_e[d]*eebasis(j,m,d);
            dot_j+=vkl_j[d]*eebasis(j,m,d);
          }
          //cout << "old: vkl " << vkl << endl;
          //cout << "old: dot_e " << dot_e <<  "dot_j " << dot_j << endl;
          
          lap(0,j,4)+=vkl*eebasis(j,m,4);
          lap(1,j,4)+=vkl*eebasis(j,m,4);

          lap(0,j,4)+=vkl_e[4]*eebasis(j,m,0);
          lap(1,j,4)+=vkl_j[4]*eebasis(j,m,0);

          lap(0,j,4)+=2*dot_e;
          lap(1,j,4)-=2*dot_j;
        }
      */
        //for(int j=0; j < nelectrons; j++) { 
        //  cout << "old lap ";
        //  for(int d=0; d< 5; d++) 
         //   cout << lap(0,j,d) << " ";
         // cout << endl;
        //}
        
        
      }
    }
  }
  
  
}

//-----------------------------------------------------------
void Jastrow_threebody_piece::updateVal(int e,
                                         const Array4 <doublevar> & eibasis,
                                         const Array3 <doublevar> & eebasis,
                                         Array1 <doublevar> & updated_val) {

  assert(eibasis.GetDim(1) >= parm_centers.GetDim(0));
  int natoms=parm_centers.GetDim(0);
  int nelectrons=eebasis.GetDim(0);

  const doublevar tiny=1e-14;
  //cout << "updateLap " << endl;
  for(int at=0; at < natoms; at++) {
    int p=parm_centers(at);
    for(int i=0; i< _nparms(p); i++) {
      doublevar parm=unique_parameters(p,i);
      int k=klm(i,0), el=klm(i,1), m=klm(i,2);
      if(fabs(eibasis(e,at,k,0)) > tiny
          || fabs(eibasis(e,at,el,0)) > tiny) { 
        for(int j=0; j< e; j++) {

          doublevar vkl=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,0)
              +eibasis(j,at,k,0)*eibasis(e,at,el,0));
          updated_val(j)+=vkl*eebasis(j,m,0);
        }
        for(int j=e+1; j< nelectrons; j++) {
          doublevar vkl=parm*(eibasis(e,at,k,0)*eibasis(j,at,el,0)
              +eibasis(j,at,k,0)*eibasis(e,at,el,0));
          updated_val(j)+=vkl*eebasis(j,m,0);
        }
      }
    }
  } 
}


//-----------------------------------------------------------

void Jastrow_threebody_piece::getParmDeriv(const Array3 <doublevar> & eibasis,
                                        const Array3 <doublevar> & eebasis,
                                       Parm_deriv_return & deriv) {
  
  assert(eibasis.GetDim(1) >= parm_centers.GetDim(0));
  int natoms=parm_centers.GetDim(0);
  int nelectrons=eebasis.GetDim(0);
  
  const doublevar tiny=1e-14;
  //cout << "updateLap " << endl;
  for(int at=0; at < natoms; at++) {
    int p=parm_centers(at);
    for(int i=0; i< _nparms(p); i++) {
      int index=linear_parms(p,i);
      int k=klm(i,0), el=klm(i,1), m=klm(i,2);
      for(int e=0; e< nelectrons; e++) { 
        if(fabs(eibasis(e,at,k)) > tiny
           || fabs(eibasis(e,at,el)) > tiny) { 
          for(int j=e+1; j< nelectrons; j++) {
            doublevar vkl=(eibasis(e,at,k)*eibasis(j,at,el)
                                +eibasis(j,at,k)*eibasis(e,at,el));
            deriv.gradient(index)+=vkl*eebasis(e,j,m);
          }
        }
      }
    }
  } 
}
//----------------------------------------------------------------------

void Jastrow_threebody_piece::getParmDeriv(const Array4 <doublevar> & eibasis,
                                        const Array4 <doublevar> & eebasis,
                                       Parm_deriv_return & deriv) {
  int natoms=parm_centers.GetDim(0);
  int nelectrons=eebasis.GetDim(0);
  //assert(lap.GetDim(1) >= nelectrons);
  int np=nparms();
  Array2 <int> index(unique_parameters.GetDim(0),unique_parameters.GetDim(1));  
  Array1 <doublevar> coeff(np);

  int counter=0;
  for(int i=0; i< unique_parameters.GetDim(0); i++) {
    for(int j=0; j< _nparms(i); j++) {
      index(i,j)=counter;
      coeff(counter++) = unique_parameters(i,j);//*natoms;
    }
  }



  const doublevar tiny=1e-14;
  int ei_nb=eibasis.GetDim(2);
  int ee_s1=eebasis.GetDim(1)*eebasis.GetDim(2)*eebasis.GetDim(3);
  int ee_s2=eebasis.GetDim(2)*eebasis.GetDim(3);
  int ee_s3=eebasis.GetDim(3);
  int nd=5;
  //Scaling factors for the electron-ion basis
  int ei_s1=eibasis.GetDim(1)*eibasis.GetDim(2)*eibasis.GetDim(3);
  int ei_s2=eibasis.GetDim(2)*eibasis.GetDim(3);
  int ei_s3=eibasis.GetDim(3);

  doublevar lap1[5];
  doublevar lap2[5];
  Array3 <doublevar> func(np,nelectrons,5,0.0);
  for(int at=0; at < natoms; at++) {
    int p=parm_centers(at);

    for(int ind=0; ind< _nparms(p); ind++) {
      doublevar parm=1.0;//unique_parameters(p,i);
      int thisindex=index(p,ind);
      int k=klm(ind,0), el=klm(ind,1), m=klm(ind,2);

      for(int i=0; i< nelectrons; i++) { 
        for(int j=i+1; j< nelectrons; j++) {
          //double sign=1;
          //if( j> e) sign=-1;
          doublevar sign=-1;
          for(int d=0; d< 5; d++) { 
            lap1[d]=0.0;
            lap2[d]=0.0;
          }
          eval_threebody_derivative(parm,
              eibasis.v+ i*ei_s1 + at*ei_s2 + k *ei_s3,
              eibasis.v+ i*ei_s1 + at*ei_s2 + el*ei_s3,
              eibasis.v+ j*ei_s1 + at*ei_s2 + k *ei_s3,
              eibasis.v+ j*ei_s1 + at*ei_s2 + el*ei_s3,
              eebasis.v+ i*ee_s1 + j *ee_s2 + m *ee_s3,
              sign,
              lap1,
              lap2);
          func(thisindex,i,0)+=lap1[0];
          for(int d=1; d< 5; d++) { 
            func(thisindex,i,d)+=lap1[d];
            func(thisindex,j,d)+=lap2[d];
          }
        }
      }
    }
  }

  if(freeze) create_parm_deriv_frozen(func,coeff,deriv);
  else create_parm_deriv(func,coeff,deriv);
  
}


//-----------------------------------------------------------

int Jastrow_threebody_piece::nparms() {
  int nsec=unique_parameters.GetDim(0);
  assert(nsec==_nparms.GetDim(0));

  if(freeze) return 0;
  int tot=0;
  for(int i=0; i<nsec; i++)
    tot+=_nparms(i);
  return tot;
}
//-----------------------------------------------------------

void Jastrow_threebody_piece::getParms(Array1 <doublevar> & parms) {
  if(freeze) {
    parms.Resize(0);
  }
  else {
    parms.Resize(nparms());
    int counter=0;
    int natoms=parm_centers.GetDim(0);
    for(int i=0; i< unique_parameters.GetDim(0); i++) {
      for(int j=0; j< _nparms(i); j++) {
        parms(counter++) = unique_parameters(i,j);//*natoms;
        
      }
    }
  }

}
//-----------------------------------------------------------

void Jastrow_threebody_piece::setParms(Array1 <doublevar> & parms) {
  assert(parms.GetDim(0)==nparms());

  if(freeze) return;
  int counter=0;
  int natoms=parm_centers.GetDim(0);
  for(int i=0; i< unique_parameters.GetDim(0); i++) {
    for(int j=0; j< _nparms(i); j++) {
      unique_parameters(i,j)=parms(counter++);//natoms;
      //cout << "set one-body " << unique_parameters(i,j) << endl;
    }
  }
}

//--------------------------------------------------------------------------
