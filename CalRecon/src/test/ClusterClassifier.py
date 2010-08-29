import ROOT

ROOT.gStyle.SetCanvasColor(0)
ROOT.gStyle.SetPalette(1)
ROOT.gStyle.SetPaintTextFormat('.3f')
ROOT.gStyle.SetOptStat(0)


LOG_E_MIN = 1
LOG_E_MAX = 6
NUM_E_BINS = 10
PRE_CUT   = 'CalNumXtals >= 4'


class ClusterVariable:

    def __init__(self, expression, minValue, maxValue, numBins = 50,
                 label = None):
        self.Expression = expression
        self.MinValue = minValue
        self.MaxValue = maxValue
        self.NumBins = numBins
        self.Label = label or expression

    def __str__(self):
        return 'Variable "%s" (%.3f--%.3f in %d bins)' %\
               (self.Expression, self.MinValue, self.MaxValue, self.NumBins)
        
    

VARIABLE_LIST  = [ClusterVariable('CalTransRms', 0, 100),
                  ClusterVariable('CalLRmsAsym', 0, 0.25),
                  ClusterVariable('CalNumXtals/log10(CalEnergyRaw)',
                                  0, 150, label = 'NumXtals')]

FILE_PATH_DICT = {'gamma': '/data/mc/allGamma-GR-v18r4p2-FAKEOVRLY/allGamma-GR-v18r4p2-FAKEOVRLY-Simple_merit.root',
                  'had'  : '/data/mc/bkg-GR-v17r31/Bkg_1_v17r31_merit.root'
                  #'had'  : '/data44/mc/backgnd-GR-v15r39p1-Day-NoP/backgnd-GR-v15r39p1-Day-NoP-merit_04900_05000-100-QualityCut-unbiased-CREClassMCv1p12-fix2-merit.root'
                  #'mip'  : 'mips_*_merit.root'
                  }
COLORS_DICT = {'gamma': ROOT.kRed,
               'had'  : ROOT.kBlue,
               'mip'  : ROOT.kBlack
               }


def hname(label, topology):
    return 'fPdf_%s_%s' % (label, topology)

def getColor(topology):
    try:
        return COLORS_DICT[topology]
    except KeyError:
        return ROOT.kBlack


OBJECT_POOL = {}

def toPool(rootObject):
    OBJECT_POOL[rootObject.GetName()] = rootObject


