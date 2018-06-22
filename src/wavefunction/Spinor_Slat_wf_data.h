/*
 
Original Copyright (C) 2007 Lucas K. Wagner

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

#ifndef SPINOR_SLAT_WF_DATA_H_INCLUDED
#define SPINOR_SLAT_WF_DATA_H_INCLUDED

#include "Qmc_std.h"
#include "Wavefunction_data.h"
#include "MO_matrix.h"
#include "clark_updates.h"
#include "Orbital_rotation.h"
template <class T> class Spinor_Slat_wf;
/*!
\brief
A Slater determinant or several determinants.  Also can use the same
molecular orbitals to create several separate combinations of determinants,
and return them as a vector.

<h3> Options </h3> <br>
<b> Required </b> <br>
<ul>

<li>
<b> DETWT </b> List of the weights of the determinants. 
</li>

<li>
<b> STATES </b> List of the occupations of the molecular orbitals, first
spin up, then spin down.  For example, a RHF determinant might be <br>
1 2 3 <br>
1 2 3 <br>
and a UHF might be <br>
1 2 3 <br>
4 5 6 <br>
If there are more than one determinants, simply continue listing up and 
down occupations.<br>  Also, list several STATES functions, and they
will be added to the vector in order. 
</li>

<li>
<b> ORBITALS </b> This section is an input deck for an MO_matrix
</li>
</ul>

<b> Optional </b>
<ul>
<li>
<b> NSPIN </b>  Override the global parameter.
</li>
</ul>

 */
class Spinor_Slat_wf_data : public Wavefunction_data
{
public:

  Spinor_Slat_wf_data():molecorb(NULL) {}

  ~Spinor_Slat_wf_data()
  {
    if(molecorb != NULL ) delete molecorb;
    if(cmolecorb != NULL) delete cmolecorb;
  }

  int optimize_mo; //!< whether to optimize MO basis
  virtual int valSize() {
    if(optimize_mo) return 0;
    if(optimize_det) return 2*detwt.GetDim(0);
    else return nfunc*2;
  }
  virtual void lockInParms(Array1<doublevar> & parms){
    if(optimize_mo)
     error("Can't optimize MOs");
     //orbrot->lockIn(parms);
  }
  virtual void getVarParms(Array1 <doublevar> & parms);
  virtual void setVarParms(Array1 <doublevar> & parms);
  virtual void linearParms(Array1 <bool> & is_linear);

  virtual void renormalize();

  virtual void read(vector <string> & words,
                    unsigned int & pos,
                    System * sys
                   );
  virtual int supports(wf_support_type );
  void generateWavefunction(Wavefunction *&);


  int showinfo(ostream & os);

  int writeinput(string &, ostream &);

  int nparms()
  {
    if(optimize_mo) 
      error("Can't optimize MOs");
      //return orbrot->nparms();
    else if(optimize_det){ return ncsf-1; } 
    else return 0;
  }

  Spinor_Slat_wf_data * clone() const{
    return new Spinor_Slat_wf_data(*this);
  }

private:
  void init_mo();
  friend class Spinor_Slat_wf<doublevar>;
  friend class Spinor_Slat_wf<dcomplex>;
  //friend class Cslat_wf;

  Array2 < Array1 <int> > occupation;
  //!< The place in totoccupation for the molecular orbitals for (fn, det) Used to look up the right MO from MO_matrix.
  Array2 < Array1 <int> > occupation_orig;
  //!< The molecular orbitals numbers for (function, determinant)
  Array1 <log_value<doublevar> > detwt;
  Array1 <int> totoccupation; //!< all the molecular orbitals 

  int max_occupation_changes;

  int nelectrons; //!< number of electrons

  int nmo;        //!<Number of molecular orbitals
  int ndet;       //!<Number of determinants
  int ndim;       //!<Number of (spacial) dimensions each electron has
  int nfunc;      //!<Number of separate functions this represents

  Array1 <int> orbitals_for_optimize_mo; //!< which orbitals should be optimized 
  string mo_place; //!< where to place the new mo's
  int optimize_det; //!< whether to optimize determinant coefficients
  Array1 <Array1 <doublevar> > CSF;
  int ncsf; 
  int sort; //whether to sort det. weights by size
  
  
  General_MO_matrix * genmolecorb; 
  MO_matrix * molecorb;
  int use_complexmo;
  bool use_clark_updates; //!<Use Bryan Clark's updates.
  Spinor_Excitation_list excitations;
  Complex_MO_matrix * cmolecorb;
  Orbital_rotation * orbrot;

};

#endif //SPINOR_SLAT_WF_DATA_H_INCLUDED
//------------------------------------------------------------------------
