#include <iostream>
#include <string>
#include <cstdlib>

#include <stdio.h>
#include <getopt.h>

#include "boost/program_options.hpp"

#include "libutil/exceptions.h"

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

enum OptionValues {  
  NO_OPTIONS = 0,
  HELP = 1,
  DUMP_AST = (1 << 1),
  ERROR = (1 << 2)
};

struct arguments {
  int flags;
  std::string filename;
};

struct arguments parse_cmd_args(int argc, char *argv[]) {
  int dump_ast;
  struct option long_options[] = {
       {"help", no_argument, 0, 'h'},
       {"dump-ast", no_argument, &dump_ast, 1},
       {0, 0, 0, 0}
  };

  int option_index = 0;
  int opt;
  int flags = 0;

  while ((opt = getopt_long(argc, argv, "h",
                            long_options, &option_index)) != -1) {
    switch(opt) {
      case 0:
        if (option_index == 1) {
          flags |= OptionValues::DUMP_AST;
        } else {
          flags |= OptionValues::ERROR;
        }
        break;
      case 'h':
        flags |= OptionValues::HELP;
        break;
      case '?':
        flags |= OptionValues::ERROR;
        /* getopt_long already printed an error message. */
        break;
      default:
        flags |= OptionValues::ERROR;
        break;
    }
  }
  struct arguments opts;
  opts.flags = flags;

  if (optind == argc-1) {
    opts.filename = std::string(argv[optind]);
  }
  return opts;
}

int main (int argc, char *argv[]) {
  int res = 0;
  struct arguments opts = parse_cmd_args(argc, argv);

  if ((opts.flags & OptionValues::ERROR) != 0) {
    std::cerr << "There has been an error" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if (opts.filename.size() == 0) {
    std::cerr << "No filename provided" << std::endl;
    return EXIT_FAILURE;
  }
  //
  // Setup the driver
  Driver driver;
  global_driver = &driver;

  switch (opts.flags) {
    case OptionValues::DUMP_AST: {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file" << std::endl;
        delete driver.result;
        return EXIT_FAILURE;
      }

      AstDumpVisitor dump_visitor;
      AstWalker<AstDumpVisitor, bool> dump_walker(dump_visitor);
      dump_walker.walk_specification(driver.result);
      std::cout << dump_visitor.get_dump();
    }
    case OptionValues::NO_OPTIONS: {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file " << driver.result << std::endl;
        delete driver.result;
        return EXIT_FAILURE;
      }

      TypecheckVisitor typecheck_visitor(driver);
      AstWalker<TypecheckVisitor, Type> typecheck_walker(typecheck_visitor);
      typecheck_walker.walk_specification(driver.result);
      if (!driver.ok()) {
        res = 1;
      } else {
        ExecutionContext ctx(driver.current_symbol_table);
        ExecutionVisitor visitor(ctx, driver.get_init_rule(), driver);
        ExecutionWalker walker(visitor);
        try {
          walker.run();
        } catch (const RuntimeException& ex) {
          res = EXIT_FAILURE;
        }
      }
    }
  }
  delete driver.result;

  return res;
}
