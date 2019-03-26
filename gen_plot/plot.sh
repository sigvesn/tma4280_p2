#!/bin/bash
# Generates a plot from the files created by aggregate_plotdata.sh using
# gnuplot and the graph_template.plg template file.

DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"

# set terminal png nocrop enhanced font "/usr/share/fonts/dejavu/DejaVuSansMono.ttf,8" size 800,600
gnuplot << EOF
set terminal pngcairo enhanced font "TeX Gyre Pagella, 12" size 800,600
set output '/dev/null'
load '$DIR/graph_template.plg'
set output '$1.png'
replot
EOF
