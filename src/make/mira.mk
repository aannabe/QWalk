######################################################################
# Compiler definitions for jaguar.ccs.ornl.gov (compute nodes) 
#  all compiler specific information should be declared here
CXX:=mpixlcxx
F77:=mpixlf90
EINSPLINE_LIBS := -L/home/mbennett/local/lib -leinspline
EINSPLINE_INCLUDE := -I/home/mbennett/local/include/einspline
CXXFLAGS := -g -O2
CXXFLAGS += -DUSE_MPI -DUSE_EINSPLINE -DUSE_RESTRICT -DUSE_BLAS -DUSE_LAPACK
CXXFLAGS += $(INCLUDEPATH)
CXXFLAGS:= -O3 -qstrict -qhot -qarch=qp -qtune=qp -qprefetch -qunroll=yes 
CXXFLAGS += -DUSE_BLAS -DUSE_LAPACK -DUSE_MPI ${INCLUDEPATH}
SOFTPATH:=/soft/libraries/alcf/current/xl/
BLAS_LIBS := -L/soft/compilers/ibmcmp-may2015/xlf/bg/14.1/lib64/  -L${SOFTPATH}/BLAS/lib -L${SOFTPATH}/CBLAS/lib/ -L${SOFTPATH}/LAPACK/lib -llapack -lcblas -lblas -lxlopt -lxlf90_t -lxl -lpthread -lmass 
BLAS_INCLUDE := -I${ESSL}/include -I${SOFTPATH}/CBLAS/include/
BLAS_LIBS += $(EINSPLINE_LIBS)
BLAS_INCLUDE += $(EINSPLINE_INCLUDE)
DEBUG:= -DNO_RANGE_CHECKING -DNDEBUG  
######################################################################
# This is the invokation to generate dependencies
DEPENDMAKER:=g++ -MM  $(INCLUDEPATH)

