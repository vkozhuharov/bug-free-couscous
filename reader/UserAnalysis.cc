#include "HistoSvc.hh"
#include "ProtoFocalEvent.hh"
#include <iostream>
#include <iomanip>
#include "TF1.h"
#include "TCanvas.h"
#include "stdio.h"

FILE *b0out;
FILE *b1out;

const float b1ped = 0.;//4.11554e+03;
const float b0ped = 0.;//2.09573e+03;


int InitAnalysis() {
  //Book the histograms
  HistoSvc *histo = HistoSvc::GetInstance();
  
  char name[256];
  char title[256];

  b0out = fopen("b0dump.log","w");
  b1out = fopen("b1dump.log","w");
  histo->AddHisto("EventType",new TH1F("EventType","Type of events with fired boards",4,0,4));
  
  for(int ib = 0;ib < MAXNBOARDS; ib++) {
    for (int i = 0;i< MAXNCHANNELS;i++){
      sprintf(name,"B_%d_Ch_%02d",ib,i);
      sprintf(title,"Charge in channel %02d , board %d",i,ib);
      histo->AddHisto(name, new TH1F(name, title, 1000, 0.0, 4096));
    }
  }
  histo->AddHisto("ProtoFoCalMap",new TH2F("ProtoFoCalMap","Charge map of a single event",NROWS,0,NROWS,NCOLUMNS,0,NCOLUMNS));
  histo->AddHisto("ProtoFoCalMapComulative",new TH2F("ProtoFoCalMapComulative","Comulative charge map",NROWS,0,NROWS,NCOLUMNS,0,NCOLUMNS));

  
  histo->AddHisto("TotalCharge",new TH1F("TotalCharge","Total charge in the event", 10000, 0.0,400000));
  histo->AddHisto("TotalCharge2B",new TH1F("TotalCharge2B","Total charge in the event", 10000, 0.0,400000));
  histo->AddHisto("TotalChargeB0",new TH1F("TotalChargeB0","Total charge in the event", 10000, 0.0,400000));
  histo->AddHisto("TotalChargeB1",new TH1F("TotalChargeB1","Total charge in the event", 10000, 0.0,400000));

  
  histo->AddHisto("AvalancheX", new TH1F("AvalancheX", "Position X of Avalanche",1000, 0.0, 20));
  histo->AddHisto("AvalancheY", new TH1F("AvalancheY", "Position Y of Avalanche",1000, 0.0, 20));
  
  histo->AddHisto("AvalancheSigmaX", new TH1F("AvalancheSigmaX", "Sigma Position X of Avalanche",1000, 0.0, 20));
  histo->AddHisto("AvalancheSigmaY", new TH1F("AvalancheSigmaY", "Sigma Position Y of Avalanche",1000, 0.0, 20));

  histo->AddHisto("AvalancheSigmaXvsSigmaY", new TH2F("AvalancheSigmaXvsSigmaY", "Sigma Position Y of Avalanche",1000, 0.0, 20,1000, 0.0, 20));
  histo->AddHisto("AvalancheRadius", new TH1F("AvalancheRadius", "Radius of Avalanche",1000, 0.0, 20));
  histo->AddHisto("TotChargeVsRadius",new TH2F("TotChargeVsRadius","Total charge vs radius of the shower",
					       10000, 0.0,400000,1000, 0.0, 20));


  histo->AddHisto("NFired",new TH1F("NFired","Number of fired SiPMs",48,0.0,48.0));
  
  histo->AddHisto("TotChargeVsNFired",new TH2F("TotChargeVsNFired","Total charge vs nfired",
					       1000, 0.0,400000,48, 0.0, 48));

  histo->AddHisto("Emax",new TH1F("Emax","Cell with maximal energy",1000,0.0,4096));
  histo->AddHisto("EmaxVsEtot",new TH1F("EmaxVsEtot","Ratio between maximal cell  and total energy",1000,0.0,1.1));
  
  histo->AddHisto("TotChargeVsEmax_ov_Etot",new TH2F("TotChargeVsEmax_ov_Etot","Total charge vs Emax/Etot",
					       1000, 0.0,400000,100,0.0,1.1));


  histo->AddHisto("MuonMap",new TH2F("MuonMap","Map of events with Nfired=5 and charge about 4000",NROWS,0,NROWS,NCOLUMNS,0,NCOLUMNS));

  
  
  //Do whatever you want 
  
  return 0;
}


void DumpTimeStamps(ProtoFoCalHEvent *evt){
  if(evt->bOK[0] == 1) {
    fprintf(b0out,"B0 evt: %8d, TS0: %10d, TS1: %10d\n", evt->evn,evt->t0[0],evt->t1[0]);
    fprintf(stdout,"B0 evt: %8d, TS0: %10d, TS1: %10d\n", evt->evn,evt->t0[0],evt->t1[0]);
  }
  if(evt->bOK[1] == 1) {
    fprintf(b1out,"B1 evt: %8d, TS0: %10d, TS1: %10d\n", evt->evn,evt->t0[1], evt->t1[1]);
  }

}


