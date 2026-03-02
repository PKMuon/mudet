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
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"

DetectorConstruction::DetectorConstruction()
{
  fWorldX = 40 * cm;
  fWorldY = 40 * cm;
  fWorldZ = 40 * cm;
  fTargetX = 2 * cm;
  fTargetY = 2 * cm;
  fTargetZ = 2 * cm;
  fHPGeInnerR = 3 * cm;
  fHPGeOuterR = 15 * cm;
  fHPGeLength = 15 * cm;
  fSourcePosition = { 0, 0, -fWorldZ * 0.5 };
}

DetectorConstruction::~DetectorConstruction() { }

static G4Material *GetTargetMaterial()
{
  std::string name_in = "Th-229";
  if(char *envname = getenv("MUDET_TARGET_MATERIAL")) name_in = envname;

  auto nist = G4NistManager::Instance();
  G4Element *F_element = nist->FindOrBuildElement("F");
  G4Element *Ca_element = nist->FindOrBuildElement("Ca");
  G4Element *Sr_element = nist->FindOrBuildElement("Sr");

  // https://pubchem.ncbi.nlm.nih.gov/compound/167312#section=Computed-Properties
  // https://pubchem.ncbi.nlm.nih.gov/compound/11817119#section=Computed-Properties
  // https://pubchem.ncbi.nlm.nih.gov/element/90#section=Density
  auto Th_229_isotope = new G4Isotope("Th-229", 90, 229, 229.03176 * g / mole);
  auto Th_232_isotope = new G4Isotope("Th-232", 90, 232, 232.03805 * g / mole);
  auto Th_229_element = new G4Element("Th-229", "Th-229", 1);
  Th_229_element->AddIsotope(Th_229_isotope, 100. * perCent);
  auto Th_232_element = new G4Element("Th-232", "Th-232", 1);
  Th_232_element->AddIsotope(Th_232_isotope, 100. * perCent);
  auto Th_229_material = new G4Material("Th-229", 11.72 * (229.03176 / 232.0377) * g / cm3, 1);
  Th_229_material->AddElement(Th_229_element, 1);
  auto Th_232_material = new G4Material("Th-232", 11.72 * (232.03805 / 232.0377) * g / cm3, 1);
  Th_232_material->AddElement(Th_232_element, 1);

  auto build_Th_XF2_material = [=](const G4String &name, G4double Th_232_number_density, G4double Th_229_number_density,
                                   G4double pure_XF2_density, G4double pure_XF2_molar_mass, G4Element *X_element) {
    const G4double Th_232_density = Th_232_number_density / CLHEP::Avogadro * Th_232_isotope->GetA();
    const G4double Th_229_density = Th_229_number_density / CLHEP::Avogadro * Th_229_isotope->GetA();
    const G4double pure_XF2_number_density = pure_XF2_density / pure_XF2_molar_mass * CLHEP::Avogadro;
    const G4double Th_232_fraction = Th_232_number_density / pure_XF2_number_density;
    const G4double Th_229_fraction = Th_229_number_density / pure_XF2_number_density;
    G4double Th_XF2_density_boost = 1;
    Th_XF2_density_boost += Th_232_fraction * (Th_232_isotope->GetA() / X_element->GetA() - 1);
    Th_XF2_density_boost += Th_229_fraction * (Th_229_isotope->GetA() / X_element->GetA() - 1);
    const G4double Th_XF2_density = pure_XF2_density * Th_XF2_density_boost;

    const G4double X_number_density = pure_XF2_number_density - Th_232_number_density - Th_229_number_density;
    const G4double F_number_density = pure_XF2_number_density * 2;
    const G4double X_density = X_number_density / CLHEP::Avogadro * X_element->GetA();
    const G4double F_density = F_number_density / CLHEP::Avogadro * F_element->GetA();
    auto Th_XF2_material = new G4Material(name, Th_XF2_density, 4);
    Th_XF2_material->AddElement(Th_232_element, Th_232_density / Th_XF2_density);
    Th_XF2_material->AddElement(Th_229_element, Th_229_density / Th_XF2_density);
    Th_XF2_material->AddElement(X_element, X_density / Th_XF2_density);
    Th_XF2_material->AddElement(F_element, F_density / Th_XF2_density);
    return Th_XF2_material;
  };

  // https://www.crystran.com/optical-materials/calcium-fluoride-caf2/
  // https://www.crystran.com/optical-materials/strontium-fluoride-srf2/
  auto Th_CaF2_material =
      build_Th_XF2_material("Th:CaF2", 3.86e19 / cm3, 1.15e11 / cm3, 3.18 * g / cm3, 78.08 * g / mole, Ca_element);
  auto Th_SrF2_material =
      build_Th_XF2_material("Th:SrF2", 4.72e20 / cm3, 1.4e12 / cm3, 4.24 * g / cm3, 125.62 * g / mole, Sr_element);

  if(name_in == "Th-229") return Th_229_material;
  if(name_in == "Th-232") return Th_232_material;
  if(name_in == "Th:CaF2") return Th_CaF2_material;
  if(name_in == "Th:SrF2") return Th_SrF2_material;

  return G4NistManager::Instance()->FindOrBuildMaterial(name_in);  // e.g., G4_Al, G4_Cu
}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
  auto nist = G4NistManager::Instance();

  auto world_s = new G4Box("world", fWorldX * 0.5, fWorldY * 0.5, fWorldZ * 0.5);
  auto world_l = new G4LogicalVolume(world_s, nist->FindOrBuildMaterial("G4_Galactic"), "world");
  auto world_p = new G4PVPlacement(NULL, {}, world_l, "world", NULL, false, 0, true);

  auto target_s = new G4Box("target", fTargetX * 0.5, fTargetY * 0.5, fTargetZ * 0.5);
  auto target_l = new G4LogicalVolume(target_s, GetTargetMaterial(), "target");
  G4VisAttributes target_vis;
  target_vis.SetForceSolid();
  target_vis.SetColor(1.0, 0.0, 0.0, 0.8);
  target_l->SetVisAttributes(target_vis);
  new G4PVPlacement(NULL, {}, target_l, "target", world_l, false, 0, true);

  auto HPGe_s = new G4Tubs("HPGe", fHPGeInnerR, fHPGeOuterR, fHPGeLength * 0.5, 0, 360 * deg);
  auto HPGe_l = new G4LogicalVolume(HPGe_s, nist->FindOrBuildMaterial("G4_Ge"), "HPGe");
  G4VisAttributes HPGe_vis;
  HPGe_vis.SetForceSolid();
  HPGe_vis.SetColor(0.5, 0.5, 0.5, 0.3);
  HPGe_l->SetVisAttributes(HPGe_vis);
  new G4PVPlacement(NULL, {}, HPGe_l, "HPGe", world_l, false, 0, true);

  return world_p;
}
