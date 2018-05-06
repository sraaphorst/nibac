# nauty22

This is a very stripped down, slightly modified, bare bones verson of Brendan McKay's `nauty` library, only containing the bare essentials for `nibac`. It is configured using GNU autotools and thus, until this project has been updated, can be built manually using:

```
./configure
make
```

`nibac` will need access to the `nauty.h` header and the `nauty.o`, `nautil.o`, and `naugraph.o` files. Configuration as to where the compiler can locate them is found in [`src/Makefile`](../../src/Makefile) and, for now, must be configured by hand.

`nauty` is released under the Apache License 2.0, and more information can be found here:

http://users.cecs.anu.edu.au/~bdm/nauty
