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

#include "MuDiracMuonMinusAtomicCapture.hh"

#include "G4Nucleus.hh"
#include "G4RunManager.hh"
#include "MuDiracEmCaptureCascade.hh"
#include "Run.hh"
#include "RunAction.hh"

MuDiracMuonMinusAtomicCapture::MuDiracMuonMinusAtomicCapture(const G4String &name)
    : G4MuonMinusAtomicCapture_11_3_2(name)
{
  delete fEmCascade;
  fEmCascade = new MuDiracEmCaptureCascade();
}

G4VParticleChange *MuDiracMuonMinusAtomicCapture::AtRestDoIt(const G4Track &track, const G4Step &step)
{
  fElementSelector->SelectZandA(track, &targetNucleus);
  if(targetNucleus.GetZ_asInt() == 32) {  // Disabled for performance.
    theTotalResult->Initialize(track);
    return theTotalResult;
  }
  if(targetNucleus.GetZ_asInt() == 82) {  // Disabled for diagnostics.
    theTotalResult->Initialize(track);
    return theTotalResult;
  }

  Run *run = ((RunAction *)G4RunManager::GetRunManager()->GetUserRunAction())->GetRun();
  G4VParticleChange *change = G4MuonMinusAtomicCapture_11_3_2::AtRestDoIt(track, step);
  run->AddMuonCapture(&targetNucleus, change);
  return change;
}
