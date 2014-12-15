#!/usr/bin/python

import sys
import os
import shutil
import re

def analyzeSingleLine(l):
    TEST = re.compile('TEST:(.+):(.+)')

    m = TEST.match(l)
    if m:
        if (m.group(1) == m.group(2)):
            print "%s <- OK" % l
            return 0
        else:
            print "%s <- FAIL" % l
            return 1
    else:
        print "%s <- FAIL (test unrecognized)" % l
        return 1

def analyzeLogfile(filename):
    TEST = re.compile('TEST:')

    tests = 0
    passed = 0
    failed = 0

    with open(filename,"r") as f:
        for line in f:
            m = TEST.match(line)
            if m:
                tests += 1

                if analyzeSingleLine(line.rstrip()) == 0:
                    passed += 1
                else:
                    failed += 1


    print "Test cases: %d" % tests
    print "PASSED: %d" % passed
    if failed > 0:
        print "FAILED: %d" % failed

    return (failed != 0)

if __name__ == '__main__':
    if len(sys.argv) > 0:
        for arg in sys.argv:
            analyzeLogfile(arg)
    sys.exit(0)
