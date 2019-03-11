#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define true 1
#define false 0

typedef double real;
typedef int bool;

real* mk_1D_array(size_t n, bool zero);
real** mk_2D_array(size_t n1, size_t n2, bool zero);
void transpose(real** bt, real** b, size_t m);
real rhs(real x, real y);

/* Functions implemented in FORTRAN in fst.f and called from C.  The trailing
underscore comes from a convention for symbol names, called name mangling:
if can differ with compilers. */
void fst_(real* v, int* n, real* w, int* nn);
void fstinv_(real* v, int* n, real* w, int* nn);
