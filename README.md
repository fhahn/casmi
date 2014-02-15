casmi
=====

``
$ cd path/to/casmi
$ mkdir build && cd build
$ cmake ..
$ make
```


Tests
------------------------

At the moment `ctest` is used for the integration tests.
To run the tests, use `make check`. 

To pass options to `ctest` use `make ARGS="-E foo"`
