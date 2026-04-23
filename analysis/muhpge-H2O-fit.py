#!/usr/bin/env python3

import awkward as ak
import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(5, 4))

nmuon = 351560897
nmuon_one_year = 19 * 19 * 86400 * 365 // 60
print(f"Number of incident muons: {nmuon} / {nmuon_one_year}")
weight = nmuon_one_year / nmuon
events = ak.from_parquet(f"../build/tree/latest.parquet")
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

for i, (cut, bins) in enumerate([((0.120, 0.150), 30)], 1):
    plt.subplot(1, 1, i)

    n, bins_, _ = plt.hist(gamma_energies, bins=bins, range=cut, weights=np.ones_like(gamma_energies) * weight, histtype="step", color="blue", label=f"Geant4/MuDirac $E_\\gamma$")
    plt.errorbar(0.5 * (bins_[:-1] + bins_[1:]), n, yerr=np.maximum(np.sqrt(n), np.sqrt(n / weight) * weight), fmt="none", color="blue", elinewidth=1.0, capsize=0)

    n, bins_, _ = plt.hist(total_edep, bins=bins, range=cut, weights=np.ones_like(total_edep) * weight, histtype="step", color="red", label=f"HPGe $E_\\mathrm{{dep}}$")
    plt.errorbar(0.5 * (bins_[:-1] + bins_[1:]), n, yerr=np.maximum(np.sqrt(n), np.sqrt(n / weight) * weight), fmt="none", color="red", elinewidth=1.0, capsize=0)

    def f(x, a, b, c, d, e, x0, dx):
        return a * x**3 + b * x**2 + c * x + d + e * np.exp(-((x - x0) / dx)**2 / 2)
    x = 0.5 * (bins_[1:] + bins_[:-1])
    y = n
    yerr = np.maximum(np.sqrt(n), np.sqrt(n / weight) * weight)
    from scipy.optimize import curve_fit
    popt, pcov = curve_fit(f, x, y, p0=(1e-3, 1e-3, (y[-1] - y[0]) / (x[-1] - x[0]), y[0], 1e3, np.mean(x), 1e-3), sigma=yerr, absolute_sigma=True)
    x_fit = np.linspace(cut[0], cut[1], 1001)
    plt.plot(x_fit, f(x_fit, *popt), color="gray", label=f"Fit ($E_\\gamma = {popt[5]:.6f} \pm {np.sqrt(pcov[5][5]):.6f}$ MeV)")

    plt.xlabel(r"$E$ [MeV]")
    plt.ylabel("Events / year")
    plt.legend()
    plt.grid()

plt.tight_layout()
plt.savefig(f"muhpge_latest.pdf")
