#!/bin/bash
#SBATCH --job-name=JOB
#SBATCH --partition=WORKQ
#SBATCH --output=./plotdata/OUT
#SBATCH --nodes=NODES
#SBATCH --ntasks-per-node=MPI_PROS
#SBATCH --cpus-per-task=THREADS

mpirun ./poisson NUMBER

# nodes er antall fysiske noder. Hver består av 20 cores.
# ntasks-per-node er MPI prosesser per node.
# cpus-per-task er threads per prosess.
