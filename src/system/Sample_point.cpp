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

#include "Sample_point.h"
#include "Wavefunction.h"
void Sample_point::attachObserver(Wavefunction * wfptr)
{
  assert(wfptr != NULL);
  wfObserver=wfptr;
  wfptr->notify(sample_attach, 0);
}


int Sample_point::ndim() { return 3; }

//----------------------------------------------------------------------

int read_config(string & last_read, istream & is, 
                Sample_point * sample) {
  if(last_read=="CONFIGS") {
    is >> last_read;
    if(last_read != "{") error("Need a { after CONFIGS");
    sample->rawInput(is);
    is >> last_read;
    if(last_read != "}") error("Need a closing }  for CONFIGS");
    return 1;
  }
  else 
    return 0;
}


void write_config(ostream & os, 
                  Sample_point * sample) {
  os << "   CONFIGS { \n";
  sample->rawOutput(os);
  os << "   }\n";
}


//-------------------------------------------------------------------------


void Config_save_point::savePos(Sample_point * sample) {
  
  int nelectrons=sample->electronSize();
  
  if(electronpos.GetDim(0)!=nelectrons) {
    electronpos.Resize(nelectrons);
    for(int i=0; i< nelectrons; i++) {
      electronpos(i).Resize(3);
  
    }
  }
  
  
  for(int i=0; i< nelectrons; i++) {
    sample->getElectronPos(i,electronpos(i));
  }

  //CM:
  dynspin = sample->dynspin;
  if(dynspin) {
    if(electronspin.GetDim(0)!=nelectrons) {
      electronspin.Resize(nelectrons);
      for (int i = 0; i < nelectrons; i++) 
	sample->getElectronSpin(i,electronspin(i));
    }
  }
}

void Config_save_point::restorePos(Sample_point * sample) {
  int nelectrons=sample->electronSize();
  assert(nelectrons==electronpos.GetDim(0));
  for(int i=0; i< nelectrons; i++) 
    sample->setElectronPos(i,electronpos(i));
  //CM:
  dynspin = sample->dynspin;
  if (dynspin) {
    assert(nelectrons==electronspin.GetDim(0));
    for (int i=0; i < nelectrons; i++)
      sample->setElectronSpin(i,electronspin(i));
  }
}

void Config_save_point::mpiReceive(int node) {
#ifdef USE_MPI
  MPI_Status status;
  int nelectrons;
  MPI_Recv(&nelectrons,1, MPI_INT, node, 0, MPI_Comm_grp,
           &status);
  electronpos.Resize(nelectrons);
  Array2 <doublevar> epos(nelectrons,3);
  MPI_Recv(epos.v,3*nelectrons,MPI_DOUBLE,node,0,MPI_Comm_grp,&status);

  for(int e=0; e< nelectrons; e++) {
    electronpos(e).Resize(3);
    for(int d=0; d < 3; d++) electronpos(e)(d)=epos(e,d);
   // MPI_Recv(electronpos(e).v, 3, MPI_DOUBLE,
   //     node, 0, MPI_Comm_grp, &status);
  }

  //CM:
  if (dynspin) {
      electronspin.Resize(nelectrons);
      Array1 <doublevar> espin(nelectrons);
      MPI_Recv(espin.v,nelectrons,MPI_DOUBLE,node,0,MPI_Comm_grp,&status);
      for (int e = 0; e < nelectrons; e++) {
	  electronspin(e) = espin(e);
      }
  }
#endif
}

void Config_save_point::mpiSend(int node) {
#ifdef USE_MPI
  int nelectrons=electronpos.GetDim(0);
  MPI_Send(&nelectrons,1, MPI_INT, node, 0, MPI_Comm_grp);
  Array2 <doublevar> epos(nelectrons,3);
  for(int e=0; e< nelectrons; e++) {
    for(int d=0; d< 3; d++) epos(e,d)=electronpos(e)(d);
  }
  MPI_Send(epos.v, 3*nelectrons, MPI_DOUBLE,
        node, 0, MPI_Comm_grp);

  //CM:
  if (dynspin) {
    Array1 <doublevar> espin(nelectrons);
    for (int e = 0; e < nelectrons; e++) 
      espin(e) = electronspin(e);
    MPI_Send(espin.v, nelectrons, MPI_DOUBLE, node, 0, MPI_Comm_grp);
  }

#endif
}

//----------------------------------------------------------------------
void Config_save_point::read(istream & is) { 
  string dummy;
  while(is >> dummy) { 
    if(dummy=="nElec") break;
  }
  if(dummy!="nElec") error("Couldn't find nElec reading configurations");
  int nelec;
  is >> nelec;
  electronpos.Resize(nelec);
  is >> dummy;
  assert(dummy=="ndim");
  int ndim;
  is >> ndim;
  //CM:
  if (ndim == 4) {
    dynspin = true;
    electronspin.Resize(nelec);
  }
  for(int e=0; e< nelec; e++) { 
    electronpos(e).Resize(ndim);
    for(int d=0; d< 3; d++) { 
      is >> electronpos(e)(d);
    }
    //CM:
    if (dynspin) is >> electronspin(e);
  }
}
void Config_save_point::write(ostream & os) { 
  //CM:
  //os << "nElec " << electronpos.GetDim(0) << " ndim " << 3 << endl;
  if (!dynspin)
    os << "nElec " << electronpos.GetDim(0) << " ndim " << 3 << endl;
  else
    os << "nElec " << electronpos.GetDim(0) << " ndim " << 4 << endl;
  for(int e=0; e< electronpos.GetDim(0); e++) { 
    for(int d=0; d< 3; d++) { 
      os << electronpos(e)(d) << " ";
    }
    //CM:
    if (dynspin)
      os << electronspin(e);
    os << endl;
  }
}
#include "qmc_io.h"
//----------------------------------------------------------------------
int Config_save_point::writeBinary(FILE * f,doublevar weight) { 
  //for(int e=0; e< electronpos.GetDim(0); e++) { 
  //  fwrite(electronpos(e).v, sizeof(doublevar),3, f);
  //}

  int ndim=electronpos(0).GetDim(0);
  int ne=electronpos.GetDim(0);
  Array1<doublevar> a(ne*ndim+1);
  int count=0;
  for(int e=0; e< ne; e++) { 
    for(int d=0; d< ndim; d++) {
      a(count++)=electronpos(e)(d);
    }
  }
  a(count)=weight;
  binary_write_checksum(a,f);
  
  return 1;
}
//----------------------------------------------------------------------
int Config_save_point::readBinary(FILE * f,int nelec, int ndim, doublevar & weight) { 
  Array1 <doublevar> a;
  if(!binary_read_checksum(a,f,nelec*ndim+1)) return 0;
  if(a.GetDim(0)!=nelec*ndim+1) error("Array size wrong in Config_save_point::readBinary");
  electronpos.Resize(nelec);
  int count=0;
  for(int e=0; e< electronpos.GetDim(0); e++) { 
    electronpos(e).Resize(ndim);
    //nread=fread(electronpos(e).v, sizeof(doublevar),ndim, f);
    for(int d=0; d< ndim; d++) electronpos(e)(d)=a(count++);
  }
  weight=a(count);
  return 1;
  
}
//----------------------------------------------------------------------

