#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(15, 4))

file = uproot.open(f"../build/tree/latest.root")
nmuon = int(file["nmuon"].to_numpy()[0][0])
nmuon_one_year = 19 * 19 * 86400 * 365 // 60
print(f"Number of incident muons: {nmuon} / {nmuon_one_year}")
weight = nmuon_one_year / nmuon
events = file["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid"])
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

for i, (cut, bins) in enumerate([((0.050, 0.200), 150), ((0.130, 0.140), 10), ((0.155, 0.165), 10)], 1):
    plt.subplot(1, 3, i)

    n, bins_, _ = plt.hist(gamma_energies, bins=bins, range=cut, weights=np.ones_like(gamma_energies) * weight, histtype="step", color="blue", label=f"Geant4/MuDirac $E_\\gamma$")
    plt.errorbar(0.5 * (bins_[:-1] + bins_[1:]), n, yerr=np.maximum(np.sqrt(n), np.sqrt(n / weight) * weight), fmt="none", color="blue", elinewidth=1.0, capsize=0)

    n, bins_, _ = plt.hist(total_edep, bins=bins, range=cut, weights=np.ones_like(total_edep) * weight, histtype="step", color="red", label=f"HPGe $E_\\mathrm{{dep}}$")
    plt.errorbar(0.5 * (bins_[:-1] + bins_[1:]), n, yerr=np.maximum(np.sqrt(n), np.sqrt(n / weight) * weight), fmt="none", color="red", elinewidth=1.0, capsize=0)

    plt.xlabel(r"$E$ [MeV]")
    plt.ylabel("Events / year")
    plt.legend()
    plt.grid()

plt.tight_layout()
plt.savefig(f"muhpge_latest.pdf")
