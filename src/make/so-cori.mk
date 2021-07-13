
CXX:=CC
#F77:=ftn
#CXX_SERIAL:=CC
#EINSPLINE_LIBS := -L${HOME}/local/lib -leinspline
#EINSPLINE_INCLUDE := -I${HOME}/local/include/einspline
#CXXFLAGS:=-h new_for_init -fast -D__STL_NO_EXCEPTION_HEADER
#CXXFLAGS += -xHost -ipo -no-prec-div
CXXFLAGS := -axMIC-AVX512,AVX -fno-alias -funroll-loops
CXXFLAGS += -DUSE_MPI
CXXFLAGS += -std=c++11
CXXFLAGS += -DUSE_BLAS -DUSE_LAPACK
CXXFLAGS += -DUSE_RESTRICT $(INCLUDEPATH)
#CXXFLAGS += -I/global/u1/m/mcbennet/local/include/einspline
#CXXFLAGS += -L/global/u1/m/mcbennet/local/lib -leinspline
CXXFLAGS += $(MKL) $(MKL_INC) -mkl
DEBUG:= -DNO_RANGE_CHECKING -DNDEBUG -DDEBUG_WRITE
######################################################################
# This is the invokation to generate dependencies
DEPENDMAKER:=CC -MM $(INCLUDEPATH)

