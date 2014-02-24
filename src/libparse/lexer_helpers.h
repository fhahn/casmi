#ifndef CASMI_LIBPARSE_LEXER_HELPERS_H
#define CASMI_LIBPARSE_LEXER_HELPERS_H

#include <cstdint>

#include "libparse/driver.h"

#include "parser.tab.h"

uint64_t convert_to_long(char* val, uint8_t base, casmi_driver &driver, yy::location loc);

#endif
