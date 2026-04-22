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

#include <TClonesArray.h>
#include <TFile.h>
#include <TH1L.h>
#include <TROOT.h>
#include <TTree.h>
#include <unistd.h>

#include <filesystem>
#include <map>

#include "G4LogicalVolumeStore.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
#include "Object.hh"

namespace fs = std::filesystem;

namespace {

struct ROOTInitializer {
  ROOTInitializer() { ROOT::EnableThreadSafety(); }
} rootInitializer [[maybe_unused]];

}  //namespace

G4String Run::fDirName = "tree/" + std::to_string(getpid());
G4String Run::fTreeName = "tree";
G4String Run::fTreeTitle = "tree";

class Run::Manager {
public:
  Manager();
  ~Manager();

  G4LogicalVolume *GetScoringVolume();
  G4LogicalVolume *GetTargetVolume();

  void Branch(TTree *tree);
  bool PreFill();
  void Reset();
  void AddTrack(const G4Track *track);
  void AddStep(const G4Step *track);
  void AddMuonCapture(const G4Nucleus *nucleus, const G4VParticleChange *change);
  void AddEvent(const Event *event);
  void SaveCuts();
  void SaveNMuon();

private:
  TFile *fFile;
  TTree *fCuts;
  TH1L *fNMuon;
  TClonesArray Edeps;
  TClonesArray Tracks;
  TClonesArray Cuts;
  TClonesArray MuonCaptures;
  TClonesArray Events;
  G4LogicalVolume *fScoringVolume, *fTargetVolume;
  std::map<G4int, G4double> fEnergyDeposit;
};

G4LogicalVolume *Run::Manager::GetScoringVolume()
{
  if(!fScoringVolume) fScoringVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("HPGe");
  return fScoringVolume;
}

G4LogicalVolume *Run::Manager::GetTargetVolume()
{
  if(!fTargetVolume) fTargetVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("target");
  return fTargetVolume;
}

Run::Run()
{
  extern int64_t seed;
  G4String filename = fDirName + "/" + std::to_string(seed == -1 ? gettid() : seed) + ".root";
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
  fManager->SaveNMuon();
}

void Run::FillAndReset()
{
  if(fManager->PreFill()) fTree->Fill();
  fManager->Reset();
}

void Run::AddTrack(const G4Track *track) { fManager->AddTrack(track); }

void Run::AddStep(const G4Step *step) { fManager->AddStep(step); }

void Run::AddMuonCapture(const G4Nucleus *nucleus, const G4VParticleChange *change)
{
  fManager->AddMuonCapture(nucleus, change);
}

void Run::AddEvent(const Event *event) { fManager->AddEvent(event); }

Run::Manager::Manager() : Edeps("Edep"), Tracks("Track"), Cuts("Cuts"), MuonCaptures("MuonCapture"), Events("Event")
{
  fFile = NULL;
  fCuts = NULL;
  fScoringVolume = NULL;
}

Run::Manager::~Manager()
{
  if(fFile == NULL) return;
  fFile->cd();
  fCuts->Write(NULL, fCuts->kOverwrite);
}

void Run::Manager::Branch(TTree *tree)
{
  tree->Branch("Edeps", &Edeps);
  tree->Branch("Tracks", &Tracks);
  tree->Branch("MuonCaptures", &MuonCaptures);
  tree->Branch("Events", &Events);

  fFile = tree->GetCurrentFile();
  fFile->cd();
  fCuts = new TTree("cuts", "cuts");
  fCuts->Branch("Cuts", &Cuts);
  fNMuon = new TH1L("nmuon", "nmuon", 1, -0.5, 0.5);
}

bool Run::Manager::PreFill()
{
  Int_t n = Tracks.GetEntries();
  std::vector<Track *> tracks;
  for(Int_t i = 0; i < n; ++i) tracks.push_back((Track *)Tracks[i]);
  sort(tracks.begin(), tracks.end(), [](Track *a, Track *b) { return a->Id < b->Id; });
  for(Int_t i = 0; i < n; ++i) Tracks[i] = tracks[i];

  //G4double edepSum = 0.0;
  for(const auto [pdgId, edep] : fEnergyDeposit) {
    *(Edep *)Edeps.ConstructedAt(Edeps.GetEntries()) = { pdgId, edep };
    //edepSum += edep;
  }
  //G4cout << "Debug: Total energy deposit: " << edepSum / CLHEP::MeV << " MeV" << G4endl;
  fEnergyDeposit.clear();

  //return Events.GetEntries() > 1;  // [NOTE] Requires at least one incident particle to the target.
  return Edeps.GetEntries() > 0;  // [NOTE] Requires at least one energy deposit in the scoring volume.
}

void Run::Manager::Reset()
{
  Edeps.Clear();
  Tracks.Clear();
  MuonCaptures.Clear();
  Events.Clear();
}

void Run::Manager::AddTrack(const G4Track *track)
{
  const G4VProcess *creatorProcess = track->GetCreatorProcess();
  std::string creatorProcessName = creatorProcess ? creatorProcess->GetProcessName() : "";
  auto pdgId = track->GetParticleDefinition()->GetPDGEncoding();
  if(creatorProcessName == "MuDiracMuonMinusAtomicCapture" || pdgId == 13) {  // mu-
    *(Track *)Tracks.ConstructedAt(Tracks.GetEntries()) = *track;
  }
}

void Run::Manager::AddStep(const G4Step *step)
{
  G4LogicalVolume *volume = step->GetTrack()->GetVolume()->GetLogicalVolume();
  //if(volume == GetTargetVolume() && step->IsFirstStepInVolume()) {
  //  Event event;
  //  event.Pid = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
  //  event.Px = step->GetTrack()->GetMomentum().x();
  //  event.Py = step->GetTrack()->GetMomentum().y();
  //  event.Pz = step->GetTrack()->GetMomentum().z();
  //  event.E = step->GetTrack()->GetTotalEnergy();
  //  event.X = step->GetTrack()->GetPosition().x();
  //  event.Y = step->GetTrack()->GetPosition().y();
  //  event.Z = step->GetTrack()->GetPosition().z();
  //  event.T = step->GetTrack()->GetGlobalTime();
  //  AddEvent(&event);
  //}
  if(volume != GetScoringVolume()) return;
  fEnergyDeposit[step->GetTrack()->GetParticleDefinition()->GetPDGEncoding()] += step->GetTotalEnergyDeposit();
}

void Run::Manager::AddMuonCapture(const G4Nucleus *nucleus, const G4VParticleChange *change)
{
  *(MuonCapture *)MuonCaptures.ConstructedAt(MuonCaptures.GetEntries()) = { *nucleus, *change };
}

void Run::Manager::AddEvent(const Event *event)
{
  *(Event *)Events.ConstructedAt(Events.GetEntries()) = *event;
  if(abs(event->Pid) == 13) fNMuon->Fill(0);
}

void Run::Manager::SaveCuts()
{
  *(::Cuts *)Cuts.ConstructedAt(0) = *GetScoringVolume();
  fCuts->Fill();
  fCuts->AutoSave("SaveSelf, Overwrite");
}

void Run::Manager::SaveNMuon()
{
  fFile->cd();
  fNMuon->Write(NULL, fNMuon->kOverwrite);
}
