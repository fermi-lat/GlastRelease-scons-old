"""
Tool to generate CAL offline cidac2adc (intNonlin) calibration XML file from space delimited TXT file.
Note: inlTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).

inlTXT2XML [-doptional.dtd] input.txt output.xml

where:
    -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
    input.txt  = input txt file, space delimited in following format:
        twr lyr col face erange adc dac
    output.xml = properly formatted offline CAL cidac2adc XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL CIDAC2ADC calibration XML files from TXT."
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
    dtdName    = "calCalib_v2r3.dtd" #default value
    nTXTFields = 7

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('inlTXT2XML')
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

    # define output arrays
    # start w/ empty lists
    # eash array is a list of 4 big numeric arrays
    lengthData = []
    dacData = []
    adcData = []
    for rng in range(4):
        lengthData.append(numarray.zeros((calConstant.NUM_TEM,
                                         calConstant.NUM_ROW,
                                         calConstant.NUM_END,
                                         calConstant.NUM_FE,
                                         1),
                                        numarray.Int32))

        dacData.append(numarray.zeros((calConstant.NUM_TEM,
                                         calConstant.NUM_ROW,
                                         calConstant.NUM_END,
                                         calConstant.NUM_FE,
                                         cgc_util.N_DAC_PTS),
                                        numarray.Float32))

        adcData.append(numarray.zeros((calConstant.NUM_TEM,
                                         calConstant.NUM_ROW,
                                         calConstant.NUM_END,
                                         calConstant.NUM_FE,
                                         cgc_util.N_DAC_PTS),
                                        numarray.Float32))


    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue


        vals = line.split()

        if len(vals) != nTXTFields :
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face erange adc dac]")
            sys.exit(-1)

        # convert vals array to floats instead of strings
        for i in range(len(vals)):
            vals[i] = float(vals[i])

        (twr, lyr, col, face, rng, dac, adc) = vals

        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)
        rng  = int(rng)

        # convert offline face numbering to online face numbering
        face = calConstant.offline_face_to_online[face]
        # also convert layer2row
        row = calCalibXML.layerToRow(int(lyr))

        # make sure current tower is on list
        twrSet.add(twr)

        # populate arrays
        lengthData[rng][twr, row, face, col, 0] += 1
        length = lengthData[rng][twr, row, face, col]
        dacData[rng][twr, row, face, col, length-1] = dac
        adcData[rng][twr, row, face, col, length-1] = adc

    log.info('Writing output file %s', outPath)

    # WARNING! semi-hack!
    # calibUtil dtd file does not allow for intNonlin channels w/ zero entries.
    # I need this in some cases (for instance diagnostic tests for HE only would
    # require that LE channels be empty.
    #
    # the way I'm going to fix it is to make empty channels have a single zero point
    # (dac = 0, adc = 0) instead of not points at all.  shouldn't break too much :(
    for rng in range(4):
        zero_channels = (lengthData[rng] == 0)
        lengthData[rng] = numarray.where(zero_channels, 1, lengthData[rng])

    outFile = calCalibXML.calIntNonlinCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(lengthData, dacData, adcData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)


    sys.exit(0)                            


