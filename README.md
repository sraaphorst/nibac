# nibac

NOTE: This project is being updated and as such, will likely be unstable until stated otherwise!

# C++ Nonisomorphic Branch-and-Cut

This is the code that I used to generate results for my Master's Thesis in computer science.
It is based in part on a series of papers by Francois Margot in order to produce a branch-and-cut search / generation algorithm where the symmetry group of the problem is either specified or determined, and used to prune nodes from the search space so that only one solution per isomorphism class (i.e. one orbit per the partition of solutions as induced by the symmetry group) is produced.

# Warning

While I am trying to clean it up for public distribution, this code is messy and disorganized. Use at your own risk.

It still compiles, provided you set the paths properly in `src/Makefile`. After installing `glpk` and `nauty` (see below), you should `make` and then `make install`. The examples should then properly compile. While `nauty` is not yet built into the final library, the plan is to include it.

Right now things might break as I am migrating from a plain makefile to `CMake` for configuration of the project and to include `nauty` in the final library to facilitate usage. I am working in CLion and thus, there are CLion project files included in the repository.

# LP Solver

nibac requires the use of an external LP solver in order to solve LPs at each node. Support is currently provided for both (older versions of) the commercial CPLEX and the free, open source GLPK. I recommend you use GLPK 4.47, as that was the API under which this code was written, and significant, incompatible changes have been introduced into GLPK in later versions.

You can obtain it here:
http://ftp.gnu.org/gnu/glpk/glpk-4.47.tar.gz

I intend to write a plugin for `Clp`, the COIN-OR LP solver. More information can be found here:

https://projects.coin-or.org/Clp

It was last tested with CPLEX 7; as I no longer have access to CPLEX and it is a commercial application, CPLEX support for updated versions will likely not be provided.

# nauty

Brendan McKay's `nauty` (released under the Apache License 2.0) is also necessary. We recommend version 22, and as it doesn't install nicely and comes with many features unnecessary for nibac, we include a heavily pared down bare-bones version in [`extern_src/nauty22`](extern_src/nauty22/README.md) that, when the update to this project is completed, should be configured, built, and bundled automatically in the final nibac library. In the interim, you will have to provide access to the headers and `.o` files by modifying [`src/Makefile`](src/Makefile).

For more information about `nauty`, please visit:

http://users.cecs.anu.edu.au/~bdm/nauty

# Final notes

The core code for nibac is contained in `src`.

Examples using nibac are contained in `examples` and are not built by default.
