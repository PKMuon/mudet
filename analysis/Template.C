#include "../include/Object.hh"
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <iostream>
#include <iomanip>

using namespace std;

void Template(const char *path = "../build/tree/latest.root")
{
  TFile *file = TFile::Open(path);
  auto tree = (TTree *)file->Get("tree");
  TClonesArray *Tracks = NULL;
  tree->SetBranchAddress("Tracks", &Tracks);

  for(Long64_t i = 0; tree->GetEntry(i); ++i) {
    Int_t nTrack = Tracks->GetEntries();
    for(Int_t iTrack = 0; iTrack < nTrack; ++iTrack) {
      auto track = (Track *)Tracks->UncheckedAt(iTrack);
      cout << scientific << setprecision(3)
           << "Event " << setw(4) << i << "\tTrack " << setw(2) << iTrack
           << "\tPID=" << setw(2) << track->Pid << "\tE=" << track->E << "MeV"
           << endl;
    }
  }

  file->Close();
}
