#include <string>
#include <iostream>
#include <fstream>
#include "SpectObj/SpectObj.h"

const double erg2meV   = 624151.0;


SpectObj::SpectObj(const TH2D* In_Nv, int type) //Max
{
  gDirectory->Delete("newNv");
  Nv   = (TH2D*)In_Nv->Clone(); // ph/kev/s/m�
  Nv->SetName("newNv");
  ne   = Nv->GetNbinsY();
  sourceType = type; //Max
  

  double* en   = new double[ne+1];
  emin = Nv->GetYaxis()->GetXmin();
  emax = Nv->GetYaxis()->GetXmax();
  
  nt   = Nv->GetNbinsX();
  tmin = Nv->GetXaxis()->GetXmin();
  tmax = Nv->GetXaxis()->GetXmax();
  deltat   = Nv->GetXaxis()->GetBinWidth(0);
  
  for(int ei = 0 ; ei<=ne ; ei++) 
    {      
      en[ei] =  Nv->GetYaxis()->GetBinLowEdge(ei+1);
    }
  
  double dei;
  for (int ei = 0; ei<ne; ei++)
    {
      dei   = Nv->GetYaxis()->GetBinWidth(ei+1);
      for(int ti = 0; ti<nt; ti++)
	{
	  Nv->SetBinContent(ti+1, ei+1, 
			    Nv->GetBinContent(ti+1, ei+1)*dei*deltat); //[ph/m�]
	}  
    }
  gDirectory->Delete("spec");
  gDirectory->Delete("times");
  spec  = new TH1D("spec","spec",ne,en);
  times = new TH1D("times","times",nt+1,tmin-deltat/2.,tmax+deltat/2.);
  delete en;
  std::cout << "Selected type source " << sourceType << std::endl;
  std::cout<<" GRB SpectObj initialized !"<<std::endl;
  //////////////////////////////////////////////////
}
//////////////////////////////////////////////////
TH1D *SpectObj::GetSpectrum(double t)
{
  if (t == 0.0) return spec;
  int ti = Nv->GetXaxis()->FindBin(t);
  gDirectory->Delete("sp");
  TH1D *sp = (TH1D*) spec->Clone();
  sp->SetName("sp");
  for(int ei = 1; ei <= ne; ei++)
    sp->SetBinContent(ei,Nv->GetBinContent(ti,ei));
  return sp; //ph/m�
}

TH1D *SpectObj::GetTimes(double en)
{
  if (en == 0.0) return times;
  int ei = Nv->GetYaxis()->FindBin(en);
  gDirectory->Delete("lc");
  TH1D *lc = (TH1D*) times->Clone();
  lc->SetName("lc");
  for(int ti = 1; ti <= nt; ti++)
    lc->SetBinContent(ti,Nv->GetBinContent(ti,ei));
  return lc; //ph/m�
}

//////////////////////////////////////////////////
TH1D *SpectObj::Integral_E(double e1, double e2)
{
  int ei1 = Nv->GetYaxis()->FindBin(e1);
  int ei2 = Nv->GetYaxis()->FindBin(e2); 
  return Integral_E(ei1,ei2);
}

TH1D *SpectObj::Integral_E(int ei1, int ei2)
{
  //  gDirectory->Delete("ts");
  TH1D *ts = (TH1D*) times->Clone();
  ts->SetName("ts");  
  
  for(int i = 0; i<nt; i++)
    ts->SetBinContent(i+1,Nv->Integral(i+1,i+1,ei1,ei2));
  return ts; //ph/m�
}

double SpectObj::Integral_E(TH1* Sp, double e1, double e2)
{
  // Sp in ph/m�
  int ei1 = Sp->FindBin(e1);
  int ei2  = Sp->FindBin(e2);
  return Integral_E(Sp,ei1,ei2); //ph/m�
}

double SpectObj::Integral_E(TH1* Sp, int ei1, int ei2)
{
  // Sp in ph/m�
  return Sp->Integral(ei1,ei2);//ph/m�
}

