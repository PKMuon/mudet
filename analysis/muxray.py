#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

basename = "Cu_old"
tracks = uproot.open(f"../build/tree/{basename}.root")["tree"]["Tracks"].arrays()

# ['Tracks.fUniqueID', 'Tracks.fBits', 'Tracks.Id', 'Tracks.Mother', 'Tracks.Pid', 'Tracks.Px', 'Tracks.Py', 'Tracks.Pz', 'Tracks.E', 'Tracks.X', 'Tracks.Y', 'Tracks.Z', 'Tracks.T']
print(tracks.fields)

gammas = tracks[tracks["Tracks.Pid"] == 22]
gamma_energies = ak.flatten(gammas["Tracks.E"]).to_numpy()

plt.hist(gamma_energies, bins=100, label="Geant4")
plt.xlabel(r"$E_\gamma$ [MeV]")
plt.ylabel("Events")
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig(f"{basename}.pdf")
plt.clf()

plt.hist(gamma_energies, bins=100, range=[1.4, 1.7], label="Geant4")
plt.xlabel(r"$E_\gamma$ [MeV]")
plt.ylabel("Events")
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig(f"{basename}_1.4-1.7.pdf")
plt.clf()

plt.hist(gamma_energies, bins=100, range=[0.10, 0.36], label="Geant4")
plt.xlabel(r"$E_\gamma$ [MeV]")
plt.ylabel("Events")
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig(f"{basename}_0.10-0.36.pdf")
plt.clf()
