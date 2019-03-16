#include "poisson-lib.h"

/*
 *  The equation is solved on a 2D structured grid and homogeneous Dirichlet
 *  conditions are applied on the boundary:
 *  - the number of grid points in each direction is n+1,
 *  - the number of degrees of freedom in each direction is m = n-1,
 *  - the mesh size is constant h = 1/n.
 *
 *  This function parses command lne arguments and inits n, m and h
 *  It also starts the time measurments and inits MPI variables
 */
int argparse(int argc, char** argv, int* n, int* m, int* nn, real* h, int* size, int* rank, double* time)
{
    if (argc < 2) {
        printf("Usage:\n");
        printf("  poisson n\n\n");
        printf("Arguments:\n");
        printf("  n: the problem size (must be a power of 2)\n");
        return 1;
    }

    *n = atoi(argv[1]);
    if ((*n & (*n - 1)) != 0) {
        printf("n must be a power-of-two\n");
        return 2;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, size);
    MPI_Comm_rank(MPI_COMM_WORLD, rank);

    *m = *n - 1;
    *h = 1.0 / (*n);
    *nn = 4 * *n;

    if (rank == 0)
        *time = MPI_Wtime();

    return 3;
}

// Finalize MPI and print results
void finalize(double u_max, double start_time, int rank, int m)
{

    if (rank == 0) {
        double duration = (MPI_Wtime() - start_time);
        printf("Duration:       %f\n", duration);
        printf("Numerical max:  %f\n", u_max);

        /* double a_max = 0.0; */
        /* for (int i = 0; i < m; i++) */
        /*     for (int j = 0; j < m; j++) */
        /*         a_max = a_max > fabs(f(i, j)) ? a_max : fabs(f(i, j)); */
        /*  */
        /* printf("Analytical max: %f\n", a_max); */
    }

    MPI_Finalize();
}

void gen_limits(int* counts, int* displs, int rank, int size, int m)
{
    //distribute colums imatrixes to mpi processes
    int part = m / size;
    for (int i = 0; i < size; i++)
        counts[i] = part;

    //columns leftover from dividing
    int overflow = m % size;
    for (int i = 1; i <= overflow; i++)
        counts[size - i]++;

    //each i specifies the displacement of the counts array element i from the start
    displs[0] = 0;
    for (int i = 1; i < size + 1; i++)
        displs[i] = displs[i - 1] + counts[i - 1];

    /* printf("counts[%d]: %d\n", rank, counts[rank]); */
    /* printf("displs[%d]: %d\n", rank + 1, displs[rank + 1]); */
}

/*
 * This function is used for initializing the right-hand side of the equation.
 * Other functions can be defined to swtich between problem definitions.
 */

real rhs(real x, real y)
{
    return 5 * PI * PI * sin(PI * x) * sin(2 * PI * y);
}

real f(real x, real y)
{
    return sin(PI * x) * sin(2 * PI * y);
}
/*
 * Write the transpose of b a matrix of R^(m*m) in bt.
 * In parallel the function MPI_Alltoallv is used to map directly the entries
 * stored in the array to the block structure, using displacement arrays.
 */

void transpose(real** bt, real** b, int* counts, int* displs, int size, int rank)
{
    // Buffers for recieving data, counterparts tob,counts, and displs
    double* recv_buf = calloc(counts[rank] * size, sizeof(double));
    int* recvcounts = calloc(size, sizeof(int));
    int* rdispls = calloc(size + 1, sizeof(int));

    // In the receiving matrix, we want to
    // recvcounts is a copy of counts, each process sendsand recieves the same amount of data since their part of the matrix is constant.
    // rdispls is the displacement of the receiving buffer from the start
    rdispls[0] = 0;
    for (int i = 1; i < size + 1; i++) {
        rdispls[i] = rdispls[i - 1] + counts[rank];
        recvcounts[i - 1] = counts[rank];
    }
    printf("recvcounts[%d]: %d\n", rank, recvcounts[rank]);
    printf("rdispls[%d]: %d\n", rank + 1, rdispls[rank + 1]);

    // for each element in the process matrix part (we filthe overflow elements from the back, so the last element will be equal to or the largest)
    for (int i = 0; i < counts[size - 1]; i++) {
        // Send the counts[rank] elements dispaced displs[rank] from b[i] to the correstpoinding recievbuffer in rank.
        if (i < counts[rank])
            MPI_Alltoallv(b[i], counts, displs, MPI_DOUBLE,
                recv_buf, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD);
        // if this process matrix part is 1 smaller han the rest, send the last element twice to avoid the reciever waiting forever (could thus be any element in b)
        else
            MPI_Alltoallv(b[i - 1], counts, displs, MPI_DOUBLE,
                recv_buf, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD);

        // Fill bt with the results from alltoallv such that bt is transposed.
        // For each column in this ranks generated part of bt, if we are in
        // range of the displacement: get the prt of the rerecv_buf required to
        // fill up bt as explained i appendix C.
        for (int r = 0; r < size; ++r)
            for (int c = 0; c < counts[rank]; ++c)
                if (displs[r] + i < displs[r + 1])
                    bt[c][displs[r] + i] = recv_buf[rdispls[r] + c];
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
