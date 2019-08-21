#!/bin/bash

if [[ $# < 2 ]]; then
  echo "must specify [tag, depth] in that order"
  exit 1
fi

TAG=$1
DEPTH=$2
LENGTH="60"

DCS="30 40 50 60 70 60 50 40 30 40 50 60 70 30 40 50 60 70"
for dc in $DCS; do
  CMD="sudo ./rheometer -d $DEPTH -t $TAG -c data/constant-${DC}perc.json -l $LENGTH"
  echo $CMD
  $CMD
done
