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


rational_t convert_to_rational(char* val, Driver &driver, yy::location loc) {
  char *numerator_start = val;
  char *denominator_start = nullptr;

  uint32_t len = strlen(val);
  for (uint32_t i=0; i < len; i++) {
    if (val[i] == '/') {
      val[i] = 0;
      denominator_start = &val[i+1];
    }
  }

  int64_t numerator = convert_to_long(numerator_start, 10, driver, loc);
  int64_t denominator = 1;
  if (denominator_start) {
    denominator = convert_to_long(denominator_start, 10, driver, loc);
  }

  return { numerator, denominator };
}
