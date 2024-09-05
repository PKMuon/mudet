#include "../include/Object.hh"
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>
#include <iostream>
#include <iomanip>
#include <unordered_map>

using namespace std;

void Energy(const char *path = "../build/tree/latest.root")
{
  TFile *file = TFile::Open(path);
  auto tree = (TTree *)file->Get("tree");

  gStyle->SetHistLineWidth(2);
  TCanvas *canvas = new TCanvas;
  canvas->SetTopMargin(0.05);
  canvas->SetBottomMargin(0.08);
  canvas->SetLeftMargin(0.12);
  canvas->SetRightMargin(0.03);

  tree->Draw("Tracks.E", "Tracks.Id == 1");
  auto Energy = (TH1 *)gROOT->FindObject("htemp");
  Energy->SetName("Energy");
  Energy->SetTitle("");
  Energy->SetXTitle("Energy [MeV]");
  Energy->SetYTitle("Events");
  Energy->Draw();

  canvas->SaveAs("Energy.pdf");
  delete canvas;

  unordered_map<Double_t, Long64_t> Ecount;
  Double_t ESum = 0, E2Sum = 0;
  TClonesArray *Tracks = NULL;
  tree->SetBranchAddress("Tracks", &Tracks);
  for(Long64_t i = 0; tree->GetEntry(i); ++i)
  {
    auto track = (Track *)Tracks->At(0);  // incoming beam
    ++Ecount[track->E];
    ESum += track->E;
    E2Sum += track->E * track->E;
  }
  cout << scientific << setprecision(18);
  cout << "Mean energy: " << ESum / tree->GetEntries() << " MeV" << endl;
  cout << "Standard deviation: "
       << sqrt(E2Sum / tree->GetEntries() - (ESum / tree->GetEntries()) * (ESum / tree->GetEntries()))
       << " MeV" << endl;
  cout << "Energy distribution:" << endl;
  for(auto &p : Ecount) {
     cout << "  " << p.first << " MeV\t" << p.second << " events" << endl;
  }

  file->Close();
}
