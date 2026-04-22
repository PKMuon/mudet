#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

file = uproot.open(f"../build/tree/latest.root")
events = file["tree"].arrays(["Events.E", "Events.Pid"])
print(f"Number of events: {len(events)}")
print(f"Number of particles: {len(ak.flatten(events['Events.Pid']))}")

pids = np.unique(ak.flatten(np.abs(events["Events.Pid"])))

bins = np.logspace(0, 7, 71)
for pid in pids:
    energies = ak.flatten(events["Events.E"][np.abs(events["Events.Pid"]) == pid])
    plt.hist(energies, bins=bins, histtype="step", label=f"CRY abs(pid)={pid}")
plt.xlabel(r"$E$ [MeV]")
plt.ylabel("Events")
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig(f"cry.pdf")
