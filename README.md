casmi
=====

This project is divided into two parts. The first part consists of a compiler
frontend for CASM. The frontend is responsible for the parsing and type-checking
of CASM programs. The second part is an interpreter for CASM, which provides a
concrete as well as a symbolic execution mode.

This is the first implementation of CASM released under an open source license
(it is released under the 3 clause BSD license; see LICENSE file).
The frontend can be used without proprietary dependencies, but the interpreter 
uses a closed source implementation for managing update sets. At the moment,
building the interpreter without this closed source dependency will not work.
There is an open source implementation of this dependency on the horizon however.


Build CASMI
-----------------------------

To build `casmi` you'll need

* flex (>= 2.5.0)
* bison (>= 3.0)
* cmake (>= 2.6)
* closed source implementation of the update set


The `cmake` build system is used to build `casmi`. For convenience there
exists a plain `Makefile` in the root directory of the project, that performs
an out-of-source build (using a `build` directory). To build `casmi`, use

```
$ cd path/to/casmi
$ make casmi
```

To run the test suite use

```
$ cd path/to/casmi
$ make check
```

If you want to take full control of the build process, feel free to use 
`cmake` directly!
