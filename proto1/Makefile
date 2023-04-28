# flags from pkg-config tool
# All proto files
PBSRCS := $(wildcard *.proto)

# Generated files, see the pattern
PBGENS := $(PBSRCS:.proto=.pb.cc) $(PBSRCS:.proto=.pb.h) $(PBSRCS:.proto=.pb.go)

# compile a proto file using $< as the supplid argument
%.pb.cc: %.proto
	protoc --cpp_out=protocpp $<

%.pb.go: %.proto
	protoc --go_out=protogo $<

# Nothing to be done to generate the .h files
%.pb.h: %.proto
	@echo

all: $(PBGENS)
	sed -i 's/public ::google/virtual public ::google/g' protocpp/*.pb.h
	make -C protocpp all

# The generated files are not intermediates and so the
# make tool should not delete them.
.PRECIOUS: $(PBGENS)

clean:
	rm -rf protocpp/*.pb.cc protocpp/*.pb.h protocpp/pblib.so protogo/*.pb.go
	make -C protocpp clean


