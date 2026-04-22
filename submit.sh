#!/bin/bash

exec hep_sub ./mudet -argu run.mac %{ProcId} -n 1000 -o run_1.log -e run_2.log
