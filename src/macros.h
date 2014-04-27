#ifndef CASMI_MACROS_H
#define CASMI_MACROS_H

#include <iostream>

#define DEBUG(x) do { std::cerr << "DEBUG: " << x << std::endl; } while (0)
//#define DEBUG(x) do { } while (0)

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif
