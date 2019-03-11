#include "poisson-lib.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage:\n");
        printf("  poisson n\n\n");
        printf("Arguments:\n");
        printf("  n: the problem size (must be a power of 2)\n");
        return 1;
    }

    /*
     *  The equation is solved on a 2D structured grid and homogeneous Dirichlet
     *  conditions are applied on the boundary:
     *  - the number of grid points in each direction is n+1,
     *  - the number of degrees of freedom in each direction is m = n-1,
     *  - the mesh size is constant h = 1/n.
     */
    int n = atoi(argv[1]);
    if ((n & (n - 1)) != 0) {
        printf("n must be a power-of-two\n");
        return 2;
    }

    int m = n - 1;
    real h = 1.0 / n;

    /*
     * Grid points are generated with constant mesh size on both x- and y-axis.
     */
    real* grid = mk_1D_array(n + 1, false);
    for (int i = 0; i < n + 1; i++) {
        grid[i] = i * h;
    }

    /*
     * The diagonal of the eigenvalue matrix of T is set with the eigenvalues
     * defined Chapter 9. page 93 of the Lecture Notes.
     * Note that the indexing starts from zero here, thus i+1.
     */
    real* diag = mk_1D_array(m, false);
    for (int i = 0; i < m; i++) {
        diag[i] = 2.0 * (1.0 - cos((i + 1) * PI / n));
    }

    /*
     * Allocate the matrices b and bt which will be used for storing value of
     * G, \tilde G^T, \tilde U^T, U as described in Chapter 9. page 101.
     */
    real** b = mk_2D_array(m, m, false);
    real** bt = mk_2D_array(m, m, false);

    /*
     * This vector will holds coefficients of the Discrete Sine Transform (DST)
     * but also of the Fast Fourier Transform used in the FORTRAN code.
     * The storage size is set to nn = 4 * n, look at Chapter 9. pages 98-100:
     * - Fourier coefficients are complex so storage is used for the real part
     *   and the imaginary part.
     * - Fourier coefficients are defined for j = [[ - (n-1), + (n-1) ]] while
     *   DST coefficients are defined for j [[ 0, n-1 ]].
     * As explained in the Lecture notes coefficients for positive j are stored
     * first.
     * The array is allocated once and passed as arguments to avoid doings
     * reallocations at each function call.
     */
    int nn = 4 * n;
    real* z = mk_1D_array(nn, false);

    /*
     * Initialize the right hand side data for a given rhs function.
     *
     */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            b[i][j] = h * h * rhs(grid[i + 1], grid[j + 1]);
        }
    }

    /*
     * Compute \tilde G^T = S^-1 * (S * G)^T (Chapter 9. page 101 step 1)
     * Instead of using two matrix-matrix products the Discrete Sine Transform
     * (DST) is used.
     * The DST code is implemented in FORTRAN in fst.f and can be called from C.
     * The array zz is used as storage for DST coefficients and internally for
     * FFT coefficients in fst_ and fstinv_.
     * In functions fst_ and fst_inv_ coefficients are written back to the input
     * array (first argument) so that the initial values are overwritten.
     */
    for (int i = 0; i < m; i++) {
        fst_(b[i], &n, z, &nn);
    }
    transpose(bt, b, m);
    for (int i = 0; i < m; i++) {
        fstinv_(bt[i], &n, z, &nn);
    }

    /*
     * Solve Lambda * \tilde U = \tilde G (Chapter 9. page 101 step 2)
     */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            bt[i][j] = bt[i][j] / (diag[i] + diag[j]);
        }
    }

    /*
     * Compute U = S^-1 * (S * Utilde^T) (Chapter 9. page 101 step 3)
     */
    for (int i = 0; i < m; i++) {
        fst_(bt[i], &n, z, &nn);
    }
    transpose(b, bt, m);
    for (int i = 0; i < m; i++) {
        fstinv_(b[i], &n, z, &nn);
    }

    /*
     * Compute maximal value of solution for convergence analysis in L_\infty
     * norm.
     */
    double u_max = 0.0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            u_max = u_max > fabs(b[i][j]) ? u_max : fabs(b[i][j]);
        }
    }

    printf("u_max = %e\n", u_max);

    return 0;
}
