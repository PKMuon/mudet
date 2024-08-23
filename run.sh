#!/bin/bash

./mudet run.mac &
PID=$!
wait
hadd tree/$PID.root $(ls tree/$PID/*.root | sort -V)
rm -rf tree/$PID
