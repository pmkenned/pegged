#!/bin/bash

if [ $# -le 0 ]; then
    echo "must specify program" > /dev/stderr
    exit 1
fi
PROG=$1

N=3
if [ $# -gt 1 ]; then
    N=$2
fi

i=1
TIMEFORMAT='%R'
min="10000"
max="0"
sum="0"
avg="0"
while [ "$i" -le "$N" ]; do
    t="$(time ( $PROG $i > /dev/null) 2>&1)"
    sum="$(echo $sum + $t | bc -l)"
    echo "$i: $t"
    i=$(($i + 1))
    if (( $(echo "$t < $min" | bc -l) )); then
        min=$t
    fi
    if (( $(echo "$t > $max" | bc -l) )); then
        max=$t
    fi
done

echo  "MINIMUM: $min"
echo  "MAXIMUM: $max"

avg="$(echo "$sum / $N" | bc -l)"
echo -n "AVERAGE: "
printf "%.*f\n" "3" "$avg"
