#!/bin/bash

for file in stats1.csv graph1.json table1.json backup1.gz count1.csv final1.csv failure_stats1.json stats2.csv graph2.json table2.json backup2.gz count2.csv final2.csv failure_stats2.json stats3.csv graph3.json table3.json backup3.gz count3.csv final3.csv failure_stats3.json output1.txt output2.txt output3.txt; do
    if [ -e "$file" ]; then
        echo "Error: file $file already exists."
        exit 1
    fi
done

echo "Running get_stats_compare_heuristics.pl..."
start_time_1=$(date +%s)
perl get_stats_compare_heuristics.pl stats1.csv graph1.json table1.json backup1.gz count1.csv final1.csv failure_stats1.json > output1.txt &
wait
end_time_1=$(date +%s)
time_1=$((end_time_1 - start_time_1))
echo "get_stats_compare_heuristics.pl took $time_1 seconds to run"

echo "Running get_stats_max_heuristics.pl..."
start_time_2=$(date +%s)
perl get_stats_max_heuristics.pl stats2.csv graph2.json table2.json backup2.gz count2.csv final2.csv failure_stats2.json > output2.txt &
wait
end_time_2=$(date +%s)
time_2=$((end_time_2 - start_time_2))
echo "get_stats_max_heuristics.pl took $time_2 seconds to run"

echo "Running get_stats_timelimits.pl..."
start_time_3=$(date +%s)
perl get_stats_timelimits.pl stats3.csv graph3.json table3.json backup3.gz count3.csv final3.csv failure_stats3.json > output3.txt &
wait
end_time_3=$(date +%s)
time_3=$((end_time_3 - start_time_3))
echo "get_stats_timelimits.pl took $time_3 seconds to run"

total_time=$((time_1 + time_2 + time_3))
echo "The three Perl scripts took a total of $total_time seconds to run"
