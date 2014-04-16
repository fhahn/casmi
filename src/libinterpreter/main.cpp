#include <iostream>
#include <string>
#include <cstdlib>

#include <getopt.h>

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
  PARSE_ONLY = (1 << 2),
  ERROR = (1 << 9)
};

struct arguments {
  int flags;
  std::string filename;
};

struct arguments parse_cmd_args(int argc, char *argv[]) {
  int dump_ast = 0;
  int parse_only = 0;
  struct option long_options[] = {
       {"help", no_argument, 0, 'h'},
       {"dump-ast", no_argument, &dump_ast, 1},
       {"parse-only", no_argument, &parse_only, 1},
       {0, 0, 0, 0}
  };

  int option_index = 0;
  int opt;
  int flags = 0;

  while ((opt = getopt_long(argc, argv, "h",
                            long_options, &option_index)) != -1) {
    switch(opt) {
      case 0:
        switch (option_index) {
          case 1: flags |= OptionValues::DUMP_AST; break;
          case 2: flags |= OptionValues::PARSE_ONLY; break;
          default: flags |= OptionValues::ERROR;
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

void print_help() {
  std::cout << "USAGE: casmi [OPTIONS] <filename>" << std::endl;
  std::cout << std::endl;
  std::cout << "OPTIONS:" << std::endl;
  std::cout << "  -h, --help" << "\t\t" << "shows command line options" << std::endl;
  std::cout << "  --dump-ast" << "\t\t" << "dumps the AST as dot graph" << std::endl;
  std::cout << "  --parse-only" << "\t\t" << "only parse the input, does not run typechecking" << std::endl;
}

int main (int argc, char *argv[]) {
  int res = 0;
  struct arguments opts = parse_cmd_args(argc, argv);

  if ((opts.flags & OptionValues::ERROR) != 0) {
    std::cerr << "There has been an error" << std::endl;
    return EXIT_FAILURE;
  }

  if (opts.flags == OptionValues::HELP) {
    print_help();
    return EXIT_SUCCESS;
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
    case OptionValues::PARSE_ONLY: {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file" << std::endl;
        res = EXIT_FAILURE;
      } else {
        res = EXIT_SUCCESS;
      }
      break;
    }
    case OptionValues::DUMP_AST: {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file" << std::endl;
        res = EXIT_FAILURE;
        break;
      }

      try {
        AstDumpVisitor dump_visitor;
        AstWalker<AstDumpVisitor, bool> dump_walker(dump_visitor);
        dump_walker.walk_specification(driver.result);
        std::cout << dump_visitor.get_dump();
        res = EXIT_SUCCESS;
      } catch (char const *e) {
        std::cerr << "Abort after catching a string: "<< e;
        res = EXIT_FAILURE;
      }
      break;
    }
    case OptionValues::NO_OPTIONS: {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file " << driver.result << std::endl;
        res = EXIT_FAILURE;
        break;
      }

      TypecheckVisitor typecheck_visitor(driver);
      AstWalker<TypecheckVisitor, Type> typecheck_walker(typecheck_visitor);
      typecheck_walker.walk_specification(driver.result);
      if (!driver.ok()) {
        res = EXIT_FAILURE;
      } else {
        ExecutionContext ctx(driver.function_table, driver.get_init_rule());
        ExecutionVisitor visitor(ctx, driver);
        ExecutionWalker walker(visitor);
        try {
          walker.run();
          res = EXIT_SUCCESS;
        } catch (const RuntimeException& ex) {
          std::cerr << "Abort after runtime exception: "<< ex.what();
          res = EXIT_FAILURE;
        } catch (char * e) {
          std::cerr << "Abort after catching a string: "<< e;
          res = EXIT_FAILURE;
        }
      }
      break;
    }
  }
  if (driver.result) {
    delete driver.result;
  }

  return res;
}
