#!/bin/bash
# Extracts and aggregates the data clreated by generate_plotdata.sh into one
# file each for the number of processes used in running the program.

echo "Aggregating ..."

for f in plotdata/*.out; do
    n=$(sed -n 's/^n = \([0-9]*\).*/\1/p'                               ${f})
    p=$(sed -n 's/^nprocs = \([0-9]*\).*/\1/p'                          ${f})
    e=$(sed -n 's/^error max = \([0-9].[0-9]*e[-|+][0-9]*\).*/\1/p'     ${f})
    m=$(sed -n 's/^numerical max = \([0-9].[0-9]*e[-|+][0-9]*\).*/\1/p' ${f})
    d=$(sed -n 's/^time = \([0-9].[0-9]*e[-|+][0-9]*\).*/\1/p'          ${f})

    echo $n, $p, $e, $m, $d

    if ! [ -z "${n}" ] && ! [ -z "${p}" ] && ! [ -z "${e}" ] && ! [ -z "${m}" ] && ! [ -z "${d}" ]; then
        echo $n $e $m $d >> plotdata/aggregate_p${p}.txt
    fi
done
