#!/bin/bash

#!/bin/sh

usage ()
{
  echo 'Usage : ./run_test.sh <executable> <instances_file> <result_file> <timeout_in_seconds>' >&2
  exit
}

if [ "$#" -ne 4 ]
then
  usage
fi

instance_dir=$(sed 's|^\./||; s|/\*\.cnf||' "$2")
result_file="$1_${instance_dir}_$3"

if [ "$1" = "CCAnr_MAB" ]
then
  echo 'algorithm_version;instance;nb_vars;nb_clauses;ratio;nb_literals;avg_min_ max;seed;ls_no_improv_times;p_scale;q_scale;threshold;scale_ave;aspiration;nb_arms_MAB;lambda_MAB;delay_MAB;gamma_MAB;sat;steps;resolve_time' >> $result_file
else
  echo 'algorithm_version;instance;nb_vars;nb_clauses;ratio;nb_literals;avg_min_ max;seed;ls_no_improv_times;p_scale;q_scale;threshold;scale_ave;aspiration;sat;steps;resolve_time' >> $result_file

fi
for instance in `cat $2 | sort`
do
     ulimit -t $4
     r=`./$1 -inst $instance`
     echo $1';'$instance';'$r >> $result_file
     #echo "" >> $3
done
