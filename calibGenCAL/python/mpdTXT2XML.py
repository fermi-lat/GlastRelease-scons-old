"""
Tool to generate CAL offline MeVPerDAC calibration XML file from space delimited TXT file.
Note: mpdTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).


mpdTXT2XML [-doptional.dtd] input.txt output.xml

where:
       -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
       input.txt  = input txt file, space delimited in following format:
                      twr lyr col diode mevPerDAC sigma
       output.xml = properly formatted offline CAL mevPerDAC XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL mevPerDAC calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date$"
__version__   = "$Revision$, $Author$"
__release__   = "$Name$"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import numarray
import sets
import array

import calCalibXML
import calConstant
import cgc_util

#######################################################################################

if __name__ == '__main__':

    # constants
    dtdName    = "calCalib_v2r3.dtd"
    nTXTFields = 6
    
    # setup logger
    logging.basicConfig()
    log = logging.getLogger('mpdTXT2XML')
    log.setLevel(logging.INFO)


    # get environment settings
    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    

    # parse commandline
    #  - code stolen from: http://python.active-venture.com/lib/module-getopt.html
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.exception("getopt exception: "+__doc__)
        sys.exit(-1)

    for o, a in opts:
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

    if (len(args) != 2):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)
        
    inPath = args[0]
    outPath = args[1]

    # read input file
    inFile = open(inPath, 'r')

    lines = inFile.readlines()

    outData = numarray.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_FE, 8),
                            numarray.Float32)


    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue


        (twr, lyr, col, diode, mpd, sig)= line.split()
        # convert array index values to integer.
        twr = int(twr)
        lyr = int(lyr)
        col = int(col)
        diode = int(diode)
        mpd = float(mpd)
        sig = float(sig)

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))

        valIdx = cgc_util.mpdBigValIdx
        sigIdx = cgc_util.mpdBigSigIdx
        if (diode == calConstant.CDIODE_SM):
            valIdx = cgc_util.mpdSmallValIdx
            sigIdx = cgc_util.mpdSmallSigIdx
            
        outData[twr, row, col, valIdx] = mpd
        outData[twr, row, col, sigIdx] = sig

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calMevPerDacCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)


    sys.exit(0)                            

    
