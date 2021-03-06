#include "poisson-lib.h"

int main(int argc, char** argv)
{
    int n, m, nn, size, rank;
    double start_time;
    real h;

    int ret = argparse(argc, argv, &n, &m, &nn, &h, &size, &rank, &start_time);
    if (ret < 3)
        return ret;

    /* Grid points generated with constant mesh size on both x- and y-axis. */
    real* grid = mk_1D_array(n + 1, false);
#   pragma omp parallel for schedule(static)
    for (int i = 0; i < n + 1; i++)
        grid[i] = i * h;

    /* The diagonal of the eigenvalue matrix of T is set with the eigenvalues */
    real* diag = mk_1D_array(m, false);
#   pragma omp parallel for schedule(static)
    for (int j = 0; j < m; j++)
        diag[j] = 2.0 * (1.0 - cos((j + 1) * PI / n));

    int* counts = calloc(size, sizeof(int));
    int* displs = calloc(size + 1, sizeof(int));
    gen_limits(counts, displs, rank, size, m);

    /* Matrices b and bt used for storing value of G, ~G^T, U, ~U^T */
    real** b = mk_2D_array(counts[rank], m, false);
    real** bt = mk_2D_array(counts[rank], m, false);

    /* Holds coefficients of the Discrete Sine Transform */
    real** z = mk_2D_array(counts[rank], nn, true);

    /* Initialize the right hand side data for a given rhs function (G). */
    /* We have to add the ranks displacement to the grid calculation */
    for (int i = 0; i < counts[rank]; i++)
#       pragma omp parallel for schedule(static)
        for (int j = 0; j < m; j++)
            b[i][j] = h * h * rhs(grid[displs[rank] + i + 1], grid[j + 1]);

    /* Compute ~G^T = S^-1 * (S * G)^T  (Chapter 9. page 101 step 1) */
    for (int i = 0; i < counts[rank]; i++)
        fst_(b[i], &n, z[i], &nn);

    transpose(bt, b, counts, displs, size, rank);

    for (int i = 0; i < counts[rank]; i++)
        fstinv_(bt[i], &n, z[i], &nn);

    /* Solve  λ * ~U = ~G               (Chapter 9. page 101 step 2) */
    /* We have to add the ranks displacement to find the right diag value */
    for (int i = 0; i < counts[rank]; i++)
#       pragma omp parallel for schedule(static)
        for (int j = 0; j < m; j++)
            bt[i][j] = bt[i][j] / (diag[displs[rank] + i] + diag[j]);

    /* Compute U = S^-1 * (S * ~U^T)    (Chapter 9. page 101 step 3) */
    for (int i = 0; i < counts[rank]; i++)
        fst_(bt[i], &n, z[i], &nn);

    transpose(b, bt, counts, displs, size, rank);

    for (int i = 0; i < counts[rank]; i++)
        fstinv_(b[i], &n, z[i], &nn);

    /* Compute maximal value of solution for convergence analysis in L^∞-norm. */
    double u_max = 0.0;
    for (int i = 0; i < counts[rank]; i++)
        for (int j = 0; j < m; j++)
            u_max = u_max > fabs(b[i][j]) ? u_max : fabs(b[i][j]);

    double global_u_max;
    MPI_Reduce(&u_max, &global_u_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    /* Compute maximal error between points between our solution and the analytical solution */
    double e_max = 0;
    for (int i = 0; i < counts[rank]; i++)
        for (int j = 0; j < m; j++) {
            double analytical = u(grid[displs[rank] + i + 1], grid[j + 1]);
            double err = fabs(b[i][j] - analytical);
            e_max = e_max > err ? e_max : err;
        }

    double global_e_max;
    MPI_Reduce(&e_max, &global_e_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    finalize(global_u_max, global_e_max, start_time, rank, m, size);
    free(counts);
    free(displs);

    return 0;
}
