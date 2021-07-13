######################################################################
# Compiler definitions for Linux systems
# all compiler specific information should be declared here
# sudo aptitude install libblas-dev liblapack-dev openmpi-bin openmpi-doc libopenmpi-dev

CXX:=mpicxx 
CXX_SERIAL:=g++

CXXFLAGS := -O3 -ffast-math -fomit-frame-pointer -malign-double -fstrict-aliasing -funroll-loops 
CXXFLAGS += -DUSE_BLAS -DUSE_LAPACK -DUSE_MPI -DUSE_RESTRICT  ${INCLUDEPATH} 

LAPACK_LIBS := -L/usr/lib -llapack -lblas
#LAPACK_INCLUDE := -I/usr/include/atlas

BLAS_LIBS :=  $(LAPACK_LIBS) 
#BLAS_INCLUDE := $(LAPACK_INCLUDE) 

DEBUG:= -Wall -DNO_RANGE_CHECKING -DNDEBUG -D__USE_GNU -DDEBUG_WRITE
LDFLAGS:= 

HDF_LIBS:=-L/opt/hdf5-1.8.5/lib -lhdf5
HDF_INCLUDE:=-I/opt/hdf5-1.8.5/include

#XML_LIBS:=$(shell xml2-config --libs)
#XML_INCLUDE:=$(shell xml2-config --cflags)

######################################################################
# This is the invokation to generate dependencies
DEPENDMAKER:=g++ -MM  $(INCLUDEPATH)
