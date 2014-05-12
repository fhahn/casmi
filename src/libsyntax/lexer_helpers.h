#ifndef CASMI_LIBPARSE_LEXER_HELPERS_H
#define CASMI_LIBPARSE_LEXER_HELPERS_H

#include <cstdint>
#include <iostream>

#include "libsyntax/types.h"
#include "libsyntax/driver.h"
#include "libsyntax/parser.tab.h"

#include "libinterpreter/value.h"

INT_T convert_to_long(const char* val, int base, Driver &driver, yy::location loc);
FLOAT_T convert_to_float(const char* val, Driver &driver, yy::location loc);
rational_t convert_to_rational(char* val, Driver &driver, yy::location loc);

#endif
