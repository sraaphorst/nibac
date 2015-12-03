# nibac
C++ Nonisomorphic Branch-and-Cut

This is the code that I used to generate results for my Master's Thesis in computer science.
It is based in part on a series of papers by Francois Margot in order to produce a branch-and-cut search / generation algorithm where the symmetry group of the problem is either specified or determined, and used to prune nodes from the search space so that only one solution per isomorphism class (i.e. one orbit per the partition of solutions as induced by the symmetry group) is produced.

This code is messy and disorganized. Use at your own risk.
It requires the use of an external LP solver in order to solve LPs at each node. Support is currently provided for both (older versions of) the commercial CPLEX and the free, open source GLPK.

The core code for nibac is contained in core.
Examples using nibac are contained in examples.
