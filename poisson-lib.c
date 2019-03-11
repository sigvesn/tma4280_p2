#include "poisson-lib.h"

/*
 * This function is used for initializing the right-hand side of the equation.
 * Other functions can be defined to swtich between problem definitions.
 */

real rhs(real x, real y)
{
    return 1;
}

/*
 * Write the transpose of b a matrix of R^(m*m) in bt.
 * In parallel the function MPI_Alltoallv is used to map directly the entries
 * stored in the array to the block structure, using displacement arrays.
 */

void transpose(real** bt, real** b, size_t m)
{
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < m; j++) {
            bt[i][j] = b[j][i];
        }
    }
}

/*
 * The allocation of a vectore of size n is done with just allocating an array.
 * The only thing to notice here is the use of calloc to zero the array.
 */

real* mk_1D_array(size_t n, bool zero)
{
    if (zero) {
        return (real*)calloc(n, sizeof(real));
    }
    return (real*)malloc(n * sizeof(real));
}

/*
 * The allocation of the two-dimensional array used for storing matrices is done
 * in the following way for a matrix in R^(n1*n2):
 * 1. an array of pointers is allocated, one pointer for each row,
 * 2. a 'flat' array of size n1*n2 is allocated to ensure that the memory space
 *   is contigusous,
 * 3. pointers are set for each row to the address of first element.
 */

real** mk_2D_array(size_t n1, size_t n2, bool zero)
{
    // 1
    real** ret = (real**)malloc(n1 * sizeof(real*));

    // 2
    if (zero) {
        ret[0] = (real*)calloc(n1 * n2, sizeof(real));
    } else {
        ret[0] = (real*)malloc(n1 * n2 * sizeof(real));
    }

    // 3
    for (size_t i = 1; i < n1; i++) {
        ret[i] = ret[i - 1] + n2;
    }
    return ret;
}
