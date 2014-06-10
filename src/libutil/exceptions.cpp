#include "libutil/exceptions.h"


RuntimeException::RuntimeException(const std::string& msg) : msg_(msg) {}

const char* RuntimeException::what() const throw() {
  return msg_.c_str();
}

const char* ImpossibleException::what() const throw() {
  return "ImpossibleException";
}
