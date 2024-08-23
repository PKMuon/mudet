# Studies on PKMuon Detector

Contact \<seeson@pku.edu.cn\> on any problem.

Setup ROOT and Geant4 environment before everything.

Compile at the first time:

```bash
cd mudet
mkdir build
cd build
cmake ..
make -j
```

Recompile (needed on any modification on source files (`.cc`)) afterwards:

```bash
cd mudet/build
make -j
```

Launch GUI:

```bash
cd mudet/build
./mudet
```

Launch simulation (with multi-thread parallelism):

```bash
cd mudet/build
./run.sh
```

Analyze data from the previous step:

TBD
