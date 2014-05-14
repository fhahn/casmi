#ifndef CASMI_MACROS_H
#define CASMI_MACROS_H

#include <iostream>

#if defined(CASMI_DEBUG)
  #define DEBUG(x) do { std::cerr << "DEBUG: " << x << std::endl; } while (0)
#else
  #define DEBUG(x) do { } while (0)
#endif


#define UNUSED(expr) do { (void)(expr); } while (0)

#ifdef __clang__
  #define IGNORE_VARIADIC_WARNINGS \
    _Pragma("GCC diagnostic ignored \"-Wvariadic-macros\"") \
    _Pragma("GCC diagnostic ignored \"-Wgnu-zero-variadic-macro-arguments\"")
#else
  #define IGNORE_VARIADIC_WARNINGS \
    _Pragma("GCC diagnostic ignored \"-Wvariadic-macros\"")
#endif


#ifdef __clang__
  #define REENABLE_VARIADIC_WARNINGS \
    _Pragma("GCC diagnostic warning \"-Wvariadic-macros\"") \
    _Pragma("GCC diagnostic warning \"-Wgnu-zero-variadic-macro-arguments\"")
#else
  #define REENABLE_VARIADIC_WARNINGS \
    _Pragma("GCC diagnostic warning \"-Wvariadic-macros\"")

#endif

#endif
