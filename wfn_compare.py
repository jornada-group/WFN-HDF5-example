#!/usr/bin/env python

import numpy as np
import h5py

verbose = False
verbose = True

f1 = h5py.File('WFNq.h5', 'r')
f2 = h5py.File('WFN_new.h5', 'r')

ngk = f1['mf_header/kpoints/ngk'][()]

gvecs1 = f1['wfns/gvecs'][()]
gvecs2 = f2['wfns/gvecs'][()]

coeffs1 = f1['wfns/coeffs'][()]
coeffs2 = f2['wfns/coeffs'][()]

print('G-vector error:', end=' ')
print(np.fabs(gvecs1 - gvecs2).max())
print('WFN error:', end=' ')
print(np.linalg.norm(coeffs1 - coeffs2, axis=-1).max())

if verbose:
    nk = f1['mf_header/kpoints/nrk'][()]
    nb = f1['mf_header/kpoints/mnband'][()]
    for ik in range(nk):
        ig_start = ngk[:ik].sum()
        ig_end = ig_start + ngk[ik]

        gs = np.loadtxt(f'gvecs_ik_{ik:02d}.dat')
        gs1 = gvecs1[ig_start:ig_end]
        gs2 = gvecs2[ig_start:ig_end]
        print('G-vectors')
        print('  Text file - orig WFN error:', end=' ')
        print(np.linalg.norm(gs - gs1, axis=-1).max())
        print('  Text file - output WFN error:', end=' ')
        print(np.linalg.norm(gs - gs2, axis=-1).max())

        print()
        print('Coefficients')
        for ib in range(nb):
            fname = f'coeffs_ik_{ik:02d}_ib_{ib:04d}.dat'
            cg = np.loadtxt(fname)

            cg1 = coeffs1[ib,0,ig_start:ig_end,:]
            cg2 = coeffs2[ib,0,ig_start:ig_end,:]
            print('  Text file - orig WFN error:', end=' ')
            print(np.linalg.norm(cg - cg1, axis=-1).max())
            print('  Text file - output WFN error:', end=' ')
            print(np.linalg.norm(cg - cg2, axis=-1).max())
        #break
        print()

