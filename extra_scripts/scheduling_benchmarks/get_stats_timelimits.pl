#!/usr/bin/perl

use warnings;
use strict;
use POSIX qw/strftime/;
use File::ReadBackwards;
use Time::HiRes qw( time );

my $start = time();
print strftime('%Y-%m-%d %H:%M:%S',localtime time);
print "\n";

my $header = "query_id,filter_chance,filter_first_lower_bound,filter_second_lower_bount,filter_first_upper_bount,filter_second_upper_bound,leave_empty_join_chance,join_chance,arithmetic_chance,aggregation_chance,multiplier_chance,generator_query_count,table_size_lower_bound,table_size_upper_bound,max_node_limit,min_node_limit,filter_global_count,linear_sort_global_count,merge_sort_global_count,merge_join_global_count,addition_global_count,multiplier_global_count,global_sum_global_count,global_node_count,node_count_mean,node_count_std_dev,node_count_min,node_count_max,final_query_count,filter_mean_count,filter_std_dev_count,filter_min_count,filter_max_count,filter_dnf_mean,filter_dnf_std_dev,filter_dnf_min,filter_dnf_max_count,filter_comparison_mean,filter_comparison_std_dev,filter_comparison_min,filter_comparison_max,linear_sort_mean_count,linear_sort_std_dev_count,linear_sort_min_count,linear_sort_max_count,merge_sort_mean_count,merge_sort_std_dev_count,merge_sort_min_count,merge_sort_max_count,merge_join_mean_count,merge_join_std_dev_count,merge_join_min_count,merge_join_max_count,addition_mean_count,addition_std_dev_count,addition_min_count,addition_max_count,multiplier_mean_count,multiplier_std_dev_count,multiplier_min_count,multiplier_max_count,global_sum_mean_count,global_sum_std_dev_count,global_sum_min_count,global_sum_max_count,table_size_mean,table_size_std_dev,table_size_sum_min,table_size_max_count,table_mean_count,table_std_dev_count,table_min_count,table_max_count,selectivity,time_limit,heuristic,utility_scaler,frame_scaler,utility_per_frame_scaler,plan_count,placed_nodes,discarded_placements,plans_chosen,run_count,performance_s,cost_eval_s,timeouts,utility,frames_written,utility_per_frames,score,exec_time,config_time,exec_and_config";

my ($stats_filename, $graph_filename, $table_filename, $backup_filename, $count_stats, $final_stats, $pass_fail_json) = @ARGV;
# perl get_stats_compare_heuristics.pl stats.csv graph.json table.json backup.gz count.csv final.csv failure_stats.json
print "perl get_stats_compare_heuristics.pl $stats_filename $graph_filename $table_filename $backup_filename $count_stats $final_stats $pass_fail_json\n";

if (not defined $table_filename) {
  die "Need filenames\n";
}

open(my $stats_file, ">>$stats_filename");
print $stats_file "$header";

#my @repeat_runs = (1..5);
#my @filter_chance = (0.4,0.2);
#my @table_low = (1000,100000);
#my @table_upper = (100000,10000000);
#my @filter_dnf_low = (1,10);
#my @filter_dnf_high = (8,32);
#my @filter_comp_low = (1,2);
#my @filter_comp_high = (3,4);
#my @leave_empty_join = (0.75,0.25,0);
#my @join_chance = (0.4,0.2);
#my @arithmetic_chance = (0.4,0.2);
#my @generator_query_count = (4,3);
#my @selectivity = (0.75,0.5,0.25,0.1);
#my @timeouts = (0.01,0.1,0.2,0.4,0.6,0.8,1,2,3);
#my @timeouts = (0.01);
#my $max_node_limit = 25;
#my $min_node_limit = 12;
#my $equal_scaler = 0;
#my $preferred_scaler = 1;
#my @heuristic_choice = (0,1,2,3,4,5,6);

#my @repeat_runs = (1..5);
my @repeat_runs = (1);
my @filter_chance = (0.5,0.4);
my @table_low = (100);
my @table_upper = (10000);
my @filter_dnf_low = (1,10);
my @filter_dnf_high = (8,32);
my @filter_comp_low = (1,2);
my @filter_comp_high = (3,4);
my @leave_empty_join = (0.75,0.25,0);
my @join_chance = (0.5,0.4);
my @arithmetic_chance = (0.5,0.4);
my @generator_query_count = (2,1);
my @selectivity = (1);
my @timeouts = (0.01,0.1,0.2,0.4,0.6,0.8,1,2,3);
my $max_node_limit = 7;
my $min_node_limit = 5;
my $equal_scaler = 0;
my $preferred_scaler = 1;
#my @heuristic_choice = (6,5);
my @heuristic_choice = (0,1,2,3,4,5,6);

