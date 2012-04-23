OPENCL_INC = /opt/AMDAPP/include

PROJ_INC = SDK/include
PROJ_SRC = SDK

VPATH = $(PROJ_INC):$(PROJ_SRC)

GCC = g++

GCCFLAGS = -g -m64 -Wall -O3
           
GCCINCLUDES = -I$(PROJ_INC) \
              -I$(OPENCL_INC)

GCCLIBS = -lOpenCL

CPPSOURCES := $(wildcard $(PROJ_SRC)/*.cpp) $(wildcard ./*.cpp)
CPPHEADERS := $(wildcard $(PROJ_INC)/*.h) $(wildcard ./*.h)
CPPOBJECTS  = $(CPPSOURCES:.cpp=.o)

ALLOBJECTS = $(CPPOBJECTS)

EXECUTABLE = main

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(ALLOBJECTS)
	$(GCC) $(GCCLIBS) $(ALLOBJECTS) -o $@
	
-include $(ALLOBJECTS:.o=.d)

%.o: %.cpp
	$(GCC) $(GCCINCLUDES) -c $< -o $@ $(GCCFLAGS)
	
%.d: %.cpp
	@ $(GCC) -MM -MT '$*.o $@' $(GCCFLAGS) $(GCCINCLUDES) $< > $@

clean:
	rm -rf $(ALLOBJECTS) $(ALLOBJECTS:.o=.d) $(EXECUTABLE)
