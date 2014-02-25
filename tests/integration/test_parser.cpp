#include <iostream>

#include "libparse/driver.h"
#include "libparse/parser.tab.h"

int main (int argc, char *argv[]) {
    int res = 0;
    bool print = false;
    casmi_driver driver;
    for (int i = 1; i < argc; ++i)
        if (argv[i] == std::string ("-p"))
            driver.trace_parsing = true;
        else if (argv[i] == std::string ("-s"))
            driver.trace_scanning = true;
        else if (argv[i] == std::string ("--print-ast")) {
            print = true;
        } 
        else if (!driver.parse (argv[i])) {
            std::cout << driver.result << std::endl;
            if (print) {
                PrintVisitor v;
                driver.result->visit(v);
            }
        } else
            res = 1;
    return res;
}
