/*
 
Copyright (C) 2007 Jindrich Kolorenc, Michal Bajdich

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



#include "Qmc_std.h"
#include "MO_matrix_basfunc.h"
#include "Sample_point.h"
#include "qmc_io.h"

//--------------------------------------------------------------------------

void MO_matrix_basfunc::init() {

  single_write(cout, "Basis function MO\n");
  
  //Determine where to cut off the basis functions
  cutoff.Resize(totbasis);
  
  int basiscounter=0;
  for(int ion=0; ion < centers.equiv_centers.GetDim(0); ion++) {
    for(int c=0; c < centers.ncenters_atom(ion); c++) {
      int cen2=centers.equiv_centers(ion, c);
      for(int j=0; j< centers.nbasis(cen2); j++){
	Basis_function* tempbasis=basis(centers.basis(cen2,j));
	for(int n=0; n< tempbasis->nfunc(); n++)
	  {
	    //cout << "cutoff " << endl;
	    cutoff(basiscounter)=tempbasis->cutoff(n);
	    //cout << "rcut(" << basiscounter << ") "
	    //     << cutoff(basiscounter) << endl;
	    basiscounter++;
	  }
      }
    }
  }

  obj_cutoff.Resize(basis.GetDim(0));
  for(int b=0; b< basis.GetDim(0); b++) {
    int nf=basis(b)->nfunc();
    doublevar maxcut=basis(b)->cutoff(0);
    for(int n=1; n< nf; n++) {
      doublevar cut=basis(b)->cutoff(n);
      if(cut > maxcut) maxcut=cut;
    }
    obj_cutoff(b)=maxcut;
  }
  

  ifstream ORB(orbfile.c_str());
  if(!ORB)
  {
    error("couldn't find orb file ", orbfile);
  }

  single_write(cout,"Reading orbitals from ",orbfile, "\n");
  
  // totbasis is excessively large as a second dimension of MOmat (we need
  // only number of AO's at given center), but this way we
  // are wasting only integers, so it is not so bad
  int centermax=centers.size();
  Array2 <int> MOmat(centers.equiv_centers.GetDim(0),maxbasis);
  eq_centers.Resize(nmo);
  MOmat=-1;
  int mo=0;
  int ao, ion;
  // read orb file, find out if a pair (ion,ao) corresponds to an
  // occupied MO and if it does, give it a number (or leave this
  // number -1 otherwise)
  while ( ORB >> ao >> ion ) {
    if ( mo+1 > nmo ) break;
    // remember: orb file uses human (and FORTRAN) friendly indexing
    // starting from 1 (as oposed to 0 in C/C++)
    MOmat(ion-1,ao-1)=mo;
    eq_centers(mo)=ion-1;
    mo++;
  }
  ORB.close();
  if ( mo < nmo ) {
    error("BASFUNC_MO: Not enough MOs in orb file");
  }
  single_write(cout, mo, " molecular orbitals found.\n\n");

  // find the position, called bf here, of occuppied (ion,ao) in the
  // sequentially ordered basis set



  basisMO.Resize(nmo,centermax);
  kptfac.Resize(centermax);
  basisMO=-1;
  int bf=0;
  
  for(ion=0; ion < centers.equiv_centers.GetDim(0); ion++) {
    for(int c=0; c < centers.ncenters_atom(ion); c++) {
      int cen2=centers.equiv_centers(ion, c);
      
      doublevar dot=0;
      for(int d=0; d<3; d++) 
	dot+=centers.centers_displacement(cen2,d)*kpoint(d);

      kptfac(cen2)=cos(dot*pi);

      ao=0; //# of basis func on each center
      for(int n=0; n < centers.nbasis(cen2); n++) {
	for(int l=0; l < basis(centers.basis(cen2,n))->nfunc(); l++) {
	  if ( MOmat(ion,ao) > -1 && MOmat(ion,ao) < nmo ) {
	    basisMO(MOmat(ion,ao),cen2)=bf;
	    // cout <<"MOmat(" << ion << "," << ao << ")  "<<MOmat(ion,ao)
	    //	 <<"  for center "<<cen2<<" total function is  "<< bf << endl;
	  }
	  bf++;
	  ao++;
	}
      }
    }
  }

  // for(int ion=0;ion<centermax;ion++)
  //cout << "kptfac " <<  kptfac(ion) << "  displacement " 
  // << centers.centers_displacement(ion,0) << "   "
  // << endl;            



  // cout << "orb file processing finished.\n";

}


//----------------------------------------------------------------------


void MO_matrix_basfunc::buildLists(Array1 < Array1 <int> > & occupations) {
  int numlists=occupations.GetDim(0);
  moLists.Resize(numlists);
  for(int lis=0; lis < numlists; lis++) {
    int nmo_list=occupations(lis).GetDim(0);
    moLists(lis).Resize(nmo_list);
    for(int mo=0; mo < nmo_list; mo++) {
      moLists(lis)(mo)=occupations(lis)(mo);
    }
  }
}

//----------------------------------------------------------------------

int MO_matrix_basfunc::showinfo(ostream & os)
{
  os << "Simple molecular orbital ( MO = basis function )\n";
  string indent="  ";
  os << "Basis functions: \n";
  for(int i=0; i< basis.GetDim(0); i++) {
    basis(i)->showinfo(indent, os);
  }

  os << "Number of molecular orbitals: " << nmo << endl;

  return 1;
}

int MO_matrix_basfunc::writeinput(string & indent, ostream & os)
{
  os << indent << "BASFUNC_MO" << endl;
  os << indent << "NMO " << nmo << endl;
  if(oldsofile!="") 
    os << indent << "OLDSOFILE " << oldsofile << endl;
  os << indent << "ORBFILE " << orbfile << endl;
  // MAGNIFY might help in the case of large number of particles,
  // when numerics starts to fail (at least numerics is what I blame
  // when I start getting strange results or "nan" around 200 particles
  // per spin channel). Setting MAGNIFY < 1 seems to move this limit
  // higher. What worked for me
  //   200 per channel        MAGNIFY 0.5
  //   300 per channel        MAGNIFY 0.1 (0.5 is not enough here)
  os << indent << "MAGNIFY " << magnification_factor << endl;
  string indent2=indent+"  ";
  for(int i=0; i< basis.GetDim(0); i++) {
    os << indent << "BASIS { " << endl;
    basis(i)->writeinput(indent2, os);
    os << indent << "}" << endl;
  }

  os << indent << "CENTERS { " << endl;
  centers.writeinput(indent2, os);
  os << indent << "}" << endl;
  return 1;
}

//------------------------------------------------------------------------

void MO_matrix_basfunc::updateVal(Sample_point * sample, int e,
                                   int listnum,
                                   //!< which list to use
                                   Array2 <doublevar> & newvals
                                   //!< The return: in form (MO, val)
) {

  Array1 <doublevar> basisvals(totbasis);
  centers.updateDistance(e, sample);
  Basis_function * tempbasis;
  Array1 <doublevar> R(5);
  int currfunc=0;
  newvals=0;
  basisvals=0;
  int nmo_list=moLists(listnum).GetDim(0);

  // store values of basis functions on all centers in basisvals;
  // ---
  // don't be fooled, centers.nbasis(ion) is NOT a number of
  // individual basis functions on an ion, it is something like
  // a number of sets of functions, correspondingly, the
  // tempbasis->nfunc() returns number of ALL "planewaves" on the
  // ion, for instance;
  // ---
  // it might happen that I do not need all basis functions, e.g.
  // spin-ups use some and spin-downs use some others, 
  // calcVal calculates them all, however
 
  for(int ion=0; ion < centers.equiv_centers.GetDim(0); ion++) {
    for(int c=0; c < centers.ncenters_atom(ion); c++) {
      int cen2=centers.equiv_centers(ion, c);
      centers.getDistance(e, cen2, R);
      for(int n=0; n < centers.nbasis(cen2); n++) {
	int b=centers.basis(cen2,n);
	tempbasis=basis(b);
	if(R(0) <obj_cutoff(b)) {
	  tempbasis->calcVal(R, basisvals, currfunc);
	  currfunc+=tempbasis->nfunc();
	  for(int m=0; m < nmo_list; m++) {
	    int mo=moLists(listnum)(m);
	    if(eq_centers(mo)==ion){
	       int basfunc=basisMO(mo,cen2);
	       assert(basfunc<currfunc+1);
	       if(R(0) < cutoff(basfunc)){
		 newvals(m,0)+=magnification_factor*basisvals(basfunc)*kptfac(cen2);
	       }
	    }
	  }
	}
	else 
	  currfunc+=tempbasis->nfunc();
      }
    }
  }
}

//------------------------------------------------------------------------


void MO_matrix_basfunc::updateLap(Sample_point * sample, int e,
				  int listnum,
				  //!< which list to use
				  Array2 <doublevar> & newvals
				  //!< The return: in form (MO, [val, grad, lap])
) {
  // exactly the same structure as updateVal above, only assigns
  // 5 values (function, 3 gradient components, laplacian) instead
  // of just one
  Array2 <doublevar> basisvals(totbasis,5);
  centers.updateDistance(e, sample);
  Basis_function * tempbasis;
  Array1 <doublevar> R(5);
  int currfunc=0;
  newvals=0;
  basisvals=0;
  int nmo_list=moLists(listnum).GetDim(0);

  for(int ion=0; ion < centers.equiv_centers.GetDim(0); ion++) {
    for(int c=0; c < centers.ncenters_atom(ion); c++) {
      int cen2=centers.equiv_centers(ion, c);
      centers.getDistance(e, cen2, R);
      for(int n=0; n < centers.nbasis(cen2); n++) {
	int b=centers.basis(cen2,n);
	tempbasis=basis(b);
	if(R(0)< obj_cutoff(b)) {
	  tempbasis->calcLap(R, basisvals, currfunc);
	  currfunc+=tempbasis->nfunc();
	  
	  for(int m=0; m < nmo_list; m++) {
	    int mo=moLists(listnum)(m);
	    if(eq_centers(mo)==ion){
	      int basfunc=basisMO(mo,cen2);
	      assert(basfunc<currfunc+1);
	      if(R(0) < cutoff(basfunc)){
		for (int d=0; d < 5; d++) {
		  newvals(m,d)+=magnification_factor*basisvals(basfunc,d)*kptfac(cen2);
		} 
	      }
	    }
	  }
	  
	}
	else{
	  currfunc+=tempbasis->nfunc();
	}
      }
    }
  }
}

//--------------------------------------------------------------------------


void MO_matrix_basfunc::updateHessian(
  Sample_point * sample,
  int e,
  int listnum,
  //const Array1 <int> & occupation,
  //!<A list of the MO's to evaluate
  Array2 <doublevar> & newvals
  //!< The return: in form (MO, [val, grad, dxx,dyy,...])
)
{

  assert(e < sample->electronSize());
  assert(newvals.GetDim(1)==10);
  // exactly the same structure as updateVal above, only assigns
  // 5 values (function, 3 gradient components, laplacian) instead
  // of just one
  Array2 <doublevar> basisvals(totbasis,10);
  centers.updateDistance(e, sample);
  Basis_function * tempbasis;
  Array1 <doublevar> R(5);
  int currfunc=0;
  newvals=0;
  basisvals=0;
  int nmo_list=moLists(listnum).GetDim(0);


  for(int ion=0; ion < centers.equiv_centers.GetDim(0); ion++) {
    for(int c=0; c < centers.ncenters_atom(ion); c++) {
      int cen2=centers.equiv_centers(ion, c);
      centers.getDistance(e, cen2, R);
      for(int n=0; n < centers.nbasis(cen2); n++) {
	int b=centers.basis(cen2,n);
	tempbasis=basis(b);
	if(R(0)< obj_cutoff(b)) {
	  tempbasis->calcHessian(R, basisvals, currfunc);
	}
	currfunc+=tempbasis->nfunc();
      }
      for(int m=0; m < nmo_list; m++) {
	int mo=moLists(listnum)(m);
	if(eq_centers(mo)==ion){
	  int basfunc=basisMO(mo,cen2);
	  if(R(0) < cutoff(basfunc)){
	    for (int d=0; d < 10; d++) {
	      newvals(m,d)+=magnification_factor*basisvals(basfunc,d)*kptfac(cen2);
	    }
	  }
	}
      }
    }
  }
}
//--------------------------------------------------------------------------
