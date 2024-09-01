#include "../include/Object.hh"
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <iostream>

using namespace std;

void EnergyLoss(const char *path)
{
  TFile *file = TFile::Open(path);
  auto tree = (TTree *)file->Get("tree");
  TClonesArray *Tracks = NULL;
  tree->SetBranchAddress("Tracks", &Tracks);

  gStyle->SetHistLineWidth(2);
  TH1F *Ionization = new TH1F("Ionization", "", 50, 0, 20);
  TH1F *Radiation = new TH1F("Radiation", "", 50, 0, 20);

  for(Long64_t i = 0; tree->GetEntry(i); ++i) {
    Double_t ionization = 0.0, radiation = 0.0;
    Int_t nTrack = Tracks->GetEntries();
    if(nTrack == 0) continue;
    for(Int_t iTrack = 1; iTrack < nTrack; ++iTrack) {
      auto track = (Track *)Tracks->UncheckedAt(iTrack);
      if(track->Mother != 1) continue;  // Consider only primary ionization/radiation.
      switch(track->Pid) {
        case 11: ionization += track->E; break;
        case 22: radiation += track->E; break;
        default: cerr << "WARNING: Unexpected PDG ID " << track->Pid << endl;
      }
    }
    Ionization->Fill(ionization);
    Radiation->Fill(radiation);
  }

  TCanvas *canvas = new TCanvas;
  canvas->SetTopMargin(0.05);
  canvas->SetBottomMargin(0.08);
  canvas->SetLeftMargin(0.12);
  canvas->SetRightMargin(0.03);

  Double_t maximum = max(Ionization->GetMaximum(), Radiation->GetMaximum());
  Ionization->SetMaximum(maximum * 1.1);
  Ionization->SetStats(0);
  Ionization->SetLineColor(2);
  Ionization->SetXTitle("Energy loss [MeV]");
  Ionization->SetYTitle("Events");
  Ionization->Draw();
  Radiation->SetLineColor(3);
  Radiation->Draw("Same");

  TLegend *legend = canvas->BuildLegend(0.75, 0.78, 0.95, 0.93);
  legend->Draw();
  canvas->SaveAs("EnergyLoss.pdf");

  delete canvas;
  delete Ionization;
  delete Radiation;
  file->Close();
}
