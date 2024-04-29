#!/bin/bash

# run all sh except itself

for x in "$(dirname $0)"/*.sh
do
  if [ "$(basename $x)" != "$(basename $0)" ]
  then
    open "$x"
  fi
done