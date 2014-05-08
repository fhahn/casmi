import sys
import re


#builtin_re = re.compile("DEFINE_CASM_SHARED\((\w+,)(ARG\(.+\),)*\)")
#builtin_re = re.compile("DEFINE_CASM_SHARED\((\w+), (?:ARG\(Int, (\w+)\),? ?)+\)")
builtin_re = re.compile("DEFINE_CASM_SHARED\((\w+), (.*)\)")
arg_re = re.compile("(?:ARG\(Int, (\w+)\),? ?)")

def create_typecheck_defines(builtins):
    builtin_ids = [b[0].upper() for b in builtins]
    define_builtin_ids = "#define SHARED_BUILTIN_IDS \\\n{}\n".format(
            ", \\\n   ".join(builtin_ids))

    builtin_names = ["    {:<60},".format("{ \""+b[0]+"\", true }") for b in builtins]
    define_builtin_names = "\n\n#define SHARED_BUILTIN_NAMES \\\n   {}\n".format(
            "\\\n".join(builtin_names))

    builtin_typechecks = []
    for b in builtins:
        lines = []
        lines.append("if (name == \"{}\" ) {{".format(b[0]))

        arg_count = 1
        for arg in b[2:]:
            lines.append("  Type *a"+str(arg_count)+" = new Type(TypeType::INT);")
            arg_count += 1

        lines.append("  return_type = new Type(TypeType::INT);")
        lines.append("  types = { "+",".join(["a"+str(i) for i in range(1,arg_count)])+" };")

        lines.append("  id = Id::{};".format(b[0].upper()))
        lines.append("} else ")

        check_template ="".join(len(lines)*["    {:<60}\\\n"])
        check = check_template.format(*lines)
        builtin_typechecks.append(check)

    builtin_typechecks.append("{ assert(0); } \n")
    define_typechecks = "\n\n#define SHARED_BUILTINS_TYPECHECK \\\n{}\n".format(
            "".join(builtin_typechecks))

    return "".join([define_builtin_ids, define_builtin_names, define_typechecks])

def create_dispatch_define(builtins):
    cases = []
    for b in builtins:
        arg_ind = 0;
        lines = []
        lines.append("case BuiltinAtom::Id::{}:".format(b[0].upper()))
        for arg in b[2:]:
            lines.append("  arg{0:} = {{ (uint64_t) arguments[{0:}].value.ival, 1 }};".format(str(arg_ind)))
            arg_ind += 1

        lines.append("  {}(&ret, ".format(b[0])+", ".join(["&arg"+str(i) for i in range(0, arg_ind)])+");")
        lines.append("  break;")
  
        case_template ="".join(len(lines)*["    {:<60}\\\n"])
        case = case_template.format(*lines)
        cases.append(case)

    return "\n\n#define SHARED_DISPATCH \\\n"+"".join(cases)


if __name__ == '__main__':
    builtins = []
    with open(sys.argv[1]) as f:
        builtins = [
            [match.group(1)]+arg_re.findall(match.group(2)) 
                        for match in builtin_re.finditer(f.read())
        ]

    with open(sys.argv[2], "wt") as f:
        res = create_typecheck_defines(builtins)
        res += create_dispatch_define(builtins)
        f.write(res)
