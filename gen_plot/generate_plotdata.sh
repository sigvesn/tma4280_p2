#!/bin/bash
# Uses the job.sh template configure and run a sbatch job for the './main'
# program where this script is run from for the following parameters:
#   processors = 1,2,4,8
#   n = 2^k, for k = 1,2,...,24
# All results are put in the plotdata folder


#setup folders or cleanup
mkdir -p plot
mkdir -p plotdata
for f in plot*/* ; do
    rm $f
done

for i in $(seq 8 14); do
    ns[i]=$(echo 2^$i | bc)
done

for i in $(seq 0 4); do
    ps[i]=$(echo 2^$i | bc)
done

threads=1
nodes=1


echo Generating plotdata...

# get absolute path to this script
DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
JOB=$DIR/job.sh

for n in ${ns[*]}; do
    for p in ${ps[*]}; do
        name="job_n${n}_p${p}.sh"
        cp $JOB plot/$name

        sed -i "s/OUT/${n}_p${p}.out/g" plot/$name
        sed -i "s/JOB/$name/g"          plot/$name
        sed -i "s/NODES/$nodes/g"       plot/$name
        sed -i "s/MPI_PROS/$p/g"        plot/$name
        sed -i "s/THREADS/$threads/g"   plot/$name
        sed -i "s/NUMBER/$n/g"          plot/$name

        sbatch plot/$name
    done
done
