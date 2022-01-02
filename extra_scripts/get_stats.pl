#!/usr/bin/perl

use warnings;
use strict;

my $header = "filter_chance,filter_first_lower_bound,filter_second_lower_bount,filter_first_upper_bount,filter_second_upper_bound,leave_empty_join_chance,join_chance,arithmetic_chance,aggregation_chance,multiplier_chance,generator_query_count,table_size_lower_bound,table_size_upper_bound,filter_global_count,linear_sort_global_count,merge_sort_global_count,merge_join_global_count,addition_global_count,multiplier_global_count,global_sum_global_count,final_query_count,filter_mean_count,filter_std_dev_count,filter_min_count,filter_max_count,filter_dnf_mean,filter_dnf_std_dev,filter_dnf_min,filter_dnf_max_count,filter_comparison_mean,filter_comparison_std_dev,filter_comparison_min,filter_comparison_max,linear_sort_mean_count,linear_sort_std_dev_count,linear_sort_min_count,linear_sort_max_count,merge_sort_mean_count,merge_sort_std_dev_count,merge_sort_min_count,merge_sort_max_count,merge_join_mean_count,merge_join_std_dev_count,merge_join_min_count,merge_join_max_count,addition_mean_count,addition_std_dev_count,addition_min_count,addition_max_count,multiplier_mean_count,multiplier_std_dev_count,multiplier_min_count,multiplier_max_count,global_sum_mean_count,global_sum_std_dev_count,global_sum_min_count,global_sum_max_count,table_size_mean,table_size_std_dev,table_size_sum_min,table_size_max_count,table_mean_count,table_std_dev_count,table_min_count,table_max_count,selectivity,plan_count,placed_nodes,discarded_placements,plans_chosen,run_count,performance_s";

my @a = (1..5);

my ($stats_filename, $graph_filename, $table_filename) = @ARGV;

if (not defined $table_filename) {
  die "Need filenames\n";
}

#my $filter_chance = 0.5;
#my $filter_first_lower_bound = 1;
#my $filter_second_lower_bount = 10;

#my $selectivity = 0.5;

open(my $stats_file, ">>$stats_filename");
print $stats_file "$header";

#foreach(@a){
	#open($stats_file, ">>$stats_filename");
    #print $stats_file "\n$filter_chance,$filter_first_lower_bound,$filter_second_lower_bount,5,20,0.5,0.5,0.5,0.5,0.5,3,10,10000";
    #close $stats_file;
    #my $ret1 = system("python benchmark_generator.py $stats_filename $graph_filename $table_filename");
    #my $ret2 = system("python graph_statistics.py $stats_filename $graph_filename $table_filename");
    #open($stats_file, ">>$stats_filename");
    #print $stats_file ",$selectivity";
    #close $stats_file;
    #my $ret3 = system("python schedule.py $stats_filename $graph_filename $table_filename");
#}

my @repeat_runs = (1..5);
my @filter_chance = (0.3,0.2);
my @table_low = (1,100,10000);
my @table_upper = (100,10000,1000000);
my @filter_dnf_low = (1,10);
my @filter_dnf_high = (8,16);
my @filter_comp_low = (1,4);
my @filter_comp_high = (4,8);
my @leave_empty_join = (0.75,0.25,0);
my @join_chance = (0.3,0.2);
my @arithmetic_chance = (0.3,0.2);
my @generator_query_count = (2,1);
my @selectivity = (0.75,0.5,0.25);

for my $run_i (@repeat_runs){
	for my $filter_c (@filter_chance){
        for(my $table_size_i = 0; $table_size_i <= $#table_low; $table_size_i++){
            for(my $filter_size_i = 0; $filter_size_i <= $#filter_dnf_low; $filter_size_i++) {
                for my $empty_j (@leave_empty_join){
                    for my $join_c (@join_chance){
                        for my $arith_c (@arithmetic_chance){
                            for my $query_c (@generator_query_count){
                                for my $select (@selectivity){
                                    open($stats_file, ">>$stats_filename");
                                    print $stats_file "\n$filter_c,$filter_dnf_low[$filter_size_i],$filter_comp_low[$filter_size_i],$filter_dnf_high[$filter_size_i],$filter_comp_high[$filter_size_i],$empty_j,$join_c,$arith_c,$arith_c,0.5,$query_c,$table_low[$table_size_i],$table_upper[$table_size_i]";
                                    close $stats_file;
                                    my $ret1 = system("python benchmark_generator.py $stats_filename $graph_filename $table_filename");
                                    my $ret2 = system("python graph_statistics.py $stats_filename $graph_filename $table_filename");
                                    open($stats_file, ">>$stats_filename");
                                    print $stats_file ",$select";
                                    close $stats_file;
                                    my $ret3 = system("python schedule.py $stats_filename $graph_filename $table_filename");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}