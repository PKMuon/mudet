#!/usr/bin/env python3

import uproot
import awkward as ak

file = uproot.open(f"../build/tree/latest.root")
events = file["tree"].arrays(["Edeps.Edep", "Tracks.E", "Tracks.Pid"])
print(f"Number of events: {len(events)}")
edeps = ak.sum(events["Edeps.Edep"], axis=1)
events = events[(edeps >= 0.12) & (edeps <= 0.15)]
del edeps
print(f"Number of events passing selection: {len(events)}")
ak.to_parquet(events, f"../build/tree/latest.parquet")
