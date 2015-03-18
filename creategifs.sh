#!/bin/sh

for f in bin/data/output/*
do
  if [ -d $f ]
  then
    if [ -e "$f.gif" ]
    then
      echo "Skipping $f"
    else
      echo "Processing $f"
      gifsicle --delay=5 --loop --colors 256 $f/*.gif > "$f.gif"
    fi
  fi
done

