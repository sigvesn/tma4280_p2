#!/bin/bash
# Uses the job.sh template configure and run a sbatch job for the './main'
# program where this script is run from for the following parameters:
#   processors = 1,2,4,8
#   n = 2^k, for k = 1,2,...,24
# All results are put in the plotdata folder


#setup folders or cleanup
# mkdir -p plot
# mkdir -p plotdata
# for f in plot*/* ; do
#     rm $f
# done

echo Generating plotdata...

# get absolute path to this script
DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
JOB=$DIR/job.sh

# Running with different combinations of threads and processes with p*t= 36

ix=0
function add {
    ts[$ix]=$1
    ps[$ix]=$2
    ns[$ix]=$3
    let ix++
}

add 12 1 3
add 6  2 3
add 4  3 3
add 3  4 3
add 2  6 3

add 18 1 2
add 9  2 2
add 6  3 2
add 3  6 2
add 2  9 2

n=$(echo 2^14 | bc)

for i in ${!ns[*]}; do
        nodes=${ns[$i]}
        p=$(( ${ps[$i]} * $nodes ))
        t=${ts[$i]}

        name="t${t}_p${p}"
        echo $name = $(($p * $t))
        cp $JOB plot/${name}.sh

        sed -i "s/OUT/${name}.out/g"    plot/${name}.sh
        sed -i "s/JOB/${name}/g"        plot/${name}.sh
        sed -i "s/NODES/$nodes/g"       plot/${name}.sh
        sed -i "s/MPI_PROS/${ps[$i]}/g" plot/${name}.sh
        sed -i "s/THREADS/${ts[$i]}/g"  plot/${name}.sh
        sed -i "s/NUMBER/$n/g"          plot/${name}.sh

        # sbatch plot/${name}.sh
        echo $name
done


#run for timing results

# for i in $(seq 8 14); do
#     ns[i]=$(echo 2^$i | bc)
# done
#
# for i in $(seq 0 5); do
#     ps[i]=$(echo 2^$i | bc)
# done
#
# threads=1
# ps[0]=1
#
# for n in ${ns[*]}; do
#     for p in ${ps[*]}; do
#         if [ $p -lt 20 ]; then
#             nodes=1
#         else
#             nodes=2
#             p=$(($p/2))
#         fi
#
#         name="n${n}_p$(($nodes * ${p}))"
#         cp $JOB plot/${name}.sh
#
#         sed -i "s/OUT/${name}.out/g"  plot/${name}.sh
#         sed -i "s/JOB/${name}/g"      plot/${name}.sh
#         sed -i "s/NODES/$nodes/g"     plot/${name}.sh
#         sed -i "s/MPI_PROS/$p/g"      plot/${name}.sh
#         sed -i "s/THREADS/$threads/g" plot/${name}.sh
#         sed -i "s/NUMBER/$n/g"        plot/${name}.sh
#
#         sbatch plot/${name}.sh
#     done
# done
