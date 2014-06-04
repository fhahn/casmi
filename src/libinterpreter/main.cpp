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
  DEBUGINFO_FILTER = (1 << 3),
  SYMBOLIC = (1 << 4),
  FILEOUT = (1 << 5),
  ERROR = (1 << 9)
};

struct arguments {
  int flags;
  std::string filename;
  std::string debuginfo_filter;
};

struct arguments parse_cmd_args(int argc, char *argv[]) {
  int dump_ast = 0;
  int parse_only = 0;
  struct option long_options[] = {
       {"help", no_argument, 0, 'h'},
       {"dump-ast", no_argument, &dump_ast, 1},
       {"parse-only", no_argument, &parse_only, 1},
       {"debuginfo-filter", required_argument, 0, 'd'},
       {"symbolic", no_argument, 0, 's'},
       {"fileout", no_argument, 0, 'x'},
       {0, 0, 0, 0}
  };

  int option_index = 0;
  int opt;
  int flags = 0;

  struct arguments opts;

  while ((opt = getopt_long(argc, argv, "hd:sx",
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
      case 'd':
        flags |= OptionValues::DEBUGINFO_FILTER;
        opts.debuginfo_filter = optarg;
        break;
      case 's':
        flags |= OptionValues::SYMBOLIC;
        break;
      case 'x':
        flags |= OptionValues::FILEOUT;
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
  std::cout << "  --debuginfo-filter FILTERS" << "\t\t" << "comma separated list with filter names to enable"<< std::endl;
  std::cout << "  -s, --symbolic" << "\t\t" << "enable symbolic mode" << std::endl;
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

  if ((opts.flags & OptionValues::PARSE_ONLY) != 0) {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file" << std::endl;
        res = EXIT_FAILURE;
      } else {
        res = EXIT_SUCCESS;
      }
  } else if ((opts.flags & OptionValues::DUMP_AST) != 0) {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file" << std::endl;
        return EXIT_FAILURE;
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
  } else {
      if (driver.parse(opts.filename) == nullptr) {
        std::cerr << "Error parsing file " << driver.result << std::endl;
        return EXIT_FAILURE;
      }

      TypecheckVisitor typecheck_visitor(driver);
      AstWalker<TypecheckVisitor, Type*> typecheck_walker(typecheck_visitor);
      typecheck_walker.walk_specification(driver.result);
      if (!driver.ok()) {
        res = EXIT_FAILURE;
      } else {
        ExecutionContext ctx(driver.function_table, driver.get_init_rule(),
            (opts.flags & OptionValues::SYMBOLIC) != 0,
             (opts.flags & OptionValues::FILEOUT) != 0);

        if ((opts.flags & OptionValues::DEBUGINFO_FILTER) != 0) {
          ctx.set_debuginfo_filter(opts.debuginfo_filter);
        }

        ExecutionVisitor visitor(ctx, driver);
        ExecutionWalker walker(visitor);
        try {
          walker.run();
          res = EXIT_SUCCESS;
        } catch (const RuntimeException& ex) {
          std::cerr << "Abort after runtime exception: "<< ex.what() << std::endl;;
          res = EXIT_FAILURE;
        } catch (char * e) {
          std::cerr << "Abort after catching a string: "<< e << std::endl;
          res = EXIT_FAILURE;
        }
      }
  }
  if (driver.result) {
    delete driver.result;
  }

  return res;
}
