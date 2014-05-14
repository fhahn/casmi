from datetime import datetime
import re
import os
import subprocess
import sys


NUM_RUNS = 3

bench_path = os.path.dirname(os.path.abspath(__file__))


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

def run_script(vm_info, script_path):
    p = subprocess.Popen([vm_info[1], script_path], stdout=subprocess.PIPE)
    (outstr, errstr) = p.communicate()
    if p.returncode != 0:
        print("\nBenchmark not executed correctly, error was:\n {}".format(errstr))

def run(): pass

if __name__ == '__main__':
    import timeit

    vms = [
        ("new casmi", sys.argv[1]),
        ("old casmi", os.path.expanduser('~/Desktop/casm')),
    ]

    for root, dirs, files in os.walk(bench_path):
        for bench_file in files:
            if not bench_file.endswith(".casm"): continue
            print("Running benchmark %s" % os.path.join(os.path.split(root)[1], bench_file))

            results = {}
            file_path = os.path.join(root, bench_file);
            for vm in vms:
                sys.stdout.write("\t {} ...".format(vm[0]))
                sys.stdout.flush()

                def run():
                    run_script(vm, file_path)
                time = timeit.timeit('run()'.format(vm, file_path), setup="from __main__ import run", number=NUM_RUNS)

                sys.stdout.write(" took %lf s\n" % (time))
                results[vm] = time
                dump_run(bench_file, vm, time)


            for k, v in iteritems(results):
                if k != vms[0]:
                    # compare other casm solutions to new casmi
                    factor = v / results[vms[0]]
                    print("\t'%s' is %lf faster than '%s'" % (vms[0][0], factor, k[0]))
