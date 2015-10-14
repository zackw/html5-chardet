#! /usr/bin/python3

import prescan
import sys

failures = 0
successes = 0
verbose = False

def run_one_test(data, encoding, fname, tlineno):
    global failures, successes, verbose

    try:
        data = b"".join(data)
        encoding = encoding.decode("ascii").strip()
        if encoding == "None":
            encoding = None
        else:
            encoding = encoding.lower()

        got_encoding = prescan.determine_encoding(data)
        if got_encoding == encoding:
            if verbose:
                sys.stdout.write("{}:{}: pass\n".format(fname, tlineno))
            successes += 1
        else:
            sys.stdout.write("{}:{}: fail: expected {!r} got {!r}\n"
                             .format(fname, tlineno, encoding, got_encoding))
            failures += 1
    except Exception as e:
        sys.stdout.write("{}:{}: error: {}\n"
                         .format(fname, tlineno, str(e)))
        failures += 1

def run_one_file(fp, fname):
    data = None
    encoding = None
    expect_encoding = False
    tlineno = 0
    for i, line in enumerate(fp.read().split(b'\n')):
        if line == b"#data":
            if data and encoding:
                run_one_test(data, encoding, fname, tlineno)
            data = []
            encoding = None
            tlineno = i+1
        elif line == b"#encoding":
            expect_encoding = True
        elif expect_encoding:
            encoding = line
            expect_encoding = False
        else:
            data.append(line)

def main():
    global failures, successes, verbose

    del sys.argv[0]
    if sys.argv[0] == '-v' or sys.argv[0] == '--verbose':
        del sys.argv[0]
        verbose = True

    for tfname in sys.argv:
        with open(tfname, "rb") as f:
            run_one_file(f, tfname)

    sys.stdout.write("\n{} tests succeeded".format(successes))
    if failures:
        sys.stdout.write(", {} failed".format(failures))
    sys.stdout.write(".\n")
    sys.exit(0 if (successes and not failures) else 1)

main()
