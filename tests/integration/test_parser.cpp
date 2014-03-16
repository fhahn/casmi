#include <iostream>

#include "libparse/driver.h"
#include "libparse/visitor.h"
#include "libparse/types.h"
#include "libparse/parser.tab.h"

// driver must be global, because it is needed for YY_INPUT
// defined in src/libparse/driver.cpp
extern casmi_driver *global_driver;

int main (int argc, char *argv[]) {
    int res = 0;
    bool print = false;
    casmi_driver driver = casmi_driver();
    global_driver = &driver;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == std::string ("-p"))
            driver.trace_parsing = true;
        else if (argv[i] == std::string ("-s"))
            driver.trace_scanning = true;
        else if (argv[i] == std::string ("--print-ast")) {
            print = true;
        } else if (driver.parse (argv[i]) != nullptr) {
            std::cout << driver.result << std::endl;
            if (print) {
                PrintVisitor v;
                driver.result->visit(v);
            }
            driver.result->propagate_types(Type::NO_TYPE, driver);
            delete driver.result;
        } else
            res = 1;
    }
    return res;
}
