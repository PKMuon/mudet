#!/bin/bash

./mudet run.mac &
PID=$!
wait
hadd tree/$PID.root $(ls tree/$PID/*.root | sort -V) && ln -sf $PID.root tree/latest.root
rm -rf tree/$PID
