######################################################################
# Compiler definitions for Theta KNL (compute nodes) (use aprun)
# all compiler specific information should be declared here

CXX:= CC
CXX_SERIAL:=g++
CXXFLAGS := -g -O2 -mkl
CXXFLAGS += -O3 -fno-alias -funroll-loops -ffast-math -fomit-frame-pointer
CXXFLAGS += -DUSE_MPI -DUSE_RESTRICT -DUSE_BLAS -DUSE_LAPACK $(INCLUDEPATH)

LAPACK_LIBS := -L$(MKLROOT)/lib/intel64 -Wl,--start-group -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -Wl,--end-group -lpthread -lm -ldl
LAPACK_INCLUDE := -I${MKLROOT}/include
BLAS_LIBS :=  $(LAPACK_LIBS)
BLAS_INCLUDE := $(LAPACK_INCLUDE)
DEBUG:= -DNO_RANGE_CHECKING -DNDEBUG

######################################################################
# This is the invokation to generate dependencies
DEPENDMAKER:=CC -MM $(INCLUDEPATH)

#Currently Loaded Modulefiles:
#  1) modules/3.2.11.1
#  2) intel/18.0.0.128
#  3) craype-network-aries
#  4) craype/2.5.15
#  5) udreg/2.3.2-6.0.7.1_5.13__g5196236.ari
#  6) ugni/6.0.14.0-6.0.7.1_3.13__gea11d3d.ari
#  7) pmi/5.0.14
#  8) dmapp/7.1.1-6.0.7.1_6.2__g45d1b37.ari
#  9) gni-headers/5.0.12.0-6.0.7.1_3.11__g3b1768f.ari
# 10) xpmem/2.2.15-6.0.7.1_5.11__g7549d06.ari
# 11) job/2.2.3-6.0.7.1_5.44__g6c4e934.ari
# 12) dvs/2.7_2.2.120-6.0.7.1_12.1__g74cb2cc4
# 13) alps/6.6.43-6.0.7.1_5.46__ga796da32.ari
# 14) rca/2.2.18-6.0.7.1_5.48__g2aa4f39.ari
# 15) atp/2.1.3
# 16) perftools-base/7.0.4
# 17) PrgEnv-intel/6.0.4
# 18) craype-mic-knl
# 19) cray-mpich/7.7.3
# 20) nompirun/nompirun
# 21) darshan/3.1.5
# 22) trackdeps
# 23) xalt
# 24) gcc/7.3.0

