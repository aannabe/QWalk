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

//
#ifndef AVERAGE_GENERATOR_H_INCLUDED
#define AVERAGE_GENERATOR_H_INCLUDED
#include "Qmc_std.h"
#include "Wavefunction.h"
#include "System.h"



struct Average_return {
  string type;
  Array1 <doublevar> vals;
};

/*!
 The average generators are meant to be averages which are not *too* big(ie, < 1000 doubles), and should
 go into the Properties_point structure.  They will be averaged with error bars, and can do post-processing
 either in the output of gosling or the main code(they use the same functions, so it's automatic).
 
 The interface gives the code writer a lot of power, including the power to mess up the VMC/RMC/DMC calculation.
 There are thus a few rules of engagement.  
 1) Leave the state of sample and wf as you found them.  The final electronic coordinates should never change.
 2) Try to keep the amount of data saved to the bare minimum you need, although if you know that in the future 
 you'll need some internal state to make nice output, then go ahead and store it in the logfile in the init section.
 
 If you find that you need more information than is provided via the evaluate() function(for example, maybe 
                                                                                         there are special weights for RMC or something), you can do the following:
 1) Subclass Average_generator with a different evaluate() function.  Try to keep it general so more than 
 one type can be included with the same parameters.
 2) Make an allocate() function for your subclass and add the subclass to the allocate() for Average_generator
 3) Keep an array of Subclass *'s in the averaging function and have them evaluate into the Properties_points, 
 appending to the normal list of Average_returns.
 That's about all.  Assuming that you write the write_init, read, and write_summaries, correctly, everything
 else should be taken care of.
 
 Futher hints:
 -Averages with weights can be done.  Pack the vals array with val1*weight1, weight1, val2*weight2, weight2, etc.
 The values and weights will be averaged properly, and in write_summary(), you can report <val1*weight1>/<weight1>
 as the average value.  Correlated sampling could(maybe should) be done like this, although it would need
 a slightly different evaluate() function, with the pseudopotential and the random numbers to use to evaluate it.
 
 */
class Average_generator {
public:
  virtual ~Average_generator() {};
  //these are used in the main program.  evaluate() packs the
  //averaging data into the 'vals' array.  read() does as normal
  //write_init will write a section in the log file with relevant data
  //for gosling
  virtual void evaluate(Wavefunction_data * wfdata, Wavefunction * wf,
                        System * sys, Sample_point * sample, Average_return & )=0;
  virtual void read(System * sys, Wavefunction_data * wfdata, vector
                    <string> & words)=0;
  virtual void write_init(string & indent, ostream & os)=0;
  
  //these are used in gosling: read will read in the write_init stuff
  //from above and set any relevant
  //internal variables.  gosling will average together all the
  // Average_returns and give a final averaged one
  //to write_summary, which will give a nice interpretation of the data.
  virtual void read(vector <string> & words)=0;
  virtual void write_summary(Average_return &,Average_return &, ostream & os)=0;
};

int allocate(vector<string> & words, System * sys, Wavefunction_data * wfdata, Average_generator *& avg);
int allocate(vector<string> & words, Average_generator * & avg);



class Average_dipole:public Average_generator { 
public:
  virtual void evaluate(Wavefunction_data * wfdata, Wavefunction * wf,
                        System * sys, Sample_point * sample, Average_return & );
  virtual void read(System * sys, Wavefunction_data * wfdata, vector
                    <string> & words);
  virtual void write_init(string & indent, ostream & os);
  virtual void read(vector <string> & words);
  virtual void write_summary(Average_return &,Average_return &, ostream & os);
};


class Average_structure_factor:public Average_generator { 
public:
  virtual void evaluate(Wavefunction_data * wfdata, Wavefunction * wf,
                        System * sys, Sample_point * sample, Average_return & );
  virtual void read(System * sys, Wavefunction_data * wfdata, vector
                    <string> & words);
  virtual void write_init(string & indent, ostream & os);
  virtual void read(vector <string> & words);
  virtual void write_summary(Average_return &,Average_return &, ostream & os);
private:
  int nelectrons;    
  int npoints;
  int np_side; //!< number of points on the side
  
  //Array1 <doublevar> grid; 
  //Array2 <doublevar> gvec;
  Array2 <doublevar> kpts;
  
};


#endif //AVERAGE_GENERATOR_H_INCLUDED
