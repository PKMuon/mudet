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
//
//-----------------------------------------------------------------------------
//
// GEANT4 Class file
//
// File name:  MuDiracEmCaptureCascade
//
// Author:        V.Ivanchenko (Vladimir.Ivantchenko@cern.ch)
//
// Creation date: 22 April 2012 on base of G4MuMinusCaptureCascade
//
//
//-----------------------------------------------------------------------------
//
// Modifications:
// Feb 11 2026: Y.P. Wu(
// overwrite deltaE with data from MuDirac, and add dataloader)
// 
//-----------------------------------------------------------------------------

#include "MuDiracEmCaptureCascade.hh"

#include "MuDiracDataHelper.hh"

#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4MuonMinus.hh"
#include "G4NucleiProperties.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

MuDiracEmCaptureCascade::MuDiracEmCaptureCascade()
{
  // [DONE]
  theElectron = G4Electron::Electron();
  theGamma = G4Gamma::Gamma();
  fMuMass = G4MuonMinus::MuonMinus()->GetPDGMass();
  fTime = 0.0;
}

MuDiracEmCaptureCascade::~MuDiracEmCaptureCascade() { }

G4HadFinalState *MuDiracEmCaptureCascade::ApplyYourself(const G4HadProjectile &projectile, G4Nucleus &targetNucleus)
{
  // [DONE]
  result.Clear();
  result.SetStatusChange(isAlive);
  fTime = projectile.GetGlobalTime();

  G4int Z = targetNucleus.GetZ_asInt();
  G4int A = targetNucleus.GetA_asInt();

  G4int nElec = Z;
  G4int nAuger = 1;
  

  G4int nLevel = 13; 
  G4double pGamma = (Z * Z * Z * Z);

  // [NEW] add dataloader
  MuDiracDataHelper* helper = MuDiracDataHelper::GetInstance();
  G4double edep = 0.0; 
  
  AddNewParticle(theElectron, edep);
  G4double deltaE;

  // Emit new photon or electron
  // Simplified model for probabilities
  // N.C.Mukhopadhyay Phy. Rep. 30 (1977) 1.
  do {
    // case of Auger electrons
    if((nAuger < nElec) && ((pGamma + 10000.0) * G4UniformRand() < 10000.0)) {
      ++nAuger;
      deltaE = helper->GetTransitionEnergy(Z, A, nLevel, nLevel - 1);
      --nLevel; 
      AddNewParticle(theElectron, deltaE);

    } else {
      // Case of photon cascade, probabilities from
      // C.S.Wu and L.Wilets, Ann. Rev. Nuclear Sci. 19 (1969) 527.
      G4double var = (10.0 + G4double(nLevel - 1)) * G4UniformRand();
      G4int iLevel = nLevel - 1;
      if(var > 10.0) iLevel -= G4int(var - 10.0) + 1;
      if(iLevel < 0) iLevel = 0;
      deltaE = helper->GetTransitionEnergy(Z, A, nLevel, iLevel);
      
      nLevel = iLevel;
      AddNewParticle(theGamma, deltaE);
    }
    edep += deltaE;

    // G4cout << "[[DEBUG]]" << " Z: " << Z
    //       << " A: " << A
    //       << " nLevel: " << nLevel
    //       << " deltaE: " << deltaE / keV << " keV"
    //       << G4endl;
    //Loop checking
  } while(nLevel > 0);

  result.SetLocalEnergyDeposit(edep);
  return &result;
}
