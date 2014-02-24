#include <cerrno>
#include <cstdlib>

#include "libparse/lexer_helpers.h"


uint64_t convert_to_long(char* val, uint8_t base, casmi_driver &driver, yy::location loc) {
    char* endptr;
    long res = strtol(val, &endptr, base);

    switch (errno) {
        case EINVAL:
            driver.error(loc, "invalid value");
            break;
        case ERANGE:
            driver.error(loc, "value out of range");
            break;
    }
    return res;
}
