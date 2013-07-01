#! /usr/bin/env python

#############################################################################
# Created on January 25, 2013 by Song Guo, USGS/EROS
#
# Prepare inputs for fillminima.py and call it to do image fill
# and then write the output to binary files for cfmask to read
#
############################################################################

import sys
import numpy as np
import fillminima
import os.path
from numpy import zeros , int16 , int32
ERROR = 1
SUCCESS = 0

def mainRoutine():
    nullval = -9999

    f = open('b4_b5.txt', 'r')
    b4_17 = float(f.readline().rstrip('\r\n').strip())
    b5_17 = float(f.readline().rstrip('\r\n').strip())
    nlines = int(f.readline().rstrip('\r\n').strip())
    nsamps = int(f.readline().rstrip('\r\n').strip())
    f.close()

    f = open('b4.bin', 'rb')
    b4 = np.fromfile(f, dtype=int16, count = -1).reshape(nlines, nsamps)
    f.close()

    f = open('b5.bin', 'rb')
    b5 = np.fromfile(f, dtype=int16, count = -1).reshape(nlines, nsamps)
    f.close()

    filled_b4 = fillminima.fillMinima(b4, nullval, b4_17)
    filled_b5 = fillminima.fillMinima(b5, nullval, b5_17)

    f = open('filled_b4.bin', 'wb')
    np.ndarray.tofile(filled_b4.flatten(), f)
    f.close()
    f = open('filled_b5.bin', 'wb')
    np.ndarray.tofile(filled_b5.flatten(), f)
    f.close()

if __name__ == "__main__":
    print "Start running fillminima"
    mainRoutine()
    print "Success running fillminima"
    sys.exit(SUCCESS)