//////////////////////////////////////////////////
TH1D *SpectObj::Integral_T(double t1, double t2, double en)
{
  //nv is in ph/m�
  int ti1 = Nv->GetXaxis()->FindBin(t1);
  int ti2 = Nv->GetXaxis()->FindBin(t2);
  int ei  = TMath::Max(1,Nv->GetYaxis()->FindBin(en));
  return Integral_T(ti1,ti2,ei); //ph/m�
}
TH1D *SpectObj::Integral_T(double t1, double t2, double e1, double e2)
{
  //nv is in ph/m�
  int ti1 = Nv->GetXaxis()->FindBin(t1);
  int ti2 = Nv->GetXaxis()->FindBin(t2);
  int ei1  = Nv->GetYaxis()->FindBin(e1);
  int ei2  = Nv->GetYaxis()->FindBin(e2);
  return Integral_T(ti1,ti2,ei1,ei2); //ph/m�
}

TH1D *SpectObj::Integral_T(int ti1, int ti2, int e1)
{
  TH1D *en = (TH1D*) spec->Clone();
  en->SetName("en");
  
  for(int i = e1; i<=ne; i++)
    en->SetBinContent(i,Nv->Integral(ti1,ti2,i,i));
  return en; // ph/m�
}

TH1D *SpectObj::Integral_T(int ti1, int ti2, int e1, int e2)
{
  TH1D *en = (TH1D*) spec->Clone();
  en->SetName("en");
  
  for(int i = e1; i<=e2; i++)
    en->SetBinContent(i,Nv->Integral(ti1,ti2,i,i));
  return en; // ph/m�
}

double SpectObj::Integral_T(TH1* Pt, double t1, double t2)
{
  // Pt in ph/m�
  int ti1 = Pt->FindBin(t1);
  int ti2 = Pt->FindBin(t2);
  return Integral_T(Pt,ti1,ti2); //ph/m�
}

double SpectObj::Integral_T(TH1* Pt, int ti1, int ti2)
{
  // Pt in ph/m�
  return Pt->Integral(ti1,ti2); //ph/m�
}

//////////////////////////////////////////////////
TH1D *SpectObj::ComputeProbability(double enph)
{
  
  TH1D *P = (TH1D*) times->Clone(); //ph/m�
  P->SetName("P");
  TH1D *pt = Integral_E(enph,emax); //ph/m�
  for(int ti = 1; ti <= nt; ti++)
    {
      P->SetBinContent(ti,Integral_T(pt,0,ti)); //ph/m�
    }
  delete pt;
  return P; //ph/m�
}

