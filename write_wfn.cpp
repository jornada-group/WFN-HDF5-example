// See:
// https://github.com/HDFGroup/hdf5/blob/develop/examples/h5_read.c
// https://portal.hdfgroup.org/display/HDF5/Lite


#include <iostream>
#include <fstream>
#include <complex>
using namespace std;

#include "hdf5.h"
#include "hdf5_hl.h"


int main(void)
{
    char fname[64];
    int nrk, *ngk, gvecs_ndims, nb, nspin, nspinor, ns;
    hsize_t *gvecs_dims;

    {
        cout << "Reading header" << endl;
        FILE *fp = fopen("header.dat", "r");
        fscanf(fp, "nrk: %d\n", &nrk);
        fscanf(fp, "nspin: %d\n", &nspin);
        fscanf(fp, "nspinor: %d\n", &nspinor);
        ns = nspin * nspinor;
        fscanf(fp, "nb: %d\n", &nb);
        fscanf(fp, "ngk:\n");
    	ngk = new int[nrk];
        for (int ik=0; ik<nrk; ++ik) {
            fscanf(fp, "%d\n", &ngk[ik]);
        }
        fclose(fp);
    }

    hid_t file = H5Fcreate("WFN_new.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    hsize_t ngktot = 0;
    for (int ik=0; ik<nrk; ++ik) ngktot += ngk[ik];
    cout << "ngktot: " << ngktot << endl;

    // Create /wfns group
    hid_t group = H5Gcreate2(file, "/wfns", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Gclose(group);

    // Create /wfns/gvecs dataset
    hsize_t dims_g[] = {ngktot, 3};
    hid_t dspace_g = H5Screate_simple(2, dims_g, NULL);
    hid_t dset_g = H5Dcreate2(file, "/wfns/gvecs", H5T_STD_I32LE, dspace_g, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);


    // Create /wfns/coeffs dataset
    hsize_t dims_c[] = {(hsize_t)nb, (hsize_t)ns, (hsize_t)ngktot, 2};
    hid_t dspace_c = H5Screate_simple(4, dims_c, NULL);
    hid_t dset_c = H5Dcreate2(file, "/wfns/coeffs", H5T_IEEE_F64LE, dspace_c, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    cout << "Reading and writing wavefunctions" << endl;
    // Offset in Gs associated with each k-point ik.
    // We start without any offset.
    int ng_offset = 0;
    for (int ik=0; ik<nrk; ++ik) {
        const int ng_ik = ngk[ik];
        char fname[64];
        cout << ng_ik << endl;

        // G-vectors array is of shape (ngk,3)
        int gvecs[ng_ik*3];

        // Read G-vectors
        sprintf(fname, "gvecs_ik_%02d.dat", ik);
        FILE *fp = fopen(fname, "r");
        for (int ig=0; ig<ng_ik; ++ig) {
            fscanf(fp, "%4d %4d %4d\n", 
                        &gvecs[3*ig], &gvecs[3*ig+1], &gvecs[3*ig+2]);
        }
        fclose(fp);

        // How much to offset the memory we are writing to file?
        hsize_t offset_g[2] = {(hsize_t)ng_offset, 0};
        // How many entries to write?
        hsize_t count_g[2] = {(hsize_t)ng_ik, 3};

        // Pointer to the memory region in the buffer
        hid_t memspace_g = H5Screate_simple(2, count_g, NULL);
        // Select the section (hyperslab) of the gvecs dataset to read
        H5Sselect_hyperslab(dspace_g, H5S_SELECT_SET, offset_g, NULL, count_g, NULL);
        H5Dwrite(dset_g, H5T_STD_I32LE, memspace_g, dspace_g, H5P_DEFAULT, gvecs);
        // Always close whatever you open/create
        H5Sclose(memspace_g);


        // Coefficients is of shape (nb,ngk)
        complex<double> cg[nb*ng_ik];

        // Read coefficients
        for (int ib=0; ib<nb; ++ib) {
            sprintf(fname, "coeffs_ik_%02d_ib_%04d.dat", ik, ib);
            FILE *fp = fopen(fname, "r");
            double re, im;
            char buf[256];
            for (int ig=0; ig<ng_ik; ++ig) {
                fscanf(fp, "%lf %lf\n", &re, &im);
                cg[ib*ng_ik*ns + ig] = complex<double>(re, im);
            }
            fclose(fp);
        }

        // How much to offset the memory we are reading from file?
        hsize_t offset_c[] = {0, 0, (hsize_t)ng_offset, 0};
        // How many entries to read?
        hsize_t count_c[] = {(hsize_t)nb, (hsize_t)ns, (hsize_t)ng_ik, 2};

        // Pointer to the memory region in the buffer
        hid_t memspace_c = H5Screate_simple(4, count_c, NULL);
        // Select the section (hyperslab) of the coeffs dataset to read
        H5Sselect_hyperslab(dspace_c, H5S_SELECT_SET, offset_c, NULL, count_c, NULL);
        H5Dwrite(dset_c, H5T_IEEE_F64LE, memspace_c, dspace_c, H5P_DEFAULT, cg);
        // Always close whatever you open/create
        H5Sclose(memspace_c);

        ng_offset += ng_ik;
    }

    H5Dclose(dset_g);
    H5Sclose(dspace_g);

    H5Dclose(dset_c);
    H5Sclose(dspace_c);

    H5Fclose(file);

    return 0;
}
