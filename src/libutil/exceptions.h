#ifndef CASMI_EXCEPTIONS_H
#define CASMI_EXCEPTIONS_H

#include <exception>
#include <string>

class RuntimeException : public std::exception {
  private:
    const std::string msg_;

  public:
    RuntimeException(const std::string& msg);
    virtual const char* what() const throw();

};
#endif //CASMI_EXCEPTIONS_H
