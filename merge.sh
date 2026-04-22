#!/bin/bash

hadd tree/latest.root $(seq 0 999 | sed 's@\(.*\)@tree/\1/*.root@')
