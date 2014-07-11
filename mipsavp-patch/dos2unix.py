#!/usr/bin/python
import os
import sys
import tempfile

def main():
    filename = sys.argv[1]
    tempfilename = filename+".dos2unix.temp"
    fh = open (tempfilename, 'w')
    for line in open(filename):
        line = line.rstrip()
        fh.write(line + '\n')
    os.remove(filename)
    os.rename(tempfilename, filename)


if __name__ == '__main__':
    main()
