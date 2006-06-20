"""
Generate gain (ADC->energy) settings for use by flight software.  The output is a file named cal_gains.h.
This file will be put in the FSW CxG_DB package for incorporation into the on-board loads.
The command line is:

genFlightPed [-v] [-k <key>] <gain_xml_file>

where:
    -v               = verbose; turn on debug output
    -o               = set ORIGIN macro value to 'ON_ORBIT'; default is 'PRELAUNCH'
    -k <key>         = value for KEY macro; default is '0'  
    <gain_xml_file>  = The CAL_xxx calibration XML file for obtaining values.
"""


__facility__    = "Offline"
__abstract__    = "Generate gain (ADC->energy) settings for use by flight software"
__author__      = "D.L.Wood"
__date__        = "$Date$"
__version__     = "$Revision$, $Author$"
__release__     = "$Name$"
__credits__     = "NRL code 7650"



import sys
import time
import getopt
import logging

import calCalibXML
import calConstant



END_MAP = ('N', 'P')


if __name__ == '__main__':
    
    usage = "genFlightGain [-v] [-o] [-k <key>] <gain_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genFlightGain')
    log.setLevel(logging.INFO)

    key = 0
    origin = 'PRELAUNCH'
    
    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-v-o-k:")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-v':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-o':
            origin = 'ON_ORBIT'    
        elif o[0] == '-k':
            key = int(o[1])    
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)

    inName = args[0]
 
    # read gain values file
    
    log.info("Reading gains file %s", inName)
    fio = calCalibXML.calMuSlopeCalibXML(inName)
    gains = fio.read()
    fio.close()
    
    # create output file

    outName = 'cal_gains.h'
    log.info('Writing output file %s', outName)
    fio = open(outName, 'w')
    
    ts = time.strftime('%y-%m-%d %H:%M:%S')
    
    rel = __release__[7:-2].strip()
    if len(rel) == 0:
        ver = 'dev'
    else:
        ver = rel      
    
    fio.write('// %s\n' % outName)
    fio.write('// On-board calorimeter gain (ADC->energy) values\n')
    fio.write('// Generated by genFlightGain, calibGenCAL version %s, on %s\n' % (ver, ts)) 
    fio.write('// Input calibration file: %s\n\n' % inName) 
    fio.write('// CAL_MuSlope input file is documented in LAT Calibration Metadata Database hosted at SLAC.\n')
    fio.write('// The KEY macro value contained in this file references the metadata row entry.\n')
    fio.write('// Database:   calib\n')
    fio.write('// Schema:     metadata_v2r1\n')
    fio.write('// ser_no:     %d\n' % key) 
    fio.write('\n\n')
    
    fio.write('// $Id$\n\n')
    
    # create preamble macros
    
    fio.write('#define SUBSYSTEM    CAL\n')
    fio.write('#define TYPE         GAINS\n')
    fio.write('#define STYLE        FULL\n')
    fio.write('#define ORIGIN       %s\n' % origin)
    fio.write('#define KEY          0x%08x\n\n' % key)
    
    # create gain value macros
    
    for tem in range(calConstant.NUM_TEM):
        for row in range(calConstant.NUM_ROW):
            for fe in range(calConstant.NUM_FE):
                for end in range(calConstant.NUM_END):
                    for erng in range(calConstant.NUM_RNG):
                        
                        g = gains[tem, row, end, fe, erng, 0]
                        s = '#define GAIN_T%1X_%s_C%1X_R%d_%s    %f\n' % (tem, calConstant.CROW[row], fe,
                            erng, END_MAP[end], g)
                        fio.write(s)
    
    fio.close()

    sys.exit(0)
    
    

   
