#include "MuDiracDataHelper.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4NucleiProperties.hh"
#include "G4MuonMinus.hh"
#include "Randomize.hh"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>


MuDiracDataHelper* MuDiracDataHelper::fpInstance = nullptr;

MuDiracDataHelper* MuDiracDataHelper::GetInstance()
{
  if (!fpInstance) {
    fpInstance = new MuDiracDataHelper();
  }
  return fpInstance;
}

MuDiracDataHelper::MuDiracDataHelper() {}
MuDiracDataHelper::~MuDiracDataHelper() {}

std::string MuDiracDataHelper::GetKey(G4int Z, G4int A, G4int ni, G4int nf) const
{
  std::stringstream ss;
  ss << Z << "_" << A << "_" << ni << "_" << nf;
  return ss.str();
}

void MuDiracDataHelper::LoadData(G4int Z, G4int A, G4int ni, G4int nf)
{
  std::string key = GetKey(Z, A, ni, nf);
  
  if (fDataMap.find(key) != fDataMap.end()) return;

  std::vector<std::pair<G4double, G4double>> cdf;
  
  std::stringstream filename;
  filename << "./mudirac_data/Z" << Z << "_A" << A << "_Ni" << ni << "_Nf" << nf << ".dat";
  
  std::ifstream file(filename.str());
  
  if (file.is_open()) {
    G4double energy, prob;
    G4double cumulative = 0.0;
    
    while (file >> energy >> prob) {
      if (prob > 0) {
        cumulative += prob;
        cdf.push_back({energy * eV, cumulative}); 
      }
    }
    
    if (cumulative > 0) {
      for (auto &pair : cdf) {
        pair.second /= cumulative;
      }
    }
  } 
  else { G4cout << "MuDirac file missing: " << filename.str() << G4endl; }

  fDataMap[key] = cdf;
}

G4double MuDiracDataHelper::GetTransitionEnergy(G4int Z, G4int A, G4int nLevelInitial, G4int nLevelFinal)
{

  G4int nInitial = nLevelInitial + 1;
  G4int nFinal = nLevelFinal + 1;
  LoadData(Z, A, nInitial, nFinal);
  
  std::string key = GetKey(Z, A, nInitial, nFinal);
  const auto& cdf = fDataMap[key];

  if (cdf.empty()) {
    // G4cout << "[Warning] No data for Z=" << Z << ", A=" << A << ", nInitial=" << nInitial << ", nFinal=" << nFinal 
    //        << ". Using Bohr model approximation." << G4endl;
    G4double muMass = G4MuonMinus::MuonMinus()->GetPDGMass();
    G4double massA = G4NucleiProperties::GetNuclearMass(A, Z);
    G4double reducedMass = muMass * massA / (muMass + massA);
    
    G4double rydberg = 13.6 * eV * (Z * Z) * reducedMass / electron_mass_c2;
    
    return rydberg * (1.0/(nFinal*nFinal) - 1.0/(nInitial*nInitial));
  }

  G4double rnd = G4UniformRand();
  auto it = std::lower_bound(cdf.begin(), cdf.end(), rnd, 
      [](const std::pair<G4double, G4double>& element, G4double value) {
          return element.second < value;
      });

  if (it != cdf.end()) {
      return it->first;
  }
  return cdf.back().first;
}
