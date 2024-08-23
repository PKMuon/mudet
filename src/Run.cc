//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//

#include "Run.hh"
#include "Object.hh"

#include "G4ios.hh"
#include "G4Threading.hh"
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TROOT.h>
#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

struct ROOTInitializer {
  ROOTInitializer() { ROOT::EnableThreadSafety(); }
} rootInitializer [[maybe_unused]];

}

G4String Run::fDirName = "tree/" + std::to_string(getpid());
G4String Run::fTreeName = "tree";
G4String Run::fTreeTitle = "tree";

class Run::Manager {
public:
  Manager();

  void Branch(TTree *tree);
  void PreFill();
  void Reset();
  void AddTrack(const G4Track *track);

private:
  TClonesArray Tracks;
};

Run::Run()
{
  G4String filename = fDirName + "/" + std::to_string(G4Threading::G4GetThreadId()) + ".root";
  auto parent = fs::path(filename.c_str()).parent_path();
  if(!parent.empty()) fs::create_directories(parent);

  fManager = new Manager;
  fFile = TFile::Open(filename, "RECREATE");
  if(!fFile->IsOpen()) {
    G4cerr << "Error opening file " << filename << G4endl;
    exit(1);
  }
  fTree = new TTree(fTreeName, fTreeTitle);
  fManager->Branch(fTree);
}

Run::~Run()
{
  fFile->cd();
  fTree->Write(NULL, fTree->kOverwrite);
  fFile->Close();
  delete fManager;
}

void Run::AutoSave()
{
  fTree->AutoSave("SaveSelf, Overwrite");
}

void Run::FillAndReset()
{
  fManager->PreFill();
  fTree->Fill();
  fManager->Reset();
}

void Run::AddTrack(const G4Track *track)
{
  fManager->AddTrack(track);
}

Run::Manager::Manager() : Tracks("Track")
{

}

void Run::Manager::Branch(TTree *tree)
{
  tree->Branch("Tracks", &Tracks);
}

void Run::Manager::PreFill()
{
  Int_t n = Tracks.GetEntries();

  // Inplace index sort.
  for(Int_t i = 0; i < n; ++i) {
    auto track = (Track *)Tracks[i];
    while(track->Id - 1 != i) {
      if(track->Id <= 0 || track->Id > n) {
        throw std::runtime_error("invalid track ID: " + std::to_string(track->Id));
      }
      if(((Track *)Tracks[track->Id - 1])->Id == track->Id) {
        throw std::runtime_error("duplicate track ID: " + std::to_string(track->Id));
      }
      TObject *object = track;
      std::swap(object, Tracks[track->Id - 1]);
      track = (Track *)object;
    }
    Tracks[i] = track;
  }
}

void Run::Manager::Reset()
{
  Tracks.Clear();
}

void Run::Manager::AddTrack(const G4Track *track)
{
  *(Track *)Tracks.ConstructedAt(Tracks.GetEntries()) = *track;
}
