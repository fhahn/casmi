#include <iostream>

#include "libparse/driver.h"
#include "libparse/types.h"
#include "libparse/parser.tab.h"

#include "libinterpreter/execution_visitor.h"

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
            driver.result->propagate_types(Type::NO_TYPE, driver);
            if (!driver.ok()) {
              res = 1;
            }
            ExecutionVisitor exec(driver.result);
            exec.execute();
            delete driver.result;
        } else
            res = 1;
    }
    return res;
}
