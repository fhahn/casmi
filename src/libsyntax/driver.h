#ifndef CASMI_LIBPARSE_DRIVER_H
#define CASMI_LIBPARSE_DRIVER_H
#include <string>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>

#include "libsyntax/parser.tab.h"
#include "libsyntax/symbols.h"

class AstNode;

class Driver {
  private:
    std::string filename_;
    FILE *file_;
    std::vector<std::string> lines_;
    bool error_;

  public:
    Driver ();
    virtual ~Driver ();

    std::map<std::string, RuleNode*> rules_map_;
    AstNode *result;

    std::string init_name;

    // State information for the lexer
    bool trace_parsing;
    bool trace_scanning;

    // Handling the scanner.
    size_t get_next_chars(char buffer[], size_t max_size);

    // Run the parser. Return 0 on success.
    AstNode *parse(const std::string& f);

    // Error handling.
    void error (const yy::location& l, const std::string& m);
    bool ok() const;

    // Rule handling
    bool add(RuleNode *rule_root);
    RuleNode *get_init_rule() const;

    // symbol table stuff
    FunctionTable *current_symbol_table;
};

class StringDriver: public Driver {
  private:
    std::string str_;
    std::string::iterator iter_;
    std::string::iterator end_;

  public:
    AstNode *parse (const std::string& str);
};

// Tell Flex the lexer's prototype ...
#define YY_DECL \
    yy::casmi_parser::symbol_type yylex (Driver& driver)
    YY_DECL;


#endif
