
# module load cray-hdf5-parallel
HDF5DIR = $(HDF5_DIR)

LINK = CC
#LDFLAGS = -L$(HDF5DIR)/lib -lhdf5
CXX = CC
#CXXFLAGS = -I$(HDF5DIR)/include

read_wfn.x: read_wfn.o
	$(LINK) -o $@ $^ $(LDFLAGS)

read_wfn.o: read_wfn.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f *.x *.o

.PHONY: clean
