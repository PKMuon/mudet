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

#include "MuDiracDataHelper.hh"

#include <fstream>
#include <iostream>

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

MuDiracDataHelper *MuDiracDataHelper::GetInstance()
{
  static thread_local MuDiracDataHelper tInstance;
  return &tInstance;
}

MuDiracDataHelper::MuDiracDataHelper() { }

MuDiracDataHelper::~MuDiracDataHelper() { }

const std::vector<std::pair<G4double, G4double>> *MuDiracDataHelper::QueryData(G4int Z, G4int A, G4int ni, G4int nf)
{
  auto it = fDataMap.find({ Z, A, ni, nf });
  if(it != fDataMap.end()) return &it->second;

  std::stringstream filename;
  filename << "../mudirac_data/Z" << Z << "_A" << A << "_Ni" << ni << "_Nf" << nf << ".dat";
  std::ifstream file(filename.str());
  if(!file) return nullptr;

  std::vector<std::pair<G4double, G4double>> cdf;
  G4double energy, prob, cumulative = 0.0;
  while(file >> energy >> prob) {
    if(prob > 0) {
      cumulative += prob;
      cdf.emplace_back(energy * eV, cumulative);
    }
  }
  if(cumulative == 0) return nullptr;

  for(auto &[e, p] : cdf) { p /= cumulative; }
  it = fDataMap.emplace(std::make_tuple(Z, A, ni, nf), std::move(cdf)).first;
  return &it->second;
}

G4double MuDiracDataHelper::GetTransitionEnergy(G4int Z, G4int A, G4int ni, G4int nf)
{
  const std::vector<std::pair<G4double, G4double>> *data = QueryData(Z, A, ni, nf);
  if(data == nullptr) {
    G4cout << "Warning: No data for Z=" << Z << ", A=" << A << ", Ni=" << ni << ", Nf=" << nf << "." << G4endl;
    return 0.0 / 0.0;
  }
  auto it = std::upper_bound(data->begin(), data->end(), G4UniformRand(),
      [](G4double value, const std::pair<G4double, G4double> &entry) { return value < entry.second; });
  //G4cout << "Debug: Sampled energy " << it->first / MeV << " MeV for Z=" << Z << ", A=" << A << ", Ni=" << ni
  //       << ", Nf=" << nf << "." << G4endl;
  return it->first;
}
