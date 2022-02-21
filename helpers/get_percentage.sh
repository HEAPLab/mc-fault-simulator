#!/bin/bash

N_RUNS=$2
N_TESTS=80
SUM=`cat $1 | cut -d' ' -f3 | awk '{s+=$1}END{print s}'`

echo "scale=2; $SUM * 100 / ( $N_TESTS * $N_RUNS ) " | bc -l | tr -d '\n'