class ClusterClassifier:

    def __init__(self):
        print 'Opening files...'
        self.RootTreeDict = {}
        for (topology, filePath) in FILE_PATH_DICT.items():
            self.RootTreeDict[topology] = ROOT.TChain('MeritTuple')
            self.RootTreeDict[topology].Add(filePath)
        print 'Creating histograms for pdfs...'
        self.PdfHistDict = {}
        self.PdfHistSliceDict = {}
        for topology in FILE_PATH_DICT.keys():
            self.PdfHistDict[topology] = {}
            self.PdfHistSliceDict[topology] = {}
            for var in VARIABLE_LIST:
                print 'Processing %s for %s' % (var, topology)
                self.__createPdfHist(var, topology)
        print 'Done.'

    def __createPdfHist(self, var, topology):
        # Create the two-dimensional histogram...
        hName = hname(var.Label, topology)
        hTitle = '%s p.d.f. (%s)' % (var.Expression, topology)
        h = ROOT.TH2F(hName, hTitle, NUM_E_BINS, LOG_E_MIN, LOG_E_MAX,
                      var.NumBins, var.MinValue, var.MaxValue)
        self.PdfHistDict[topology][var.Label] = h
        expr = '%s:log10(CalEnergyRaw)' % var.Expression
        cut = PRE_CUT
        self.RootTreeDict[topology].Project(hName, expr, cut)
        # ... then normalize the vertical slices.
        for i in xrange(1, NUM_E_BINS + 1):
            sum = 0.0
            for j in xrange(1, var.NumBins + 1):
                sum += h.GetBinContent(i, j)
            for j in xrange(1, var.NumBins + 1):
                try:
                    value = h.GetBinContent(i, j)/sum
                    h.SetBinContent(i, j, value)
                except ZeroDivisionError:
                    pass
            h.SetBinContent(i, 0, 0.0)
            h.SetBinContent(i, var.NumBins + 1, 0.0)
        h.SetMinimum(1e-3)
        h.SetMaximum(1)
        # ... finally create a TH1 for ewach slice.
        self.PdfHistSliceDict[topology][var.Label] = {}
        for i in range(NUM_E_BINS):
            hSlice = h.ProjectionY('%s_slice%d' % (h.GetName(), i), i+1, i+1)
            hSlice.SetLineColor(getColor(topology))
            hSlice.GetXaxis().SetLabelSize(0.07)
            hSlice.GetYaxis().SetLabelSize(0.07)
            self.PdfHistSliceDict[topology][var.Label][i] = hSlice

    def drawAllPdfHists(self):
        for var in VARIABLE_LIST:
            self.drawPdfHists(var)

    def drawPdfHists(self, var):
        cName  = '%s_2d' % var.Label
        cTitle = '%s (2d)' % var.Expression
        c = ROOT.TCanvas(cName, cTitle, 1000, 500)
        toPool(c)
        c.Divide(2, 1)
        for (i, topology) in enumerate(FILE_PATH_DICT.keys()):
            c.cd(i + 1)
            self.getPdfHist(topology, var).Draw('colz,text')
            ROOT.gPad.SetLogz(True)
        c.cd()
        c.Update()
        cName = '%s_slices' % var.Label
        cTitle = '%s (slices)' % var.Expression
        c = ROOT.TCanvas(cName, cTitle, 1000, 500)
        toPool(c)
        c.Divide(4, 3)
        for i in range(NUM_E_BINS):
            legend = ROOT.TLegend(0.65, 0.67, 0.90, 0.85)
            legend.SetName('%s_legend_slice%d' % (var.Label, i))
            legend.SetFillStyle(0)
            legend.SetLineStyle(0)
            legend.SetLineWidth(0)
            legend.SetBorderSize(0)
            legend.SetTextSize(0.08)
            toPool(legend)
            c.cd(i + 1)
            ymax = 0
            for (j, topology) in enumerate(FILE_PATH_DICT.keys()):
                hSlice = self.getPdfSliceHist(topology, var, i)
                y = hSlice.GetMaximum()
                if y > ymax:
                    ymax = y
            for (j, topology) in enumerate(FILE_PATH_DICT.keys()):
                hSlice = self.getPdfSliceHist(topology, var, i)
                hSlice.SetMaximum(1.2*ymax)
                hSlice.Draw('same'*(j!=0))
                legend.AddEntry(hSlice, topology)
            legend.Draw()
            logemin = LOG_E_MIN  +\
                      i*float(LOG_E_MAX - LOG_E_MIN)/float(NUM_E_BINS)
            logemax = LOG_E_MIN  +\
                      (i + 1)*float(LOG_E_MAX - LOG_E_MIN)/float(NUM_E_BINS)
            emin = (10**logemin)
            emax = (10**logemax)
            label = ROOT.TLatex(0.15, 0.8, '%.d--%d MeV' %\
                                (emin, emax))
            label.SetName('%s_label_slice%d' % (var.Label, i))
            label.SetTextSize(0.07)
            label.SetNDC()
            toPool(label)
            label.Draw()
        c.cd()
        c.Update()

    def getPdfHist(self, topology, var):
        return self.PdfHistDict[topology][var.Label]

    def getPdfSliceHist(self, topology, var, i):
        return self.PdfHistSliceDict[topology][var.Label][i]

    def writeOutputFile(self, filePath):
        print 'Writing output file %s...' % filePath
        outputFile = ROOT.TFile(filePath, 'RECREATE')
        for topology in FILE_PATH_DICT.keys():
            for var in VARIABLE_LIST:
                self.getPdfHist(topology, var).Write()
        outputFile.Close()
        print 'Done.'





if __name__ == '__main__':
    c = ClusterClassifier()
    c.drawAllPdfHists()
    c.writeOutputFile('cluclass.root')
