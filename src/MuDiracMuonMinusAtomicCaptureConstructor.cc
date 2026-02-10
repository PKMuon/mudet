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

#include "MuDiracMuonMinusAtomicCaptureConstructor.hh"

#include "G4GenericMuonicAtom.hh"
#include "G4MuonMinus.hh"
#include "G4ProcessManager.hh"
#include "MuDiracMuonMinusAtomicCapture.hh"

MuDiracMuonMinusAtomicCaptureConstructor::MuDiracMuonMinusAtomicCaptureConstructor(const G4String &name)
    : G4VPhysicsConstructor(name)
{
}

void MuDiracMuonMinusAtomicCaptureConstructor::ConstructParticle()
{
  G4MuonMinus::Definition();
  G4GenericMuonicAtom::GenericMuonicAtomDefinition();
}

void MuDiracMuonMinusAtomicCaptureConstructor::ConstructProcess()
{
  // Get muon particle definition
  G4ParticleDefinition *muon = G4MuonMinus::Definition();
  G4ProcessManager *pManager = muon->GetProcessManager();

  if(!pManager) {
    G4Exception("MuDiracMuonMinusAtomicCaptureConstructor::ConstructProcess", "NoProcessManager", FatalException,
        "MuonMinus has no process manager!");
    return;
  }

  // Create the atomic capture process
  auto *muCapture = new MuDiracMuonMinusAtomicCapture("MuDiracMuonMinusAtomicCapture");

  // Attach as a rest process (triggers when muon stops)
  pManager->AddRestProcess(muCapture);
}
