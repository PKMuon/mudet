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
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TROOT.h>
#include <unistd.h>
#include <filesystem>
#include <unordered_set>

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
  ~Manager();

  void Branch(TTree *tree);
  Bool_t PreFill();
  void Reset();
  void AddTrack(const G4Track *track);
  void AddVertex(const G4Track *track);
  void AddStep(const G4Step *track);
  void SaveCuts();

private:
  TFile *fFile;
  TTree *fCuts;
  TClonesArray Tracks;
  TClonesArray Vertices;
  TClonesArray Cuts;
  Double_t EnergyDeposit, NonIonizingEnergyDeposit;
  Double_t EnergyBackward;
  std::unordered_set<Int_t> ForwardTrackIDSet;
};

Run::Run()
{
  G4String filename = fDirName + "/" + std::to_string(G4Threading::G4GetThreadId()) + ".root";
  auto parent = fs::path(filename.c_str()).parent_path();
  if(!parent.empty()) fs::create_directories(parent);

  fFile = TFile::Open(filename, "RECREATE");
  if(!fFile->IsOpen()) {
    G4cerr << "Error opening file " << filename << G4endl;
    exit(1);
  }
  fTree = new TTree(fTreeName, fTreeTitle);
  fManager = new Manager;
  fManager->Branch(fTree);
}

Run::~Run()
{
  delete fManager;
  fFile->cd();
  fTree->Write(NULL, fTree->kOverwrite);
  fFile->Close();
}

void Run::AutoSave()
{
  fTree->AutoSave("SaveSelf, Overwrite");
  fManager->SaveCuts();
}

void Run::FillAndReset()
{
  if(fManager->PreFill()) fTree->Fill();
  fManager->Reset();
}

void Run::AddTrack(const G4Track *track)
{
  fManager->AddTrack(track);
}

void Run::AddVertex(const G4Track *track)
{
  fManager->AddVertex(track);
}

void Run::AddStep(const G4Step *step)
{
  fManager->AddStep(step);
}

Run::Manager::Manager() : Tracks("Track"), Vertices("Vertex"), Cuts("Cuts")
{
  fFile = NULL;
  fCuts = NULL;
  Reset();
}

Run::Manager::~Manager()
{
  if(fFile == NULL) return;
  fFile->cd();
  fCuts->Write(NULL, fCuts->kOverwrite);
}

void Run::Manager::Branch(TTree *tree)
{
  tree->Branch("Tracks", &Tracks);
  //tree->Branch("Vertices", &Vertices);
  tree->Branch("EnergyDeposit", &EnergyDeposit);
  tree->Branch("NonIonizingEnergyDeposit", &NonIonizingEnergyDeposit);

  fFile = tree->GetCurrentFile();
  fFile->cd();
  fCuts = new TTree("cuts", "cuts");
  fCuts->Branch("Cuts", &Cuts);
}

Bool_t Run::Manager::PreFill()
{
  if(EnergyBackward < 1.0 * MeV) return kFALSE;

  Int_t n = Tracks.GetEntries();

  //// Inplace index sort.
  //for(Int_t i = 0; i < n; ++i) {
  //  auto track = (Track *)Tracks[i];
  //  while(track->Id - 1 != i) {
  //    if(track->Id <= 0 || track->Id > n) {
  //      throw std::runtime_error("invalid track ID: " + std::to_string(track->Id));
  //    }
  //    if(((Track *)Tracks[track->Id - 1])->Id == track->Id) {
  //      throw std::runtime_error("duplicate track ID: " + std::to_string(track->Id));
  //    }
  //    TObject *object = track;
  //    std::swap(object, Tracks[track->Id - 1]);
  //    track = (Track *)object;
  //  }
  //  Tracks[i] = track;
  //}

  std::vector<Track *> tracks;
  for(Int_t i = 0; i < n; ++i) tracks.push_back((Track *)Tracks[i]);
  sort(tracks.begin(), tracks.end(), [](Track *a, Track *b) { return a->Id < b->Id; });
  for(Int_t i = 0; i < n; ++i) Tracks[i] = tracks[i];

  return kTRUE;
}

void Run::Manager::Reset()
{
  Tracks.Clear();
  Vertices.Clear();
  EnergyDeposit = 0;
  NonIonizingEnergyDeposit = 0;
  EnergyBackward = 0;
  ForwardTrackIDSet = {0};  // The dummy mother for the primary track.
}

void Run::Manager::AddTrack(const G4Track *track)
{
  auto momentum = track->GetMomentum();
  if(momentum.z() >= 0) {
    if(ForwardTrackIDSet.count(track->GetParentID())) ForwardTrackIDSet.insert(track->GetTrackID());
  } else {
    *(Track *)Tracks.ConstructedAt(Tracks.GetEntries()) = *track;
    if(ForwardTrackIDSet.count(track->GetParentID())) EnergyBackward += track->GetKineticEnergy();
  }
}

void Run::Manager::AddVertex(const G4Track *track)
{
  //*(Vertex *)Vertices.ConstructedAt(Vertices.GetEntries()) = *track;
}

void Run::Manager::AddStep(const G4Step *step)
{
  if(step->GetTrack()->GetTrackID() == 1) {  // primary track
    AddVertex(step->GetTrack());
  }
  EnergyDeposit += step->GetTotalEnergyDeposit();
  NonIonizingEnergyDeposit += step->GetNonIonizingEnergyDeposit();
}

void Run::Manager::SaveCuts()
{
  *(::Cuts *)Cuts.ConstructedAt(0) = *G4LogicalVolumeStore::GetInstance()->GetVolume("world");
  fCuts->Fill();
  fCuts->AutoSave("SaveSelf, Overwrite");
}
