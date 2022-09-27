
# module load cray-hdf5-parallel
HDF5DIR = $(HDF5_DIR)

LINK = CC
#LDFLAGS = -L$(HDF5DIR)/lib -lhdf5
CXX = CC
#CXXFLAGS = -I$(HDF5DIR)/include

all: write_wfn.x read_wfn.x

write_wfn.x: write_wfn.o
	$(LINK) -o $@ $^ $(LDFLAGS)

write_wfn.o: write_wfn.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

read_wfn.x: read_wfn.o
	$(LINK) -o $@ $^ $(LDFLAGS)

read_wfn.o: read_wfn.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f *.x *.o

.PHONY: clean
