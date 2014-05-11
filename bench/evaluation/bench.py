import os
import subprocess
import sys


bench_path = os.path.dirname(os.path.abspath(__file__))

def iteritems(d):
    """Factor-out Py2-to-3 differences in dictionary item iterator methods"""
    try:
        return d.iteritems()
    except AttributeError:
        return d.items()

def run_script(vm_info, script_path):
    sys.stdout.write("\t {} ...".format(vm_info[0]))
    sys.stdout.flush()
    p = subprocess.Popen([vm_info[1], script_path], stdout=subprocess.PIPE)
    (outstr, errstr) = p.communicate()
    if p.returncode != 0:
        print("\nBenchmark not executed correctly, error was:\n {}".format(errstr))

def run(): pass

if __name__ == '__main__':
    import timeit

    vms = [
        ("new casmi", os.path.join(bench_path, "../../rel_build/bin/casmi")),
        ("old casmi", os.path.expanduser('~/Desktop/casm')),
    ]

    results = {}

    for root, dirs, files in os.walk(bench_path):
        for bench_file in files:
            if not bench_file.endswith(".casm"): continue
            print("Running benchmark %s" % os.path.join(os.path.split(root)[1], bench_file))

            results = {}
            file_path = os.path.join(root, bench_file);
            for vm in vms:
                def run():
                    run_script(vm, file_path)
                time = timeit.timeit('run()'.format(vm, file_path), setup="from __main__ import run", number=1)

                sys.stdout.write(" took %lf s\n" % (time))
                results[vm] = time


            for k, v in iteritems(results):
                if k != vms[0]:
                    # compare other casm solutions to new casmi
                    factor = v / results[vms[0]]
                    print("\t'%s' is %lf faster than '%s'" % (vms[0][0], factor, k[0]))
