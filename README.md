### Source code for a parallel implementation of a solver for the two dimensional Poisson problem with homogeneous Dirichlet boundary conditions.

To run on Idun first load corresponding modules by running

```bash
./load_modules.sh
```

Then to build the project:

```bash
mkdir build && cd build
cmake ..
make
```

The main program is a parallel implementation of Einar M. Rønquist serial
poisson program. It uses the forttran implemented `fst` and `fstinv`
functions from fst.f, but has a lot of C code has been refactored into
poisson-lib.c

Scripts for scheduling the program for a variety of parameter
configurations, and for generating plots from these results can be found
in gen_plots/.
