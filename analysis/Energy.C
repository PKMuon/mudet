#include "../include/Object.hh"
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>

using namespace std;

void Energy(const char *path)
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
  file->Close();
}
