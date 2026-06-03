#!/bin/bash

for instance in `cat $2 | sort`
do
     ulimit -t 15
     r=`./$1 -inst $instance  | tail -n $4`
     echo $instance $r >> $3
     echo "" >> $3
done
