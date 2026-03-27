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

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4UserLimits.hh"

DetectorConstruction::DetectorConstruction()
{
  fWorldX = 10 * cm;
  fWorldY = 10 * cm;
  fWorldZ = 10 * cm;
  fSourcePosition = {0, 0, -fWorldZ * 0.5};
}

DetectorConstruction::~DetectorConstruction()
{

}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
  auto nist = G4NistManager::Instance();
  auto world_s = new G4Box("world", fWorldX * 0.5, fWorldY * 0.5, fWorldZ * 0.5);
  auto world_l = new G4LogicalVolume(world_s, nist->FindOrBuildMaterial("G4_AIR"), "world");
  world_l->SetUserLimits(new G4UserLimits(fWorldZ * 0.01));

  auto CaCO3 = new G4Material("CaCO3", 2.71 * g / cm3, 3, G4State::kStateSolid);
  CaCO3->AddElement(nist->FindOrBuildElement(20), 1);
  CaCO3->AddElement(nist->FindOrBuildElement(6), 1);
  CaCO3->AddElement(nist->FindOrBuildElement(8), 3);

  auto ground_s = new G4Box("ground", fWorldX * 0.5, fWorldY * 0.5, fWorldZ * 0.25);
  auto ground_l = new G4LogicalVolume(ground_s, CaCO3, "ground");
  ground_l->SetUserLimits(new G4UserLimits(fWorldZ * 0.01));
  new G4PVPlacement(NULL, {0, 0, fWorldZ * 0.25}, ground_l, "ground", world_l, false, 0, true);

  auto world_p = new G4PVPlacement(NULL, {}, world_l, "world", NULL, false, 0, true);
  return world_p;
}
