#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

basename = "Cu"

plt.figure(figsize=(15, 4))
for suffix in ["", "_old"]:
    label = basename + ": " + ("Geant4 with MuDirac" if suffix == "" else "Geant4" if suffix == "_old" else "unknown")
    tracks = uproot.open(f"../build/tree/{basename}{suffix}.root")["tree"]["Tracks"].arrays()
    print(tracks.fields)  # ['Tracks.fUniqueID', 'Tracks.fBits', 'Tracks.Id', 'Tracks.Mother', 'Tracks.Pid', 'Tracks.Px', 'Tracks.Py', 'Tracks.Pz', 'Tracks.E', 'Tracks.X', 'Tracks.Y', 'Tracks.Z', 'Tracks.T']
    gammas = tracks[tracks["Tracks.Pid"] == 22]
    gamma_energies = ak.flatten(gammas["Tracks.E"]).to_numpy()

    for i, cut in enumerate([None, (1.47, 1.54), (0.30, 0.40)], 1):
        plt.subplot(1, 3, i)
        plt.hist(gamma_energies, bins=200, range=cut, histtype="step", label=label)
        plt.xlabel(r"$E_\gamma$ [MeV]")
        plt.ylabel("Events")
        plt.legend()
        plt.grid()

plt.tight_layout()
plt.savefig(f"{basename}.pdf")