//////////////////////////////////////////////////
photon SpectObj::GetPhoton(double t0, double enph)
{
  photon ph;
  
  if (sourceType == 0 ) // Transient
    {
      double time   = 1.0e8; // > 1 year!
      double energy =  enph;
  
      if(t0 > tmax)
	{
	  ph.time   = time;
	  ph.energy = energy;
	  return ph;
	}
      
      TH1D *P = ComputeProbability(enph); //ph/m�
      int t1 = Nv->GetXaxis()->FindBin(t0);
      int t2 = t1;
      int ei  = TMath::Max(1,Nv->GetYaxis()->FindBin(enph));
      while(P->GetBinContent(t2) - P->GetBinContent(t1) < 1.0 && t2 < nt)
	{
	  t2++;
	} 
      double dp1 = P->GetBinContent(t2) - P->GetBinContent(t2-1);
      double dp2 = 1.0 + P->GetBinContent(t1) - P->GetBinContent(t2-1);
      double dt = Nv->GetXaxis()->GetBinWidth(t2);
      double Dt = Nv->GetXaxis()->GetBinCenter(t2-1)- Nv->GetXaxis()->GetBinCenter(t1);
      
      if(t2 <= nt) // the burst has finished  dp < 1 or dp >=1
	{
	  TH1D* Sp = Integral_T(t1,t2,ei); //ph/m�
	  time    = t0 + Dt + dp2/dp1*dt;//dt/dp + t0;       
	  energy  = Sp->GetRandom();
	}
      ph.time   = time;
      ph.energy = energy;
      delete P;  
    } else if (sourceType == 1) //Periodic //Max
      {
	double TimeToFirstPeriod = 0.0;
	double TimeFromLastPeriod = 0.0;
	int IntNumPer = 0;
	double ProbRest = 0.0;
	double nextTime = 0.0;
	double InternalTime = t0 - Int_t(t0/tmax)*tmax; // InternalTime is t0 reduced to a period
	TH1D *P = ComputeProbability(enph); //ph/m� 
	double Ptot = P->GetBinContent(nt) - P->GetBinContent(1); //Total proability in a period 
	int ei  = TMath::Max(1,Nv->GetYaxis()->FindBin(enph));

        //First checks if next photon lies in the same period
	if (((P->GetBinContent(nt) - P->GetBinContent(Nv->GetXaxis()->FindBin(InternalTime))) < 1.0 ))
	  {
	    TimeToFirstPeriod = tmax - InternalTime;
	    // std::cout << "Time to first period " << TimeToFirstPeriod << std::endl;
	  }
	else
	  {
	    TimeToFirstPeriod = 0.0;
	    std::cout << "Next Photon comes within the same period " << std::endl;
	  }

	// If not, proceed to find the integer number of period required to have Prob = 1
	if (TimeToFirstPeriod != 0.0)
	  {
	    ProbRest = 1 - (P->GetBinContent(nt) - P->GetBinContent(Nv->GetXaxis()->FindBin(InternalTime)));
	    //std::cout << "Computing Integer Periods , Now ProbRest is " << ProbRest << std::endl;
	    IntNumPer = Int_t(ProbRest/Ptot);
	    //std::cout << " Integer Number of Period is " << IntNumPer <<std::endl; 
	  }
	
	// Then checks for the remnant part of the probability
	if (IntNumPer > 0 )
	  {
	    ProbRest = ProbRest - IntNumPer*Ptot;
	    // std::cout << "Computing Residual time , Now ProbRest is " << ProbRest << std::endl;
            int tBinCurrent = 1;
	    while ((P->GetBinContent(tBinCurrent) - P->GetBinContent(1)) < ProbRest ) 
	      {
		tBinCurrent++;
	      }	 
	    TimeFromLastPeriod = TimeFromLastPeriod = Nv->GetXaxis()->GetBinCenter(tBinCurrent-1);
	    ProbRest = ProbRest - (P->GetBinContent(tBinCurrent-1) - P->GetBinContent(1));
	  }

	//std::cout << " At End ProbRest is (should be 0 ) " << ProbRest << std::endl;

	/*
	std::cout << " First Step " << t0 
	 	  << " to " << t0 + TimeToFirstPeriod 
	 	  << " (" << TimeToFirstPeriod << ")" << std::endl;  
 	std::cout << " Second Step " << t0 + TimeToFirstPeriod 
	 	  << " to " << t0 + TimeToFirstPeriod + IntNumPer*tmax 
	          << " (" << IntNumPer*tmax << "," << IntNumPer << " periods)" << std::endl;  
	std::cout << " Third Step " << t0 + TimeToFirstPeriod + IntNumPer*tmax 
		  << " to " << t0 + TimeToFirstPeriod + IntNumPer*tmax + TimeFromLastPeriod 
		  << " (" << TimeFromLastPeriod << ")" << std::endl;  
  	*/   

	// Now evaluate the Spectrum Histogram

 	TH1D* Sp;
	Sp = Integral_T(1,nt,ei);
	Sp->Scale(IntNumPer*1.0);
	Sp->Add(Sp,Integral_T(Nv->GetXaxis()->FindBin(TimeToFirstPeriod),nt,ei));	
	Sp->Add(Sp,Integral_T(1,Nv->GetXaxis()->FindBin(TimeFromLastPeriod),ei));
	ph.time   = t0 + TimeToFirstPeriod + IntNumPer*tmax +TimeFromLastPeriod;
 	ph.energy = Sp->GetRandom();
 	std::cout << " ====> NextTime " << ph.time
	 	  << " | Energy " << ph.energy << " KeV " << std::endl; 
 	delete Sp;
 	delete P;
       }
   return ph;
 }

void SpectObj::ScaleAtBATSE(double fluence)
{
  
  double BATSE1 = 20.0;    // 20 keV
  double BATSE2 = 1.0e+3;  // 1 MeV
  int ei1 = Nv->GetYaxis()->FindBin(BATSE1);
  int ei2 = Nv->GetYaxis()->FindBin(BATSE2);
  double norm = Nv->Integral(0,nt,ei1,ei2,"width")*1.0e-7/(deltat*erg2meV); //KeV/cm�
  Nv->Scale(fluence/norm);
  //return fluence/norm;
}

