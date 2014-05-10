#include <stdexcept>
#include <cstdio>
#include <cstring>

#include "libsyntax/driver.h"

#define BOLD_BLACK  "\x1b[1m"
#define BOLD_RED    "\x1b[1;31m"
#define RED         "\x1b[31m"
#define RESET       "\x1b[0m"

extern int yylex_destroy(void);

// driver must be global, because it is needed for YY_INPUT
Driver *global_driver;

Driver::Driver () 
    : error_(false), trace_parsing (false), trace_scanning (false), function_table(), init_dependencies(), function_trace_map() {
  file_ = nullptr;
  result = nullptr;

  std::vector<Type*> args;
  args.push_back(new Type(TypeType::SELF));
  function_table.add(new Function("program", args, new Type(TypeType::RULEREF), nullptr));
  
  lines_.push_back("");
}

Driver::~Driver () {
  if (file_ != nullptr) {
    fclose(file_);
  }
  yylex_destroy();
}

size_t Driver::get_next_chars(char buf[], size_t max_size) {
  if (fgets(buf, max_size, file_) == NULL) {
    if (ferror(file_)) {
      return -1;
    } else {
      return 0;
    }
  } else {
    // lines_ must not be empty (initialized with empty string in constructor)
    lines_.back().append(buf);
    if (buf[strlen(buf)-1] == '\n') {
      lines_.push_back("");
    }
    return strlen(buf);
  }
}

AstNode *Driver::parse (const std::string &f) {
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

    // TODO check leak on parser error
    if (res != 0 || error_) {
      return nullptr;
    }

  } catch (const std::exception& e) {
    std::cerr << "error: got exception: " << e.what() << " -- exiting" << std::endl;
    return nullptr;
  }

  return result;
}

void Driver::error (const yy::location& l, const std::string& m) {
  // Set state to error!
  error_ = true;

  std::cerr << BOLD_BLACK << filename_ << ":" << l << ": " <<
     BOLD_RED << "error: " << RESET << BOLD_BLACK << m << RESET << std::endl;

  if (l.begin.line == l.end.line && l.begin.line <= lines_.size()) {
    const std::string& error_line = lines_[l.begin.line-1];
    std::cerr << filename_ <<":" << l.begin.line <<" " << error_line;

    size_t length_to_error = filename_.size()+1+std::to_string(l.begin.line).size()+l.begin.column;
    std::cerr << std::string(length_to_error, ' ');
    std::cerr << RED << std::string(l.end.column-l.begin.column, '^') << RESET;
    std::cerr << std::endl;
  } else {
    for (size_t i=l.begin.line-1; (i < l.end.line && i < lines_.size()); i++) {
      std::cerr << filename_ <<":" << i <<" " << lines_[i];
    }
  }
}

bool Driver::ok() const {
  return !error_;
}

bool Driver::add(RuleNode *rule_root) {
  // TODO can rules and functions have the same name?
  try {
    rules_map_.at(rule_root->name);
    return false;
  } catch (const std::out_of_range& e) {
    DEBUG("Add symbol "+rule_root->name);
    rule_root->binding_offsets = std::move(binding_offsets);
    binding_offsets.clear(); // is this necessary? move should empty map
    rules_map_[rule_root->name] = rule_root;
    return true;
  }
}

RuleNode *Driver::get_init_rule() const {
  return rules_map_.at(init_name);
}

AstNode *StringDriver::parse (const std::string &str) {
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
  AstNode *res = Driver::parse(tmpname);
  remove(tmpname);

  return res;
}


