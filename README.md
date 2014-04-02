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

Tests
------------------------

Use `make check-unit` to run the unit tests (using gtest) and
`make check-integration` to run the integration tests.

`make check` is equivalent to `make check-unit check-integration`