int AnalyzeEvent(ProtoFoCalHEvent *evt ){
  HistoSvc *histo = HistoSvc::GetInstance();

  char name[256];
  static int N;
  N++;
  float totCharge=0;
  
  

  // DumpTimeStamps(evt);
  // printSingleEvent(evt);

  int evtType = 2*evt->bOK[1] + evt->bOK[0];
  histo->GetHisto("EventType")->Fill(evtType);
  
  //if(evt->bOK[1] == 1) return 0;
  
  if(evt->t0event == 0 && evt->t1event == 0 ) {
    //Good event !
    Double_t x = 0;
    Double_t y = 0;
    double sumx = 0;
    double sumy = 0;

    int nfired = 0;
    float emax = 0.;
    int ixmax;
    int iymax;
    
    for(int x = 0;x<NROWS;x++){
      for(int y=0;y<NCOLUMNS;y++){
       	histo->GetHisto("ProtoFoCalMap")->SetBinContent(NROWS-x-1,NCOLUMNS-y-1,evt->ch[x][y].charge);
	if(evt->ch[x][y].charge < 10.) continue;
	
	//	if(evt->ch[x][y].charge > 20)
	nfired ++;
	((TH2F* ) histo->GetHisto("ProtoFoCalMapComulative")) -> Fill(NROWS-x-1,NCOLUMNS-y-1,evt->ch[x][y].charge);
	totCharge+=evt->ch[x][y].charge;

	sumx+= evt->ch[x][y].charge*1.1875*(x+0.5); 
	sumy+= evt->ch[x][y].charge*1.583*(y+0.5);

	if(emax < evt->ch[x][y].charge) {
	  emax = evt->ch[x][y].charge;
	  ixmax = x;
	  iymax = y;
	}
	
      }
    }

    histo->GetHisto("Emax")->Fill(emax);
    histo->GetHisto("NFired")->Fill(nfired);
    histo->GetHisto("TotChargeVsNFired")->Fill(totCharge,nfired);

    histo->GetHisto("EmaxVsEtot")->Fill(emax/totCharge);
    histo->GetHisto("TotChargeVsEmax_ov_Etot")->Fill(totCharge,emax/totCharge);
    
    sumx = sumx/totCharge; 
    sumy = sumy/totCharge;

    
    Double_t sigmax = 0;
    Double_t sigmay = 0;

    for(int x = 0;x<NROWS;x++){
      for(int y=0;y<NCOLUMNS;y++){
	if(evt->ch[x][y].charge < 10.) continue;
	sigmax+= (sumx - nfired*evt->ch[x][y].charge/totCharge)*(sumx - nfired*evt->ch[x][y].charge/totCharge);
	sigmay+= (sumy - nfired*evt->ch[x][y].charge/totCharge)*(sumy - nfired*evt->ch[x][y].charge/totCharge);
      }
    }
    sigmax /= nfired;
    sigmax = sqrt(sigmax);
    
    sigmay /= nfired;
    sigmay = sqrt(sigmay);

    
    double r = sqrt(sigmax*sigmax + sigmay*sigmay);
    



    
    histo->GetHisto("AvalancheX")->Fill(sumx);
    histo->GetHisto("AvalancheY")->Fill(sumy);
    histo->GetHisto("AvalancheSigmaX")->Fill(sigmax);
    histo->GetHisto("AvalancheSigmaY")->Fill(sigmay);
    histo->GetHisto("AvalancheSigmaXvsSigmaY")->Fill(sigmax,sigmay);
    histo->GetHisto("AvalancheRadius")->Fill(r);

    
    histo->GetHisto("TotalCharge")->Fill(totCharge);
    histo->GetHisto("TotChargeVsRadius")->Fill(totCharge,r);


    
    if(evt->bOK[1] == 1 && evt->bOK[0] == 1) 
      histo->GetHisto("TotalCharge2B")->Fill(totCharge - b1ped -b0ped );
    if(evt->bOK[1] == 0 && evt->bOK[0] == 1) 
      histo->GetHisto("TotalChargeB0")->Fill(totCharge -b0ped );
    if(evt->bOK[1] == 1 && evt->bOK[0] == 0) 
      histo->GetHisto("TotalChargeB1")->Fill(totCharge -b1ped  );

    if(totCharge > 4000 && totCharge < 4400 && nfired >4 && nfired < 7) {

      histo->GetHisto("MuonMap")->Fill(NROWS-ixmax-1,NCOLUMNS-iymax-1);
      
      // TCanvas *c = new TCanvas();
      
      // c->cd();
      // histo->GetHisto("ProtoFoCalMap")->Draw("colz");
      // histo->GetHisto("ProtoFoCalMap")->SetStats(0);
      // //    c->SetLogz();
      // sprintf(name,"60GeV/ev%d.png",N);
      // c->Update();
      // c->Print(name);
      // delete c;
    }

  }

  
  

  
  

  return 0;
}


int printSingleEvent(ProtoFoCalHEvent *evt ){
  std::cout << "EvN: "<< std::setw(8) << evt->evn
	    << " t0_0: " << std::setw(10) << evt->t0[0]
	    << " t1_0: " <<  std::setw(10) << evt->t1[0]
	    << " t0_1: " << std::setw(10) << evt->t0[1]
	    << " t1_1: " <<  std::setw(10) << evt->t1[1]
    
	    // << " ts0_ref: " << evt->ch[0][0].ts0_ref
    	    // << " ts1_ref: " << evt->ch[0][0].ts1_ref
	    << std::endl;

  
  return 0;
}



int EndAnalysis(){
  //May be analyze the filled in histos and fill others
  HistoSvc *histo = HistoSvc::GetInstance();
  char name[64];
  //  float mean[2][32];
  fclose(b0out);
  fclose(b1out);

  //Pedestal calibration
  for(int ic = 0;ic < 2; ic++) {
    for (int iCh = 0;iCh<32;iCh++){
      sprintf(name,"B_%d_Ch_%02d",ic,iCh);      
      histo->GetHisto(name)->Fit("gaus","q");      
      float mean = ((TF1 *)((TH1F *) histo->GetHisto(name))->GetFunction("gaus"))->GetParameter(1);
      std::cout << "ped[" <<ic<< " ][" << iCh <<"] = " <<  mean << ";" << std::endl;
    }
  }
  
  return 0;
}

