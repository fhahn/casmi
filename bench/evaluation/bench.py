from datetime import datetime
import re
import os
import subprocess
import sys
import timeit
import tempfile
import shutil


NUM_RUNS = 10

bench_path = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(bench_path, "../../deps/click"))

import click


def dump_run(bench_name, vm, time):
    p = subprocess.Popen(["git", "log", "-n", "1"], stdout=subprocess.PIPE)
    (out, _) = p.communicate()
    commit_id = out.splitlines()[0].decode("utf-8").replace("commit ", "")

    p = subprocess.Popen(["strings", "-a", vm[1]], stdout=subprocess.PIPE)
    (out, _) = p.communicate()

    compiler = "unknown"

    clang_re = re.compile("clang version ([0-9].[0-9])")
    match = clang_re.search(out.decode("utf-8"))
    if match:
        compiler = "clang-"+match.groups()[0]
    else:
        p = subprocess.Popen(["objdump", "-s", "--section", ".comment", vm[1]], stdout=subprocess.PIPE)
        (out, _) = p.communicate()
        gcc_re = re.compile("\(GNU\) ([0-9].[0-9])")
        match = gcc_re.search(out.decode("utf-8"))

        if match:
            compiler = "gcc-"+match.groups()[0]

    with open(os.path.join(bench_path, "bench.log"), "a") as f:
        f.write("{} ; {} ; {} ; {} ; {} ; {} ; {}\n".format(
            str(datetime.now()), commit_id, compiler, vm[0], bench_name,
            str(time), str(NUM_RUNS)))



def iteritems(d):
    """Factor-out Py2-to-3 differences in dictionary item iterator methods"""
    try:
        return d.iteritems()
    except AttributeError:
        return d.items()

def run_script(vm_path, script_path):
    p = subprocess.Popen([vm_path, script_path], stdout=subprocess.PIPE)
    (outstr, errstr) = p.communicate()
    if p.returncode != 0:
        print("\nBenchmark not executed correctly, error was:\n {}".format(errstr))

def run_compiler(file_path):
    p = subprocess.Popen([file_path], stdout=subprocess.PIPE)
    (outstr, errstr) = p.communicate()
    if p.returncode != 0:
        print("\nBenchmark not executed correctly, error was:\n {}".format(errstr))



def error_abort(msg):
    click.echo(click.style('error: ', fg='red', bold=True), nl=False)
    click.echo(msg)
    sys.exit(1)

@click.command()
@click.option('--new-casmi', help='Path to the new (base) interpreter.', required=True)
@click.option('--legacy-casmi', help='Path to the legacy interpreter.',
              required=False, envvar='LEGACY_CASMI')
@click.option('--casm-compiler', help='Path to the CASM compiler.',
              required=False, envvar='CASM_COMPILER')
def main(new_casmi, legacy_casmi, casm_compiler):
    vms = []

    if not os.path.exists(new_casmi):
        error_abort('File `%s` for new-casmi does not exist' % click.format_filename(new_casmi))
    else:
        vms.append(("new casmi", os.path.abspath(os.path.expanduser(new_casmi))))

    if legacy_casmi is not None:
        if not os.path.exists(legacy_casmi):
            error_abort('File `%s` for legacy-casmi does not exist' % click.format_filename(legacy_casmi))
        else:
            vms.append(("old casmi", os.path.abspath(os.path.expanduser(legacy_casmi))))

    if casm_compiler is not None:
        if not os.path.exists(legacy_casmi):
            error_abort('File `%s` for casm-compiler does not exist' % click.format_filename(casm_compiler))
        vms.append(('compiler', os.path.abspath(os.path.expanduser(casm_compiler))))

    for root, dirs, files in os.walk(bench_path):
        for bench_file in files:
            if not bench_file.endswith(".casm"): continue
            #click.echo("Running benchmark %s" % os.path.join(os.path.split(root)[1], bench_file))

            results = {}
            file_path = os.path.join(root, bench_file);
            sys.stdout.write(bench_file)
            sys.stdout.flush()

            for vm in vms:
 
                if vm[0] == 'compiler':
                    tmp_dir = tempfile.mkdtemp()
                    out_file = os.path.join(tmp_dir, 'out')
                    subprocess.call([casm_compiler, file_path, '-o', out_file, '-c'])
                    time = timeit.timeit('run_compiler("{}")'.format(out_file), setup="from __main__ import run_compiler", number=NUM_RUNS)
                    shutil.rmtree(tmp_dir)
                else:
                    time = timeit.timeit('run_script("{}", "{}")'.format(vm[1], file_path), setup="from __main__ import run_script", number=NUM_RUNS)

                #sys.stdout.write(" took %lf s\n" % (time))
                results[vm[0]] = time
                #dump_run(bench_file, vm, time)

            parts = [str(NUM_RUNS), str(results["new casmi"])]

            if 'old casmi' in results:
                parts.append(str(results["old casmi"]))
            if 'compiler' in results:
                parts.append(str(results["compiler"]))
            print(" ; "+" ; ".join(parts))


            """
            for k, v in iteritems(results):
                if k != vms[0]:
                    # compare other casm solutions to new casmi
                    factor = v / results[vms[0]]
                    print("\t'%s' is %lf faster than '%s'" % (vms[0][0], factor, k[0]))
            """
if __name__ == '__main__':
    main()
