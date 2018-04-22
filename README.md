# nibac
C++ Nonisomorphic Branch-and-Cut

This is the code that I used to generate results for my Master's Thesis in computer science.
It is based in part on a series of papers by Francois Margot in order to produce a branch-and-cut search / generation algorithm where the symmetry group of the problem is either specified or determined, and used to prune nodes from the search space so that only one solution per isomorphism class (i.e. one orbit per the partition of solutions as induced by the symmetry group) is produced.

This code is messy and disorganized. Use at your own risk. It still compiles just fine, provided you set the paths properly in `core/Makefile`. After installing `glpk` and `nauty` (see below), you should `make` and then `make install`. The examples should then properly compile.

It requires the use of an external LP solver in order to solve LPs at each node. Support is currently provided for both (older versions of) the commercial CPLEX and the free, open source GLPK. I recommend you use GLPK 4.47, as that was the API under which this code was written, and significant, incompatible changes have been introduced into GLPK in later versions.

You can obtain it here:
http://ftp.gnu.org/gnu/glpk/glpk-4.47.tar.gz

Brendan McKay's `nauty` is also necessary. We recommend version 22, which can be found here:
http://users.cecs.anu.edu.au/~bdm/nauty/nauty22.tar.gz

There may be an issue for some compilers with regards to this version of `nauty` due to the use of a function called `getline`, which interferes with a stdlib `getline`. If this is the case, the `getline` in the `nauty` code should be renamed (e.g. `getline2`).

The core code for nibac is contained in core.
Examples using nibac are contained in examples.
