#!/bin/bash

FILES=data/*

for f in $FILES
do
    echo $f
    ./knap $f
    echo 
done
