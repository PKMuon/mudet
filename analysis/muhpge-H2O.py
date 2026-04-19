#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(15, 4))

events = uproot.open(f"../build/tree/latest.root")["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid"])
#events = uproot.open(f"../build/tree/latest.root")["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid", "Events.Pid"])
print(f"Number of events: {len(events)}")
events = events[ak.num(events["Edeps.Edep"], axis=1) >= 1]
print(f"Number of events with at least one energy deposition: {len(events)}")
#events = events[ak.num(events["Events.Pid"], axis=1) > 1]
#print(f"Number of events with at least one particle on target: {len(events)}")

gamma_energies = events["Tracks.E"][events["Tracks.Pid"] == 22]
gamma_energies = ak.flatten(gamma_energies).to_numpy()
print(f"Number of gammas: {len(gamma_energies)}")
total_edep = ak.sum(events["Edeps.Edep"], axis=1).to_numpy()

for i, (cut, bins) in enumerate([((0.000, 0.200), 100), ((0.120, 0.150), 30), ((0.145, 0.175), 30)], 1):
    plt.subplot(1, 3, i)
    plt.hist(gamma_energies, bins=bins, range=cut, weights=np.ones_like(gamma_energies), histtype="step", label=f"Geant4/MuDirac $E_\\gamma$")
    plt.hist(total_edep, bins=bins, range=cut, histtype="step", label=f"HPGe $E_\\mathrm{{dep}}$")
    plt.xlabel(r"$E$ [MeV]")
    plt.ylabel("Events")
    plt.legend()
    plt.grid()

plt.tight_layout()
plt.savefig(f"muhpge_latest.pdf")
