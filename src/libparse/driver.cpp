#include <stdexcept>
#include <cstdio>
#include <cstring>

#include "libparse/driver.h"

extern int yylex_destroy(void);

// driver must be global, because it is needed for YY_INPUT
casmi_driver *global_driver;

casmi_driver::casmi_driver () 
    : trace_parsing (false), trace_scanning (false) {
  file_ = nullptr;
}

casmi_driver::~casmi_driver () {
  if (file_ != nullptr) {
    fclose(file_);
  }
  yylex_destroy();

  // cleanup symbol table
  for (auto entry : symbol_table) {
    delete entry.second;
  }
}

size_t casmi_driver::get_next_chars(char buf[], size_t max_size) {
  if (fgets(buf, max_size, file_) == NULL) {
    if (ferror(file_)) {
      return -1;
    } else {
      return 0;
    }
  } else {
    return strlen(buf);
  }
}

AstNode *casmi_driver::parse (const std::string &f) {
  int res = -1;

  if (file_ != nullptr) {
    fclose(file_);
  }

  filename_ = f;
  file_ = fopen(filename_.c_str(), "rt");
  if (file_ == NULL) {
    std::cerr << "error: could not open `" << filename_ << "´" << std::endl;
    return nullptr;
  }

  yy::casmi_parser parser (*this);
  parser.set_debug_level (trace_parsing);

  try {
    res = parser.parse ();

    if (res != 0) {
      return nullptr;
    }

  } catch (const std::exception& e) {
    std::cerr << "error: got exception: " << e.what() << " -- exiting" << std::endl;
    return nullptr;
  }

  return result;
}

void casmi_driver::error (const yy::location& l, const std::string& m) {
  std::cerr << l << ": " << m << std::endl;

}

void casmi_driver::error (const std::string& m) {
  std::cerr << m << std::endl;
}

Symbol *casmi_driver::add_symbol(const std::string& s) {
  try {
    return symbol_table.at(s);
  } catch (const std::out_of_range& e) {
    Symbol *sym = new Symbol(s);
    symbol_table[s] = sym;
    return sym;
  }
}

AstNode *casmi_string_driver::parse (const std::string &str) {
  char tmpname[] = "/tmp/casmi_test_XXXXXX";
  int fd = mkstemp(&tmpname[0]);

  if (fd == -1) {
    std::cerr << "Could not create tmpfile" << std::endl;
    return nullptr;
  }

  FILE *file = fdopen(fd, "w");
  if (file == NULL) {
    std::cerr << "Could not open file stream for tmpfile" << std::endl;
    return nullptr;
  } 

  fwrite(str.c_str(), str.length(), sizeof(char), file);
  fclose(file);
  AstNode *res = casmi_driver::parse(tmpname);
  remove(tmpname);

  return res;
}

