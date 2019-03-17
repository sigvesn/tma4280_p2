#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define PI 3.14159265358979323846
#define true 1
#define false 0

typedef double real;
typedef int bool;

int argparse(int argc, char** argv, int* n, int* m, int* nn, real* h, int* size, int* rank, double* time);
void finalize(double u_max, double e_max, double start_time, int rank, int m);

real* mk_1D_array(size_t n, bool zero);
real** mk_2D_array(size_t n1, size_t n2, bool zero);
void transpose(real** bt, real** b, int* ncols, int* displ, int size, int rank);
real rhs(real x, real y);
real u(real x, real y);
void gen_limits(int* ncols, int* displ, int rank, int size, int m);

/* Functions implemented in FORTRAN in fst.f and called from C.  The trailing
underscore comes from a convention for symbol names, called name mangling:
if can differ with compilers. */
void fst_(real* v, int* n, real* w, int* nn);
void fstinv_(real* v, int* n, real* w, int* nn);
