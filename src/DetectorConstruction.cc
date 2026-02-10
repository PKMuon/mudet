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
#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

DetectorConstruction::DetectorConstruction()
{
  fWorldX = 10 * cm;
  fWorldY = 10 * cm;
  fWorldZ = 10 * cm;
  fSourcePosition = { 0, 0, -fWorldZ * 0.5 };
}

DetectorConstruction::~DetectorConstruction() { }

static G4Material *GetWorldMaterial()
{
  std::string name = "Th-229";
  if(char *envname = getenv("MUDET_WORLD_MATERIAL")) name = envname;

  if(name == "Th-229") {
    // https://pubchem.ncbi.nlm.nih.gov/compound/167312#section=Computed-Properties
    // https://pubchem.ncbi.nlm.nih.gov/element/90#section=Density
    auto Th_229_isotope = new G4Isotope("Th-229", 90, 229, 229.03176 * g / mole);
    auto Th_229_element = new G4Element("Th-229", "Th-229", 1);
    Th_229_element->AddIsotope(Th_229_isotope, 100. * perCent);
    auto Th_229_material = new G4Material("Th-229", 11.72 * (229.03176 / 232.0377) * g / cm3, 1);
    Th_229_material->AddElement(Th_229_element, 1);
    return Th_229_material;
  }

  return G4NistManager::Instance()->FindOrBuildMaterial(name);  // e.g., G4_Al, G4_Cu
}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
  auto world_s = new G4Box("world", fWorldX * 0.5, fWorldY * 0.5, fWorldZ * 0.5);
  auto world_l = new G4LogicalVolume(world_s, GetWorldMaterial(), "world");
  auto world_p = new G4PVPlacement(NULL, {}, world_l, "world", NULL, false, 0, true);

  return world_p;
}
