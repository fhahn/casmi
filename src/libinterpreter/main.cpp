#include <iostream>
#include <string>

#include "boost/program_options.hpp"

#include "libsyntax/driver.h"
#include "libsyntax/types.h"
#include "libsyntax/ast_dump_visitor.h"
#include "libsyntax/parser.tab.h"
#include "libsyntax/ast_dump_visitor.h"

#include "libmiddle/typecheck_visitor.h"

#include "libinterpreter/execution_visitor.h"
#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

// driver must be global, because it is needed for YY_INPUT
// defined in src/libsyntax/driver.cpp
extern Driver *global_driver;

namespace po = boost::program_options;

int main (int argc, char *argv[]) {
  int res = 0;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("compression", po::value<int>(), "set compression level")
    ("input-file", po::value< std::string >(), "input file")
    ("dump-ast", po::value<bool>(), "dump AST as dot graph")
  ;

  po::positional_options_description p;
  p.add("input-file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
                options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (!vm.count("input-file")) {
    std::cout << "No filename provided" << std::endl;
    std::cout << desc << "\n";
  }

  // Setup the driver
  Driver driver = Driver();
  global_driver = &driver;

  if (driver.parse(vm["input-file"].as<std::string>()) != nullptr) {

    if (vm.count("dump-ast")) {
      AstDumpVisitor dump_visitor;
      AstWalker<AstDumpVisitor, bool> dump_walker(dump_visitor);
      dump_walker.walk_specification(driver.result);
      std::cout << dump_visitor.get_dump();
      return 0;
    }
    TypecheckVisitor typecheck_visitor(driver);
    AstWalker<TypecheckVisitor, Type> typecheck_walker(typecheck_visitor);
    typecheck_walker.walk_specification(driver.result);
    if (!driver.ok()) {
      res = 1;
    } else {
      ExecutionContext ctx(driver.current_symbol_table);
      ExecutionVisitor visitor(ctx, driver.get_init_rule());
      ExecutionWalker walker(visitor);
      walker.run();
    }
  } else {
    res = 1;
  }

  delete driver.result;
  return res;
  /*
        if (argv[i] == std::string ("-p"))
            driver.trace_parsing = true;
        else if (argv[i] == std::string ("-s"))
            driver.trace_scanning = true;
  */
}
