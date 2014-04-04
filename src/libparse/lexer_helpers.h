#ifndef CASMI_LIBPARSE_LEXER_HELPERS_H
#define CASMI_LIBPARSE_LEXER_HELPERS_H

#include <cstdint>
#include <iostream>

#include "libparse/driver.h"
#include "libparse/parser.tab.h"

uint64_t convert_to_long(const char* val, int base, Driver &driver, yy::location loc);

#endif
