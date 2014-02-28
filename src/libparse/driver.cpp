#include <stdexcept>
#include <cstdio>
#include <cstring>

#include "libparse/driver.h"

// driver must be global, because it is needed for YY_INPUT
casmi_driver *global_driver;

casmi_driver::casmi_driver () 
    : trace_parsing (false), trace_scanning (false) {
  file_ = NULL;
}

casmi_driver::~casmi_driver () {
  if (file_ != NULL) {
    fclose(file_);
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

int casmi_driver::parse (const std::string &f) {
  int res = -1;

  filename_ = f;
  file_ = fopen(filename_.c_str(), "rt");
  if (file_ == NULL) {
    std::cerr << "error: could not open `" << filename_ << "´" << std::endl;
    return -1;
  }

  yy::casmi_parser parser (*this);
  parser.set_debug_level (trace_parsing);

  try {
      res = parser.parse ();
  } catch (const std::exception& e) {
      std::cerr << "error: got exception: " << e.what() << " -- exiting" << std::endl;
      return -1;
  }

  return res;
}

void casmi_driver::error (const yy::location& l, const std::string& m) {
  std::cerr << l << ": " << m << std::endl;

}

void casmi_driver::error (const std::string& m) {
  std::cerr << m << std::endl;
}

int casmi_string_driver::parse (const std::string &str) {

  int res = 0;
  char tmpname[] = "/tmp/casmi_test_XXXXXX";
  
  int fd = mkstemp(&tmpname[0]);

  if (fd == -1) {
    std::cerr << "Could not create tmpfile" << std::endl;
    res = -1;
  } else {
    FILE *file = fdopen(fd, "w");
    if (file == NULL) {
      std::cerr << "Could not open file stream for tmpfile" << std::endl;
      res = -1;
    } else {
      fwrite(str.c_str(), str.length(), sizeof(char), file);
      fclose(file);
      res = casmi_driver::parse(tmpname);
      remove(tmpname);
    }
  }

  return res;
}
