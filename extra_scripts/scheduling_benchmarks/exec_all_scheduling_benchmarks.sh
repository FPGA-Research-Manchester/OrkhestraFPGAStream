#!/bin/bash

start_time=$(date +%s)

for file in stats1.csv graph1.json table1.json backup1.gz count1.csv final1.csv failure_stats1.json stats2.csv graph2.json table2.json backup2.gz count2.csv final2.csv failure_stats2.json stats3.csv graph3.json table3.json backup3.gz count3.csv final3.csv failure_stats3.json output1.txt output2.txt output3.txt; do
    if [ -e "$file" ]; then
        echo "Error: file $file already exists."
        exit 1
    fi
done

perl get_stats_compare_heuristics.pl stats1.csv graph1.json table1.json backup1.gz count1.csv final1.csv failure_stats1.json > output1.txt

elapsed_time=$(($(date +%s) - $start_time))
echo "get_stats_compare_heuristics.pl took $elapsed_time seconds to run"

start_time=$(date +%s)

perl get_stats_max_heuristics.pl stats2.csv graph2.json table2.json backup2.gz count2.csv final2.csv failure_stats2.json > output2.txt

elapsed_time=$(($(date +%s) - $start_time))
echo "get_stats_max_heuristics.pl took $elapsed_time seconds to run"

start_time=$(date +%s)

perl get_stats_timelimits.pl stats3.csv graph3.json table3.json backup3.gz count3.csv final3.csv failure_stats3.json > output3.txt

elapsed_time=$(($(date +%s) - $start_time))
echo "get_stats_timelimits.pl took $elapsed_time seconds to run"

total_time=$(($(date +%s) - $start_time + $elapsed_time + $elapsed_time))
echo "The three Perl scripts took a total of $total_time seconds to run"
