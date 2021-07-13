######################################################################
# Compiler definitions for Summit

CXX:=mpicxx
CXX_SERIAL:=g++

CXXFLAGS := -O2 
CXXFLAGS += -DUSE_MPI -DUSE_BLAS -DUSE_LAPACK ${INCLUDEPATH} 

LAPACK_LIBS := -L${OLCF_NETLIB_LAPACK_ROOT}/lib64 -llapack -lblas -lcblas
LAPACK_INCLUDE := -I${OLCF_NETLIB_LAPACK_ROOT}/include

BLAS_LIBS :=  $(LAPACK_LIBS)
BLAS_INCLUDE := $(LAPACK_INCLUDE)

DEBUG:= -Wall -DNO_RANGE_CHECKING -DNDEBUG -D__USE_GNU -DDEBUG_WRITE
LDFLAGS:=

######################################################################
# This is the invokation to generate dependencies
DEPENDMAKER:=g++ -MM  $(INCLUDEPATH)

#module load gcc/7.4.0
#module load spectrum-mpi
#module load essl
#module load netlib-lapack
#module load netlib-scalapack
#module load hdf5
#module load fftw
#export FFTW_ROOT=$OLCF_FFTW_ROOT
#module load cmake
#module load boost
#module load cuda
#module load python/3.6.6-anaconda3-5.3.0

