"""
Tool to merge mutilple single tower CAL IntNonlin calibration XML files into a
single output file.  The command line is:

intNonlinMerge [-V] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The merged CAL Int_Nonlin calibration XML file to output.
"""


__facility__  = "Offline"
__abstract__  = "Tool to merge mutilple CAL IntNonlin calibration XML files."
__author__    = "D.L.Wood"
__date__      = "$Date$"
__version__   = "$Revision$, $Author$"
__release__   = "$Name$"
__credits__   = "NRL code 7650"


import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calCalibXML
import calConstant



class inputFile:
    """
    Represents one intNonlinMerge input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name, dacData, adcData):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        Param: dacData A list of Numeric DAC data arrays from the input file.
        Param: dacData A list of Numeric ADC data arrays from the input file.
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name
        self.dacData = dacData
        self.adcData = adcData



#######################################################################################3


if __name__ == '__main__':

    usage = "intNonlinMerge [-V] <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('intNonlinMerge')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)        
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    outName = args[1]
    

    # get environment settings

    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)


    # read config file settings

    log.info("Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("Config file %s missing or empty" % configName)
        sys.exit(1)

    # get input file names and tower ID's

    if 'infiles' not in sections:
        log.error("Config file %s missing [infiles] section" % configName)
        sys.exit(1)

    if 'dtdfiles' not in sections:
        log.error("Config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)    

    inFiles = []
    options = configFile.options('infiles')
    for opt in options:
        optList = opt.split('_')
        if len(optList) != 2 or optList[0] != 'intnonlin':
            continue
        destTwr = int(optList[1])
        if destTwr < 0 or destTwr > 15:
            log.error("Index for [infiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
        value = configFile.get('infiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 1:
            name = nameList[0]
            srcTwr = 0
        elif nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [infiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)    
        inFile = inputFile(srcTwr, destTwr, name, None, None)
        inFiles.append(inFile)
        log.debug('Adding file %s to input as tower %d', name, destTwr)


    # get DTD file name

    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("Config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)
    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))
    

    # read input files

    firstFile = True
    for f in inFiles:
        log.info('Reading file %s', f.name)
        inFile = calCalibXML.calIntNonlinCalibXML(f.name)
        (dacData, adcData) = inFile.read()
        f.adcData = adcData
        f.dacData = dacData
        if firstFile:
            info = inFile.info()
            firstFile = False
        inFile.close()

    log.debug('Using ouput info:\n%s', str(info))


    # merge tower DAC data

    dacDataOut = [None, None, None, None]
    dacDataLen = [None, None, None, None]
    
    for erng in range(4):
        maxX = 0
        for f in inFiles:
            s = len(f.dacData[erng])
            if s > maxX:
                maxX = s
                dacDataOut[erng] = f.dacData[erng]
                dacDataLen[erng] = s
                log.debug('Using ouput DAC values for range %s:\n%s', calConstant.CRNG[erng], \
                          dacDataOut[erng])
            

    # create empty output ADC data array

    adcDataOut = [None, None, None, None]
    for erng in range(4):
        adcDataOut[erng] = Numeric.zeros((16, 8, 2, 12, dacDataLen[erng]), Numeric.Float32)
        log.debug('Using output ADC array shape %s for range %s', str(adcDataOut[erng].shape),
                  calConstant.CRNG[erng])
                

    # merge tower ADC data

    for erng in range(4):
        for f in inFiles:
            adcData = f.adcData
            inData = adcData[erng]
            outData = adcDataOut[erng]
            for row in range(8):
                for end in range(2):
                    for fe in range(12):
                        inSize = inData.shape[-1]
                        outSize = outData.shape[-1]
                        for dac in range(inSize):
                            x = inData[f.srcTwr, row, end, fe, dac]
                            outData[f.destTwr, row, end, fe, dac] = x
                        if outSize > inSize:
                            for dac in range(inSize, outSize):
                               outData[f.destTwr, row, end, fe, dac] = -1.0 
                            
            

    log.info('Writing output file %s', outName)

    temList = []
    for f in inFiles:
        temList.append(f.destTwr)

    outFile = calCalibXML.calIntNonlinCalibXML(outName, calCalibXML.MODE_CREATE)
    outFile.write(dacDataOut, adcDataOut, inputSample = None, tems = temList)
    outFile.close()


    # fixup calibration XML file - insert DTD info

    outFile = open(outName, 'r')
    lines = outFile.readlines()
    outFile.close()

    dtdStr = '<!DOCTYPE calCalib ['
    dtdFile = open(dtdName, 'r')
    dtdLines = dtdFile.readlines()
    dtdFile.close()
    for l in dtdLines:
        dtdStr += l
    dtdStr += ']>\n'

    outFile = open(outName, 'w')
    outFile.write(lines[0])
    outFile.write(dtdStr)
    outFile.writelines(lines[1:])
    outFile.close()

    sys.exit(0)

