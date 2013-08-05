#! /usr/bin/env python

import sys
import numpy as np
import fillminima
import os.path
from numpy import zeros , int16 , int32

### Error/Success codes ###
ERROR = 1
SUCCESS = 0

############################################################################
# Description: logIt logs the information to the logfile (if valid) or to
# stdout if the logfile is None.
#
# Inputs:
#   msg - message to be printed/logged
#   log_handler - log file handler; if None then print to stdout
#
# Returns: nothing
#
# Notes:
############################################################################
def logIt (msg, log_handler):
    if log_handler == None:
        print msg
    else:
        log_handler.write (msg + '\n')

#############################################################################
# Created on January 25, 2013 by Song Guo, USGS/EROS
#
# Prepare inputs for fillminima.py and call it to do image fill
# and then write the output to binary files for cfmask to read
#
############################################################################

def mainRoutine(logfile=None):
    nullval = -9999
    fname_txt = 'b4_b5.txt' # input text file
    fname_b4 = 'b4.bin'     # input band 4 binary file
    fname_b5 = 'b5.bin'     # input band 5 binary file

    print "Start running fillminima"    
    # open the log file if it exists; use line buffering for the output
    log_handler = None
    if logfile != None:
        log_handler = open (logfile, 'w', buffering=1)

    # check the existenece of the input text file
    if not os.path.isfile(fname_txt): 
        msg = 'Input file does not exist: ' + fname_txt
        logIt (msg, log_handler)
        return ERROR

    # Read out the input text file
    f = open(fname_txt, 'r')
    b4_17 = float(f.readline().rstrip('\r\n').strip())
    b5_17 = float(f.readline().rstrip('\r\n').strip())
    nlines = int(f.readline().rstrip('\r\n').strip())
    nsamps = int(f.readline().rstrip('\r\n').strip())
    f.close()

    # check the existenece of the input band 4 binary file
    if not os.path.isfile(fname_b4): 
        msg = 'Input file does not exist: ' + fname_b4
        logIt (msg, log_handler)
        return ERROR

    # Read out the input band 4 bunary file
    f = open(fname_b4, 'rb')
    b4 = np.fromfile(f, dtype=int16, count = -1).reshape(nlines, nsamps)
    f.close()

    # check the existenece of the input band 5 binary file
    if not os.path.isfile(fname_b5): 
        msg = 'Input file does not exist: ' + fname_b5
        logIt (msg, log_handler)
        return ERROR

    # Read out the input band 5 bunary file
    f = open(fname_b5, 'rb')
    b5 = np.fromfile(f, dtype=int16, count = -1).reshape(nlines, nsamps)
    f.close()

    # Call the fillMinima routine for both band 4 and band 5
    filled_b4 = fillminima.fillMinima(b4, nullval, b4_17)
    filled_b5 = fillminima.fillMinima(b5, nullval, b5_17)

    # Output filled image to band 4 binary output file 
    f = open('filled_b4.bin', 'wb')
    np.ndarray.tofile(filled_b4.flatten(), f)
    f.close()

    # Output filled image to band 5 binary output file 
    f = open('filled_b5.bin', 'wb')
    np.ndarray.tofile(filled_b5.flatten(), f)
    f.close()

    print "Success running fillminima"
    return SUCCESS

if __name__ == "__main__":
    sys.exit(mainRoutine())