double SpectObj::GetFluence(double BL, double BH)
{
  if(BH<=0) BH = emax;
  int ei1 = Nv->GetYaxis()->FindBin(TMath::Max(emin,BL));
  int ei2 = Nv->GetYaxis()->FindBin(TMath::Min(emax,BH));
  return Nv->Integral(0,nt,ei1,ei2,"width")*1.0e-7/(deltat*erg2meV); //KeV/cm�
}

double SpectObj::GetT90(double BL, double BH)
{
  if(BL<emin) BL = emin;
  if(BH<=0) BH = emax;
  TH1D* LC     = Integral_E(BL,BH); //ph/m�
  LC->Scale(1.0/LC->Integral());
  int i=1;
  double inte = LC->GetBinContent(i);
  while(inte <= 0.05) 
    {
      inte += LC->GetBinContent(i+1);
      i++;
    }  
  double T05 = TMath::Max(0.0,LC->GetBinCenter(i));
  while(inte <= 0.95) 
    {
      inte += LC->GetBinContent(i+1);
      i++;
    }  
  double T95 = LC->GetBinCenter(i);  
  return T95-T05;
}


//////////////////////////////////////////////////
TH1D *SpectObj::N(TH1D *EN)
{
  TH1D* n = (TH1D*) EN->Clone();
  n->SetName("N");
  for(int i = 1; i <= EN->GetNbinsX();i++)
    n->SetBinContent(i,EN->GetBinContent(i)/EN->GetBinWidth(i));
  return n;
}

//////////////////////////////////////////////////
double SpectObj::flux(double time, double enph)
{
  if (time >= tmax) return 1.0e-6;

  TH1D* fl = GetSpectrum(time);    //ph/m�
  double integral = Integral_E(fl,enph,emax)/deltat; //ph/m�/s
  //  delete fl;
  return integral;//ph/m�/s
}

double SpectObj::interval(double time, double enph)
{
  
  return GetPhoton(time,enph).time - time;
}

double SpectObj::energy(double time, double enph)
{
  return GetPhoton(time,enph).energy;
}

//////////////////////////////////////////////////
void SpectObj::SaveParameters(double tstart, std::pair<double,double> direction)
{
  
  double BATSEL = 20.0;
  double BATSEH = 1.0e+3;

  double GBML = 10.0;
  double GBMH = 25.0e+3;
  
  double LATL = 50.0e+3;
  double LATH = emax;

  double fBATSE = GetFluence(BATSEL,BATSEH);
  double fLAT   = GetFluence(LATL,LATH);
  double fGBM   = GetFluence(GBML,GBMH);
  double fTOT   = GetFluence();

  std::cout<<"**************************************************"<<std::endl;
  std::cout<<" Tstart = "<<tstart<<" T90 = "<<GetT90()<<std::endl;
  std::cout<< " GRB Direction :  l = "<<direction.first<<", b = "<<direction.second<<std::endl;
  std::cout<<" BASTE flux ("<<BATSEL<<","<<BATSEH<<") = "<<fBATSE<<" erg/cm^2"<<std::endl;
  std::cout<<" GBM   flux ("<< GBML <<","<< GBMH <<") = "<<fGBM<<" erg/cm^2"<<std::endl;
  std::cout<<" LAT   flux ("<< LATL <<","<< LATH <<") = "<<fLAT<<" erg/cm^2"<<std::endl;
  std::cout<<" GRB   flux ("<< emin <<","<< emax <<") = "<<fTOT<<" erg/cm^2"<<std::endl;
  std::cout<<"**************************************************"<<std::endl;
    
  /*
    ofstream f1( "GRBData.txt",ios::app);
    if (! f1.is_open()) 
    {
    std::cout<<"Error Opening output file"<<std::endl;
    exit(1);
    }
    f1<<tstart<<" "<<T95-T05<<" "<<direction.first<<" "<<direction.second<<std::endl;
    f1<<Spectr->Integral(i1,i2,"width")*1.0e-7/erg2meV<<" "<<
    Spectr->Integral(i3,i4,"width")*1.0e-7/erg2meV<<" "<<
    Spectr->Integral(i5,i6,"width")*1.0e-7/erg2meV<<" "<<
    Spectr->Integral(0,ne,"width")*1.0e-7/erg2meV<<" "<<std::endl;
    f1.close();
  */
}
