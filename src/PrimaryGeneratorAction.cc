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

#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  auto detectorConstruction = (DetectorConstruction *)
    G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();

  fParticleGun = new G4ParticleGun(1);
  G4ParticleDefinition *particle = particleTable->FindParticle("mu-");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleMomentumDirection({0, 0, 1});
  fParticleGun->SetParticleEnergy(1 * GeV);
  fParticleGun->SetParticlePosition(detectorConstruction->GetSourcePosition());
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
{
  fParticleGun->GeneratePrimaryVertex(event);
}

void PrimaryGeneratorAction::SetTotalEnergy(G4double energy)
{
  energy -= fParticleGun->GetParticleDefinition()->GetPDGMass();
  if(energy < 0) throw std::invalid_argument("energy less than mass");
  fParticleGun->SetParticleEnergy(energy);
}
