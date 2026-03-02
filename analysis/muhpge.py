#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(15, 4))

events = uproot.open(f"../build/tree/latest.root")["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid"])
events = events[ak.num(events["Edeps.Edep"], axis=1) >= 1]
gamma_energies = events["Tracks.E"][events["Tracks.Pid"] == 22]
gamma_energies = ak.flatten(gamma_energies).to_numpy()
total_edep = ak.sum(events["Edeps.Edep"], axis=1).to_numpy()

for i, cut in enumerate([(0, 10), (6.05, 6.55), (9.29, 9.39)], 1):
    plt.subplot(1, 3, i)
    plt.hist(gamma_energies, bins=200, range=cut, histtype="step", label=f"Geant4/MuDirac $E_\\gamma$")
    plt.hist(total_edep, bins=200, range=cut, histtype="step", label=f"HPGe $E_\\mathrm{{dep}}$")
    plt.xlabel(r"$E$ [MeV]")
    plt.ylabel("Events")
    plt.legend()
    plt.grid()

plt.tight_layout()
plt.savefig(f"muhpge_latest.pdf")
