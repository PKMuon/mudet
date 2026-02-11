#ifndef MuDiracDataHelper_h
#define MuDiracDataHelper_h 1

#include "globals.hh"
#include <vector>
#include <map>
#include <string>

class MuDiracDataHelper
{
public:
  static MuDiracDataHelper* GetInstance();
  G4double GetTransitionEnergy(G4int Z, G4int A, G4int nInitial, G4int nFinal);

private:
  MuDiracDataHelper();
  ~MuDiracDataHelper();
  MuDiracDataHelper(const MuDiracDataHelper&) = delete;
  MuDiracDataHelper& operator=(const MuDiracDataHelper&) = delete;
  void LoadData(G4int Z, G4int A, G4int ni, G4int nf);
  std::string GetKey(G4int Z, G4int A, G4int ni, G4int nf) const;
  std::map<std::string, std::vector<std::pair<G4double, G4double>>> fDataMap;  
  static MuDiracDataHelper* fpInstance;
};

#endif
