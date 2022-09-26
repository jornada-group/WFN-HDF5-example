// See:
// https://github.com/HDFGroup/hdf5/blob/develop/examples/h5_read.c
// https://portal.hdfgroup.org/display/HDF5/Lite


#include <iostream>
#include <fstream>
#include <complex>
using namespace std;

#include "hdf5.h"
#include "hdf5_hl.h"


void get_dset_shape(hid_t file, const char *dset_name)
{
    int ndims;
    hsize_t *dims;

    // Read number of dimensions
    H5LTget_dataset_ndims(file, dset_name, &ndims);
    cout << "Rank of array " << dset_name << ": " << ndims << endl;
    dims = new hsize_t[ndims];

    // Read shape of array
    H5LTget_dataset_info(file, dset_name, dims, NULL, NULL);
    cout << "Dimensions:";
    for (int i=0; i<ndims; ++i){
        cout << " " << dims[i];
    }
    cout << endl;
    cout << endl;
}


int main(void)
{
    int nrk, *ngk, gvecs_ndims, nb, nspin, nspinor, ns;
    hsize_t *gvecs_dims;

    // Open file
    hid_t file = H5Fopen("WFNq.h5", H5F_ACC_RDONLY, H5P_DEFAULT);

    // Read number of spin components
    H5LTread_dataset_int(file, "/mf_header/kpoints/nspin", &nspin);
    cout << "Number of spin components: " << nspin << endl;

    // Read number of spinor components
    H5LTread_dataset_int(file, "/mf_header/kpoints/nspinor", &nspinor);
    cout << "Number of spinor components: " << nspinor << endl;

    // BGW eventually cares about ns := nspin*nspinor, which is either 1 or 2
    ns = nspin * nspinor;

    // Read number of bands
    H5LTread_dataset_int(file, "/mf_header/kpoints/mnband", &nb);
    cout << "Number of bands: " << nb << endl;

    // Read number of k-points
    H5LTread_dataset_int(file, "/mf_header/kpoints/nrk", &nrk);
    cout << "Number of k-points: " << nrk << endl;

    // Read array with number of G-vectors per k-point
    ngk = new int[nrk];
    H5LTread_dataset_int(file, "/mf_header/kpoints/ngk", ngk);
    cout << "Number of g-vectors per k-point: " << endl;
    for (int ik=0; ik<nrk; ++ik){
        cout << "- " << ngk[ik] << endl;
    }
    cout << endl;

    // Read rank of array /wfns/gvecs
    get_dset_shape(file, "/wfns/gvecs");

    // Read rank of array /wfns/coeffs
    get_dset_shape(file, "/wfns/coeffs");


    // G-vectors associated with each k-point
    hid_t dset_g = H5Dopen2(file, "/wfns/gvecs", H5P_DEFAULT);
    // Pointer to the memory region in the file
    hid_t dspace_g = H5Dget_space(dset_g);

    // WFN coefficients associated with each k-point
    hid_t dset_c = H5Dopen2(file, "/wfns/coeffs", H5P_DEFAULT);
    // Pointer to the memory region in the file
    hid_t dspace_c = H5Dget_space(dset_c);

    cout << "Dumping wavefunctions" << endl;
    // Offset in Gs associated with each k-point ik.
    // We start without any offset.
    int ng_offset = 0;
    for (int ik=0; ik<nrk; ++ik) {
        const int ng_ik = ngk[ik];

        // G-vectors array is of shape (ngk,3)
        int gvecs[ng_ik*3];

        // How much to offset the memory we are reading from file?
        hsize_t offset_g[2] = {(hsize_t) ng_offset, 0};
        // How many entries to read?
        hsize_t count_g[2] = {(hsize_t) ng_ik, 3};

        // Pointer to the memory region in the buffer
        hid_t memspace_g = H5Screate_simple(2, count_g, NULL);
        // Select the section (hyperslab) of the gvecs dataset to read
        H5Sselect_hyperslab(dspace_g, H5S_SELECT_SET, offset_g, NULL, count_g, NULL);
        H5Dread(dset_g, H5T_STD_I32LE, memspace_g, dspace_g, H5P_DEFAULT, gvecs);
	// Always close whatever you open/create
        H5Sclose(memspace_g);

        // Coefficients is of shape (nb,ngk)
        complex<double> cg[nb*ng_ik];

        // How much to offset the memory we are reading from file?
        hsize_t offset_c[4] = {0, 0, (hsize_t) ng_offset, 0};
        // How many entries to read?
        hsize_t count_c[4] = {(hsize_t) nb, (hsize_t) ns, (hsize_t) ng_ik, 2};

        // Pointer to the memory region in the buffer
        hid_t memspace_c = H5Screate_simple(4, count_c, NULL);
        // Select the section (hyperslab) of the coeffs dataset to read
        H5Sselect_hyperslab(dspace_c, H5S_SELECT_SET, offset_c, NULL, count_c, NULL);
        H5Dread(dset_c, H5T_IEEE_F64LE, memspace_c, dspace_c, H5P_DEFAULT, cg);
	// Always close whatever you open/create
        H5Sclose(memspace_c);


        char fname[64];

        sprintf(fname, "gvecs_ik_%02d.dat", ik);
        FILE *fp = fopen(fname, "w");
        for (int ig=0; ig<ng_ik; ++ig) {
            fprintf(fp, "%4d %4d %4d\n", 
                        gvecs[3*ig], gvecs[3*ig+1], gvecs[3*ig+2]);
        }
        fclose(fp);

	for (int ib=0; ib<nb; ++ib) {
            sprintf(fname, "coeffs_ik_%02d_ib_%04d.dat", ik, ib);
            FILE *fp = fopen(fname, "w");
            for (int ig=0; ig<ng_ik; ++ig) {
                fprintf(fp, "%13.6e %13.6e\n", 
                            cg[ib*ng_ik*ns + ig].real(), cg[ib*ng_ik*ns + ig].imag());
            }
            fclose(fp);
	}

        ng_offset += ng_ik;
    }

    H5Sclose(dspace_g);
    H5Dclose(dset_g);
    H5Fclose(file);

    return 0;
}
