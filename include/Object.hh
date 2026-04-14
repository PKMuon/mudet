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

#ifndef Object_h
#define Object_h 1

#include <TLorentzVector.h>
#include <TObject.h>

#include <tuple>

class G4Track;
class G4LogicalVolume;
class G4Nucleus;
class G4VParticleChange;

class Event : public TObject {
public:
  Int_t Pid;
  Double_t Px;
  Double_t Py;
  Double_t Pz;
  Double_t E;
  Double_t X;
  Double_t Y;
  Double_t Z;
  Double_t T;

  void Reset() { memset(&Pid, 0, (char *)&T - (char *)&Pid + sizeof T); }

  ClassDef(Event, 1);
};

class Edep : public TObject {
public:
  Edep &operator=(std::pair<Int_t, Double_t>);

  Int_t Pid;
  Double_t Edep;

  ClassDef(Edep, 1);
};

class Track : public TObject {
public:
  Track &operator=(const G4Track &);

  Int_t Id;
  Int_t Mother;
  Int_t Pid;
  Double_t Px;
  Double_t Py;
  Double_t Pz;
  Double_t E;
  Double_t X;
  Double_t Y;
  Double_t Z;
  Double_t T;

  ClassDef(Track, 1);
};

class Cuts : public TObject {
public:
  Cuts &operator=(const G4LogicalVolume &);

  Double_t GammaCut;
  Double_t GammaThreshold;
  Double_t ElectronCut;
  Double_t ElectronThreshold;
  Double_t PositronCut;
  Double_t PositronThreshold;
  Double_t ProtonCut;
  Double_t ProtonThreshold;

  ClassDef(Cuts, 1);
};

class MuonCapture : public TObject {
public:
  MuonCapture &operator=(std::tuple<const G4Nucleus &, const G4VParticleChange &>);

  Double_t NucleonZ;
  Double_t NucleonA;
  Double_t MuonPx;
  Double_t MuonPy;
  Double_t MuonPz;
  Double_t MuonE;
  Double_t MuonX;
  Double_t MuonY;
  Double_t MuonZ;
  Double_t MuonT;

  std::vector<TLorentzVector> ElectronP4;
  std::vector<TLorentzVector> GammaP4;

  ClassDef(MuonCapture, 1);
};

#endif
