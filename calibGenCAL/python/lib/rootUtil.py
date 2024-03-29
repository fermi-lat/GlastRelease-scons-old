import sys

"""
collection of PyROOT based utilities
"""

__facility__  = "Offline"
__abstract__  = "collection of PyROOT based utilities"
__author__    = "Z.Fewtrell"
__date__      = "$Date$"
__version__   = "$Revision$, $Author$"
__release__   = "$Name$"
__credits__   = "NRL code 7650"

import ROOT

def colz_plot_hist(hist):
    """
    Draw 2D hist to canvas using "colZ", return canvas
    """
    c = ROOT.TCanvas(hist.GetName()+"_color_scatter",
                     hist.GetTitle()+"_color_scatter",
                     -1)
    c.SetGrid()
    hist.Draw("colZ")
    return c
