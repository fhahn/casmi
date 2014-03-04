casmi
=====

To build `casmi` you'll need

* flex (>= 2.5.0)
* bison (>= 3.0)
* cmake (>= 2.6)


```
$ cd path/to/casmi
$ mkdir build && cd build
$ cmake ..
$ make
```

This will build the whole project and generate the following files:

* `build/bin/test_parser`
* `build/bin/unittest_runner`
* `build/lib/libparser.a`


Tests
------------------------

At the moment `ctest` is used for the integration tests.
To run the tests, use `make check`. 

To pass options to `ctest` use `make ARGS="-E foo"`
