/*  
Create a set of histograms to allow analysis of the effective area
*/
#include "PSF.h"

#include <fstream>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Aeff : public PSF {
public:
    Aeff(std::string filename): PSF(filename)
        , m_binsize(0.125)
        , m_emin(0.016), m_emax(160.0)
        , m_ngen(4.66e6)
        , m_zdir_bins(20)
        , m_target_area(6.0)
    {}

    // override this to project only the summary plots of energy for each angle range
    void project() 
    {
        open_input_file();
        TFile  hist_file(summary_filename().c_str(), "recreate"); // for the histograms
        std::cout << " writing aeff plots to " << summary_filename() << std::endl;
        int nbins=40; double xmin=0, xmax=2.0, ymax=0.15;

        for(int i=0; i<angle_bins; ++i){
            // loop over angle ranges
           char title[256];  sprintf(title,
               "Energy distirbution: for angles  %2d-%2d degrees", angles[i], angles[i+1]);
            // histogram to show energy distribution for each angle
            TH1F* h = new TH1F(hist_name(i,99), title, (5.5-1.0)/m_binsize, 1.0, 5.5);
            printf("\tProjecting angle range %s\n",angle_cut(i));
            m_tree->Project(h->GetName(), "McLogEnergy", goodEvent&& TCut(angle_cut(i)));
            h->SetDirectory(&hist_file);  // move to the summary file
        }
        for(int j=0; j<energy_bins; ++j){
           char title[256];  sprintf(title,
               "cos(theta) for %d MeV" , int(eCenter(j)+0.5));
            // histogram to show energy distribution for each angle
            TH1F* h = new TH1F(hist_name(0,j), title, m_zdir_bins, -1, 0.);
            printf("\tProjecting %s\n", title);
            m_tree->Project(h->GetName(), "McZDir", goodEvent && TCut(energy_cut(j)));
            h->SetDirectory(&hist_file);  // move to the summary file
        }

        hist_file.Write();
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void draw(std::string ps, std::string page_title, std::string hist_title)
    {
        TFile hist_file(summary_filename().c_str() ); // for the histograms
        TCanvas c;
        c.SetFillColor(10);
        divideCanvas(c,1,1,page_title + "plots from "+summary_filename());
        c.cd(1);
        TLegend* leg=new TLegend(0.13,0.7, 0.30,0.89);
        leg->SetHeader("Angle ranges");
        leg->SetFillColor(10);
        leg->SetBorderSize(1);
        leg->SetTextSize(0.04);

        // determine normalization factor to Aeff
        int ngen=4.66e6; 
        double anglebin=0.2, 
            target_area=6., 
            emax=160., emin=0.016;
        double norm_factor=target_area/m_ngen/anglebin/(m_binsize/log10(m_emax/m_emin));;
        std::cout << "Applying normailzation factor assuming " << m_ngen 
            << " generated uniformly over:"
            << "\n\t cos theta from 0 to 1"
            << "\n\t log E with E from "<< m_emin << " to " << m_emax << std::endl;

        for(int i=0; i<4; ++i){

            TH1F* h =(TH1F*)hist_file.Get(hist_name(i,99)) ; 
            if(h==0){
                std::cerr << "could not find "<< hist_name(i,99) << " in summary file " << hist_file.GetName() <<std::endl;
                return;
            }
            printf("\tdrawing %s\n", h->GetTitle());
            h->Sumw2(); // needed to preserve errors
            h->Scale(norm_factor);
            h->SetMaximum(1.0);
            h->SetLineColor(i+1);
            h->SetStats(false);
            h->SetTitle(hist_title.c_str());
            h->GetXaxis()->SetTitle("log(Egen/ 1MeV)");
            h->GetXaxis()->CenterTitle(true);
            h->GetYaxis()->SetTitle("Aeff (m^2)");
            h->GetYaxis()->CenterTitle(true);

            h->SetLineWidth(2);

            if(i==0)h->Draw(); else h->Draw("same");
            char entry[16]; sprintf(entry," %2d-%d ", angles[i], angles[i+1] );
            leg->AddEntry( h, entry, "l");
        }
        leg->Draw();
        c.Print(ps.c_str() );

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void drawAngles(std::string ps)
    {
        TFile hist_file(summary_filename().c_str() ); // for the histograms
        TCanvas c;
        int nrows=2;
        divideCanvas(c,energy_bins/nrows, nrows, "plots from "+summary_filename());

        for(int j=0; j<energy_bins; ++j){
            c.cd(j+1);
            gPad->SetRightMargin(0.02);
            gPad->SetTopMargin(0.03);
            TH1F* h = (TH1F*)hist_file.Get(hist_name(0,j));
            if( h==0) { 
                std::cerr << "could not find hist " 
                    << hist_name(0,j) << std::endl;
                return;
            }
            h->SetStats(false);
            double norm_factor=m_target_area/m_ngen*m_zdir_bins/(logedelta/log10(m_emax/m_emin));;

            std::cout << "\ndrawing " << h->GetTitle() << std::endl;

            h->Sumw2(); // needed to preserve errors
            h->Scale(norm_factor);
            h->SetMaximum(1.2);
            char title[16]; sprintf(title,"%d MeV", int(eCenter(j)+0.5));
            h->SetTitle(title);
            h->GetXaxis()->SetTitle("-cos(theta)");
            h->GetXaxis()->CenterTitle(true);
            h->GetYaxis()->SetTitle("Aeff (m^2)");
            h->GetYaxis()->CenterTitle(true);
            h->Draw(); 
        }
        c.Print(ps.c_str() );

    }
    double m_binsize;
    double m_emin, m_emax;
    int m_ngen;
    double m_target_area;
    int m_zdir_bins;

};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main(){
    Aeff  all("aeff.root");
    std::string psfile(all.output_file_root()+"aeff.ps");

   if( ! all.fileExists())
        all.project();
    all.draw(psfile+"(", "", "Effective Area vs. energy");
    all.drawAngles(psfile+")");
#if 0
    Aeff cut("cut_aeff.root");
    cut.set_user_cut(TCut("BkVeto==0"));
   if( ! cut.fileExists())
        cut.project();
    cut.draw(psfile+")", "", "Effective Area vs energy with background cuts");
#endif
    return 0;
}
