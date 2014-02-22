#include "libparse/driver.h"

casmi_driver::casmi_driver ()
  : trace_scanning (false), trace_parsing (false)
{
  variables["one"] = 1;
  variables["two"] = 2;
}

casmi_driver::~casmi_driver ()
{
}

int
casmi_driver::parse (const std::string &f)
{
  file = f;
  scan_begin ();
  yy::casmi_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void
casmi_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void
casmi_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}


