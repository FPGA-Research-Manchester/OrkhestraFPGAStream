#!/bin/bash

input_defs=(TPCH_Q19_SF001.json TPCH_Q19_SF01.json)
input_configs=(config.ini)

warm_up_runs=3
regular_runs=5

for input_def in ${input_defs[@]}
do
  for input_config in ${input_configs[@]}
  do
        echo "dbmstodspi -c $input_config -i $input_def"
        all_times=()
        echo "WARM UP runs"
        for i in $(seq 1 $warm_up_runs)
        do
                sudo ./dbmstodspi -c "$input_config" -i "$input_def" -q
        done
        echo "Benchmark runs"
        for i in $(seq 1 $regular_runs)
        do
                module_times=()
                run_time=0
                regex='.*Execution time = ([0-9]+).*'
                while read line
                do
                        if [[ $line =~ $regex ]]
                        then
                                module_times+=(" ${BASH_REMATCH[1]} ")
                        fi
                done < <(sudo ./dbmstodspi -c "$input_config" -i "$input_def")
                for time in ${module_times[@]}
                do
                        run_time=$(($run_time + $time))
                done
                all_times+=($run_time)
        done
        mean=0
        std_dev=0
        for time in ${all_times[@]}
        do
            mean=$(($mean + $time))
        done
        mean=$(($mean / ${#all_times[@]}))
        echo "Mean: $mean"
  done
done
