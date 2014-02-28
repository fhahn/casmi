#ifndef CASMI_LIBPARSE_DRIVER_H
#define CASMI_LIBPARSE_DRIVER_H
#include <string>
#include <map>
#include <fstream>
#include <cstdint>
#include <cstdio>

#include "libparse/ast.h"
#include "libparse/parser.tab.h"

// Tell Flex the lexer's prototype ...
#define YY_DECL \
    yy::casmi_parser::symbol_type yylex (casmi_driver& driver)
    YY_DECL;

class casmi_driver {
public:
  casmi_driver ();
  virtual ~casmi_driver ();

  std::map<std::string, int> variables;

  AstNode *result;

  // State information for the lexer
  std::string filename_;
  FILE *file_;
  bool trace_parsing;
  bool trace_scanning;

  // Handling the scanner.
  size_t get_next_chars(char buffer[], size_t max_size);

  // Run the parser. Return 0 on success.
  int parse (const std::string& f);

  
  // Error handling.
  void error (const yy::location& l, const std::string& m);
  void error (const std::string& m);
};

#endif
