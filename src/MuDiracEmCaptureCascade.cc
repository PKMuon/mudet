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

#include "MuDiracEmCaptureCascade.hh"

#include <math.h>

#include "G4NucleiProperties.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "MuDiracDataHelper.hh"
#include "Randomize.hh"

MuDiracEmCaptureCascade::MuDiracEmCaptureCascade() : fMuDiracEnabled(true)
{
  char *content = getenv("MUDET_DISABLE_MUDIRAC");
  if(content && *content) fMuDiracEnabled = false;
}

MuDiracEmCaptureCascade::~MuDiracEmCaptureCascade() { }

G4HadFinalState *MuDiracEmCaptureCascade::ApplyYourself(const G4HadProjectile &projectile, G4Nucleus &targetNucleus)
{
  MuDiracDataHelper *helper = MuDiracDataHelper::GetInstance();

  result.Clear();
  result.SetStatusChange(isAlive);
  fTime = projectile.GetGlobalTime();

  G4int Z = targetNucleus.GetZ_asInt();
  G4int A = targetNucleus.GetA_asInt();
  G4double massA = G4NucleiProperties::GetNuclearMass(A, Z);
  G4double mass = fMuMass * massA / (fMuMass + massA);
  G4double e = 13.6 * eV * (Z * Z) * mass / electron_mass_c2;

  // precise corrections of energy only for K-shell
  fLevelEnergy[0] = fKLevelEnergy[std::min(Z, 92)];
  for(G4int i = 1; i < 14; ++i) { fLevelEnergy[i] = e / (G4double)((i + 1) * (i + 1)); }

  G4int nElec = Z;
  G4int nAuger = 1;
  G4int nLevel = 13;
  G4double pGamma = (Z * Z * Z * Z);

  // Capture on 14-th level
  G4double edep = fLevelEnergy[13];
  AddNewParticle(theElectron, edep);
  G4double deltaE;

  // Emit new photon or electron
  // Simplified model for probabilities
  // N.C.Mukhopadhyay Phy. Rep. 30 (1977) 1.
  do {
    // case of Auger electrons
    if((nAuger < nElec) && ((pGamma + 10000.0) * G4UniformRand() < 10000.0)) {
      ++nAuger;
      deltaE = 0.0 / 0.0;
      if(fMuDiracEnabled) deltaE = helper->GetTransitionEnergy(Z, A, nLevel + 1, nLevel);
      //if(isnan(deltaE)) deltaE = fLevelEnergy[nLevel - 1] - fLevelEnergy[nLevel];
      if(isnan(deltaE)) deltaE = 0;
      --nLevel;
      if(deltaE) AddNewParticle(theElectron, deltaE);

    } else {
      // Case of photon cascade, probabilities from
      // C.S.Wu and L.Wilets, Ann. Rev. Nuclear Sci. 19 (1969) 527.

      G4double var = (10.0 + G4double(nLevel - 1)) * G4UniformRand();
      G4int iLevel = nLevel - 1;
      if(var > 10.0) iLevel -= G4int(var - 10.0) + 1;
      if(iLevel < 0) iLevel = 0;
      deltaE = 0.0 / 0.0;
      if(fMuDiracEnabled) deltaE = helper->GetTransitionEnergy(Z, A, nLevel + 1, iLevel + 1);
      //if(isnan(deltaE)) deltaE = fLevelEnergy[iLevel] - fLevelEnergy[nLevel];
      if(isnan(deltaE)) deltaE = 0;
      nLevel = iLevel;
      if(deltaE) AddNewParticle(theGamma, deltaE);
    }
    edep += deltaE;

    // Loop checking, 06-Aug-2015, Vladimir Ivanchenko
  } while(nLevel > 0);

  result.SetLocalEnergyDeposit(edep);
  return &result;
}
