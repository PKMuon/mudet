#!/usr/bin/env python3

import uproot
import awkward as ak

file = uproot.open(f"../build/tree/latest.root")
events = file["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid"])
print(f"Number of events: {len(events)}")
events = events[ak.num(events["Edeps.Edep"], axis=1) >= 1]
print(f"Number of events with at least one energy deposition: {len(events)}")
events = events[events["Edeps.Edep"] >= 0.05]
print(f"Number of events with energy deposition >= 0.05 MeV: {len(events)}")
ak.to_parquet(f"../build/tree/latest.parquet", events)
