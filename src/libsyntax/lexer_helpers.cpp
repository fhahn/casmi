#include <cerrno>
#include <cstdlib>
#include <stdexcept>

#include "libsyntax/lexer_helpers.h"


INT_T convert_to_long(const char* val, int base, Driver &driver, yy::location loc) {
  char* endptr;
  errno = 0;
  INT_T res = strtol(val, &endptr, base);

  switch (errno) {
      case EINVAL:
        driver.error(loc, "invalid value");
        errno = 0;
        throw std::out_of_range("invalid value");
      case ERANGE:
        driver.error(loc, "value out of range");
        errno = 0;
        throw std::out_of_range("value out of range");
  }

  if (*endptr != '\0') {
      driver.error(loc, "invalid value");
      throw std::out_of_range("invalid value");
  }
  return res;
}

FLOAT_T convert_to_float(const char* val, Driver &driver, yy::location loc) {
  char* endptr;
  errno = 0;
  FLOAT_T res = strtod(val, &endptr);

  if (errno == ERANGE) {
    driver.error(loc, "value out of range");
    errno = 0;
    throw std::out_of_range("value out of range");
  }

  if (*endptr != '\0') {
      driver.error(loc, "invalid value");
      throw std::out_of_range("invalid value");
  }
  return res;
}
