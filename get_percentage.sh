#!/bin/bash

N_RUNS=1000
N_TESTS=5000
SUM=`cat $1 | cut -d' ' -f3 | awk '{s+=$1}END{print s}'`

echo "$SUM / ( $N_TESTS * $N_RUNS ) * 100" | bc -l
