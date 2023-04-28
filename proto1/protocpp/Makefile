# definitions
PB_CXX_FLAGS := $(shell pkg-config --cflags protobuf) 
PB_LD_FLAGS  := $(shell pkg-config --libs protobuf)
#CXX_FLAGS := -fPIC -g -pedantic -Wall -Wextra -march=native -ggdb3 $(PB_CXX_FLAGS)
CXX_FLAGS := -fPIC -pedantic -Wall -Wextra -march=native -ggdb3 $(PB_CXX_FLAGS)
LD_FLAGS := -shared $(PB_LD_FLAGS)

# following rules are useful to compile only those files that are generated from 
# protofiles
# http://stackoverflow.com/questions/8096015/creating-a-simple-make-file-to-create-a-shared-library-whats-wrong-with-this-m

PBSRCS := $(wildcard ../*.proto)
PBOBJS1 := $(PBSRCS:.proto=.pb.o)
PBOBJS := $(patsubst ../%.pb.o, %.pb.o, $(PBOBJS1))

# the second method is simple..
#PBSRCS = $(shell echo *.pb.cc)
#PBHDRS = $(shell echo *.h)
#PBOBJS = $(PBSRCS:.pb.cc=.pb.o)

BINS := libpblib.so

$(BINS): $(PBOBJS)
	$(CXX) $(LD_FLAGS) -o $@ $(PBOBJS) 
	@echo Linking Finished ----------------------------------------------------------

all: $(BINS)

# Rule to build an object file from .cc file
%.pb.o: %.pb.cc %.pb.h
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
	@echo Compilation Finished ----------------------------------------------------------

clean:
	rm -rf $(BINS) $(PBOBJS)

show:
	@echo $(PBSRCS)
	@echo $(PBOBJS)
	@echo CXX Flags $(CXX_FLAGS)
	@echo LD Flags $(LD_FLAGS)
	


