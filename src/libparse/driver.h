#ifndef CASMI_LIBPARSE_DRIVER_H
#define CASMI_LIBPARSE_DRIVER_H
#include <string>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>

#include "libparse/parser.tab.h"
#include "libparse/symbols.h"

class AstNode;

class casmi_driver {
private:
  std::string filename_;
  FILE *file_;
  std::vector<std::string> lines_;
  bool error_;

public:
  casmi_driver ();
  virtual ~casmi_driver ();

  AstNode *result;

  // State information for the lexer
  bool trace_parsing;
  bool trace_scanning;

  // Handling the scanner.
  size_t get_next_chars(char buffer[], size_t max_size);

  // Run the parser. Return 0 on success.
  AstNode *parse(const std::string& f);

  // Error handling.
  void error (const yy::location& l, const std::string& m);

  // symbol table stuff
  SymbolTable *current_symbol_table;
};

class casmi_string_driver : public casmi_driver {
  private:
    std::string str_;
    std::string::iterator iter_;
    std::string::iterator end_;

  public:
    AstNode *parse (const std::string& str);
};

// Tell Flex the lexer's prototype ...
#define YY_DECL \
    yy::casmi_parser::symbol_type yylex (casmi_driver& driver)
    YY_DECL;


#endif
