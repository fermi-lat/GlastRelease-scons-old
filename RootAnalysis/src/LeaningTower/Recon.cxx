#include "Recon.h"

#include "TF1.h"

Recon::Recon(TFile* file) {
    reconTree = (TTree*)file->Get("Recon");
    reconTree->SetBranchAddress("TkrNumClus",     &TkrNumClus);
    reconTree->SetBranchAddress("TkrNumTracks",   &TkrNumTracks);
    reconTree->SetBranchAddress("TkrTrk1NumClus", &TkrTrk1NumClus);
    reconTree->SetBranchAddress("TkrClusX",        TkrClusX);
    reconTree->SetBranchAddress("TkrClusY",        TkrClusY);
    reconTree->SetBranchAddress("TkrClusZ",        TkrClusZ);
    reconTree->SetBranchAddress("TkrClusLayer",    TkrClusLayer);
    reconTree->SetBranchAddress("TkrClusView",     TkrClusView);
    reconTree->SetBranchAddress("TkrTrk1Clusters", TkrTrk1Clusters);
}

void Recon::GetEvent(int event) {
    int bytes = reconTree->GetEntry(event);
    if ( bytes == 0 )
        std::cerr << "Recon::GetEvent: couldn't read the Recon part!" << std::endl;
    for ( int i=0; i<TkrNumClus; ++i ) {
        TkrClusX[i] = TkrClusX[i] + 740;
        TkrClusY[i] = TkrClusY[i] + 740;
        TkrClusZ[i] = TkrClusZ[i] +  18;
    }
}

TGraph Recon::GetAllClustersGraph(const TString view, const int notLayer) const {
    if ( view == "X" )
        return GetAllClustersGraph(0, notLayer);
    else if ( view == "Y" )
        return GetAllClustersGraph(1, notLayer);
    else
        std::cerr << "Recon::GetAllClustersGraph: view = " << view << std::endl;

    std::exit(42);
    return TGraph();
}

TGraph Recon::GetAllClustersGraph(const int view, const int notLayer) const {
    TGraph clusters;
    for ( int i=0; i<GetTkrNumClus(); ++i ) {
        if ( TkrClusView[i] == view && TkrClusLayer[i] != notLayer ) {
            double pos;
            switch ( view ) {
            case 0:
                pos = TkrClusX[i];
                break;
            case 1:
                pos = TkrClusY[i];
                break;
            default:
                pos = 0;
                std::cerr << "Recon::GetAllClustersGraph: view = " << view << std::endl;
                std::exit(42);
            }
            clusters.SetPoint(clusters.GetN(), pos, TkrClusZ[i] );
        }
    }
    return clusters;
}

TGraph Recon::GetClusterGraph(const std::vector<TString> planeCol, bool* exactlyOne) const {
    TGraph tg;
    if ( planeCol.size() == 0 )  // this is a bad plane
        return tg;

    *exactlyOne = true;
    for ( std::vector<TString>::const_iterator it=planeCol.begin(); it<planeCol.end(); ++it ) {
        // loop over the names of reference planes selected for fitting
        int count = 0;
        const int refView = GetView(*it);
        const int refReconLayer = GetReconLayer(*it);
        for ( int j=0; j<GetTkrNumClus(); ++j ) {
            // a second loop over all clusters, to select those on ref planes
            if (  refReconLayer != TkrClusLayer[j] || refView != TkrClusView[j] )
                continue; // cluster is not on this ref plane
            ++count;
            double pos = 0;
            if ( refView == 0 )
                pos = TkrClusX[j];
            else if ( refView == 1 )
                pos = TkrClusY[j];
            tg.SetPoint(tg.GetN(), pos, TkrClusZ[j] );
        }
        if ( count != 1 )
            *exactlyOne = false;
    }
    return tg;
}

TGraph Recon::GetTrk1ClustersGraph(const TString view, const int notLayer) const {
    if ( view == "X" )
        return GetTrk1ClustersGraph(0, notLayer);
    else if ( view == "Y" )
        return GetTrk1ClustersGraph(1, notLayer);
    else
        std::cerr << "Recon::GetTrk1ClustersGraph: view = " << view << std::endl;

    std::exit(42);
    return TGraph();
}

TGraph Recon::GetTrk1ClustersGraph(const int view, const int notLayer) const {
    TGraph clusters;
    for ( int j=0; j<GetTkrTrk1NumClus(); ++j ) {
        int i = TkrTrk1Clusters[j];
        if ( TkrClusView[i] == view && TkrClusLayer[i] != notLayer ) {
            double pos;
            switch ( view ) {
            case 0:
                pos = TkrClusX[i];
                break;
            case 1:
                pos = TkrClusY[i];
                break;
            default:
                pos = 0;
                std::cerr << "Recon::GetTrk1ClustersGraph: view = " << view << std::endl;
                std::exit(42);
            }
            clusters.SetPoint(clusters.GetN(), pos, TkrClusZ[i] );
        }
    }
    return clusters;
}


TLine Reconstruct(const TGraph* XY, const bool verbose) {
    // LineStyle 0: less than two points
    // LineStyle 1: fit with good chi2
    // LineStyle 2: fit with bad chi2

    int N = XY->GetN();
    if ( N < 2 ) {
        TLine invalid;
        invalid.SetLineStyle(0);
        return invalid;
    }

    double* X = XY->GetX();
    double* Y = XY->GetY();
    double MinChi2 = 1E6;
    double A = 0;
    double B = 0;

    if ( N <= 3 ) {
        // swapping columns.  This improves the fit!
        TGraph g(N, Y, X);
        g.Fit("pol1", "Q");
        TF1* f = (TF1*)g.FindObject("pol1");
        MinChi2 = f->GetChisquare();      
        A = f->GetParameter(0);
        B = f->GetParameter(1);
        delete f;
    }
    else {
        for ( int i=0; i<N ; ++i ) {
            // swapping columns.  This improves the fit!
            TGraph g(N, Y, X);
            g.RemovePoint(i);
            g.Fit("pol1", "Q");
            TF1* f = (TF1*)g.FindObject("pol1");
            const double chi2 = f->GetChisquare();
            if ( chi2 < MinChi2 ) {
                MinChi2 = chi2;
                A = f->GetParameter(0);
                B = f->GetParameter(1);
            }
            delete f;
        }
    }

    const double Y1 = 0.0;
    const double Y2 = 700.0;
    // remember: x and y are swapped!
    const double x1 = A + B * Y1;
    const double y1 = Y1;
    const double x2 = A + B * Y2;
    const double y2 = Y2;
    //    std::cout << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << std::endl;

    TLine track(x1, y1, x2, y2);
    track.SetLineColor(2);

    static const double MaxChi2 = 1;
    if ( verbose )
        std::cout << "chisqr " << MinChi2 << std::endl;
    if ( MinChi2 > MaxChi2 )
        track.SetLineStyle(2);

    return track;
}

double ExtrapolateCoordinate(const TLine& track, const double z) {
    const double x1 = track.GetX1();
    const double x2 = track.GetX2();
    const double y1 = track.GetY1();
    const double y2 = track.GetY2();
    
    const double newX = ( x2 == x1 )  ?  x1  :  x1 + ( z - y1 ) / ( y2 - y1 ) * ( x2 - x1 );
    return newX;
}

ClassImp(Recon)
