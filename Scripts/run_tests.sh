#!/bin/bash

for instance in `cat $1 | sort`
do
     ulimit -t 3600
     r=`./CCAnr_original -inst $instance  | tail -n 4`
     echo $instance $r >> $2
     echo "" >> $2
done
