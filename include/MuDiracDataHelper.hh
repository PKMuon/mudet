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

#ifndef MuDiracDataHelper_h
#define MuDiracDataHelper_h 1

#include <map>
#include <tuple>
#include <vector>

#include "globals.hh"

class MuDiracDataHelper {
public:
  static MuDiracDataHelper *GetInstance();

  G4double GetTransitionEnergy(G4int Z, G4int A, G4int nInitial, G4int nFinal);

private:
  MuDiracDataHelper();
  ~MuDiracDataHelper();
  MuDiracDataHelper(const MuDiracDataHelper &) = delete;
  MuDiracDataHelper &operator=(const MuDiracDataHelper &) = delete;

  std::map<std::tuple<G4int, G4int, G4int, G4int>, std::vector<std::pair<G4double, G4double>>> fDataMap;

  const std::vector<std::pair<G4double, G4double>> *QueryData(G4int Z, G4int A, G4int nInitial, G4int nFinal);
};

#endif
