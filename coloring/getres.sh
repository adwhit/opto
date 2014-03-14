#!/bin/bash

FILES="gc_50_3 
gc_70_7
gc_100_5
gc_250_9
gc_500_1
gc_1000_5"

for f in $FILES
do 
    ./graph data/$f -s > output/$f
done
