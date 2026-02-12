#!/bin/bash

PID_BEG="" PID_END="" INPUTS="" TEMPS=""
for i in $(seq $(nproc)); do
    ./mudet run.mac &
    [ -z "${PID_BEG}" ] && PID_BEG="$!"
    PID_END="$!"
    INPUTS="${INPUTS} tree/$!/$!.root"
    TEMPS="${TEMPS} tree/$!"
done
wait
hadd "tree/${PID_BEG}-${PID_END}.root" ${INPUTS} && ln -sf "${PID_BEG}-${PID_END}.root" tree/latest.root
rm -rf ${TEMPS}