# With 1 repeated run, 1 heuristic - 8*3*8 = 192 runs

my $count = 0;
my $tolerate_errors = 0;

for my $run_i (@repeat_runs){
    print $run_i;
    print "\n";
	for my $filter_c (@filter_chance){
        for(my $table_size_i = 0; $table_size_i <= $#table_low; $table_size_i++){
            for(my $filter_size_i = 0; $filter_size_i <= $#filter_dnf_low; $filter_size_i++) {
                for my $empty_j (@leave_empty_join){
                    for my $join_c (@join_chance){
                        for my $arith_c (@arithmetic_chance){
                            for my $query_c (@generator_query_count){
                                $count++;
                                open($stats_file, ">>$stats_filename");
                                print $stats_file "\n$count,$filter_c,$filter_dnf_low[$filter_size_i],$filter_comp_low[$filter_size_i],$filter_dnf_high[$filter_size_i],$filter_comp_high[$filter_size_i],$empty_j,$join_c,$arith_c,$arith_c,0.5,$query_c,$table_low[$table_size_i],$table_upper[$table_size_i],$max_node_limit,$min_node_limit";
                                close $stats_file;
                                # 1 at the end indicates c version
                                my $query_generation = system("python3 benchmark_generator.py $stats_filename $graph_filename $table_filename 1");
                                if ($query_generation != 0 && !$tolerate_errors) {
                                    die "Error: Query generation script failed with exit code $query_generation\n";
                                }
                                my $backup = system("python3 benchmark_backup.py $graph_filename $table_filename $backup_filename -1");
                                if ($backup != 0 && !$tolerate_errors) {
                                    die "Error: Backup script failed with exit code $backup\n";
                                }
                                my $stats_generation = system("python3 graph_statistics_c.py $stats_filename $graph_filename $table_filename");
                                if ($stats_generation != 0 && !$tolerate_errors) {
                                    die "Error: Stats script failed with exit code $stats_generation\n";
                                }
                                tie *BW, 'File::ReadBackwards', $stats_filename or
                                    die "can't read $stats_filename $!" ;
                                my $generated_query = <BW>;
                                close BW;
                                my $scheduling_return = 0;
                                for(my $j = 0; $j <= $#selectivity and $scheduling_return == 0; $j++){
                                    for(my $i = 0; $i <= $#timeouts and $scheduling_return == 0; $i++){
                                        for(my $k = 0; $k <= $#heuristic_choice and $scheduling_return == 0; $k++){
                                            open($stats_file, ">>$stats_filename");
                                            print $stats_file ",$selectivity[$j],$timeouts[$i],$heuristic_choice[$k],$equal_scaler,$equal_scaler,$preferred_scaler";
                                            close $stats_file;
                                            $scheduling_return = system("python3 schedule_c.py OrkhestraFPGAStream default_config.ini $graph_filename $stats_filename $table_filename");
                                            if ($scheduling_return != 0 && !$tolerate_errors) {
                                                die "Error: Scheduling script failed with exit code $scheduling_return\n";
                                            }
                                            if (!($i == $#timeouts and $j == $#selectivity and $k == $#heuristic_choice)){
                                                open($stats_file, ">>$stats_filename");
                                                print $stats_file "\n$generated_query";
                                                close $stats_file;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}



print strftime('%Y-%m-%d %H:%M:%S',localtime time);
print "\n";

# output_count_filename = argv[0]
# input_stats_filename = argv[1]
# output_stats_filename = argv[2]
# graph_type = int(argv[3])
# stats_json = argv[4]

# 0 - just pass fail, 1 runtime with all heuristics, 2 max heuristics, 3 timeouts, 4 selectivity
print "python3 stats_table_generator.py $count_stats $stats_filename $final_stats 3 $pass_fail_json\n";
my $final_processing = system("python3 stats_table_generator.py $count_stats $stats_filename $final_stats 3 $pass_fail_json");
if ($final_processing != 0) {
    die "Error: Results parsing script failed with exit code $final_processing\n";
}

my $end = time();
printf("Execution Time: %0.02f s\n", $end - $start);
printf("Count: $count\n");

print "python3 stats_to_graphs.py $final_stats 3\n"
