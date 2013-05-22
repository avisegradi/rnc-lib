Random Linear Network Coding Library
====================================

rnc-lib is a library implementing random network coding over the finite fields
GF(256) and GF(65536). The operations are implemented using discrete logarithm
tables instead of generic finite field operations, ensuring that these
operations can be performed in constant time.

The main goals of this library are practicality and speed. It is implemented in
C/C++, and is highly optimized (inline functions, cache optimization for matrix
operations, multi-threading). The field size is a compile time information; this
reduces the number of indirections on memory access when performing
operations. By default, the 16 bit finite field is used, as it performs almost
twice as fast than the 8 bit version, and it greatly reduces the probability of
encountering a singular matrix. Bigger finite fields are not feasible to be
implemented with discrete logarithm tables. On an Intel i5-2410M CPU, using the
16 bit finite field, this library can encode/decode—in memory—a 128 MB file cut
into 32 blocks in 4.55 seconds (28 MB/s).

The project can be found on GitHub:
        https://github.com/avisegradi/rnc-lib

License
-------

    The Random Network Coding Library is released under the Apache 2.0 license:

        http://www.apache.org/licenses/LICENSE-2.0

Installation
------------

General installation instructions can be found in the INSTALL file provided in
this package.

The `configure' shell script accepts the following options beside those
described in INSTALL:

### --with-q256

The finite field (F_q) used is a compile time parameter. By default, the 16 bit
finite field (q=65536) is used. Specify --with-q256 to compile the libraries
with the 8 bit finite field.

### --with-tests=ARG

This option will be used in the early phases of development, and is subject to
change or removal.

Determines whether unit tests must be compiled, and that which rnc-lib libraries
are they to be built against.

ARG <- { no, yes, compiled, installed }
`yes' is equivalent to `compiled'.
When this option is not specified, the default is `compiled'.

no        => Unit tests will not be built.
compiled  => Unit tests will be built against the rnc-lib libraries found in the
             build directory.
installed => Unit tests will be built against installed libraries. I.e.: the
             output of `pkg-config --XXX rnc-1.0` will be used. This will be
             used to test installation in the early phases of development.
