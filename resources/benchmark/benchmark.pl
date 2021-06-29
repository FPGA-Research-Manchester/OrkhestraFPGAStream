#!/usr/bin/perl

use warnings;
use strict;

sub print_mean_and_dev {
    my @all_times = @_;

    my $mean = 0;
    foreach my $time (@all_times) {
        $mean += $time;
    }
    $mean = $mean / @all_times;
    print "Mean: $mean\n";
    my $std_dev = 0;
    foreach my $time (@all_times) {
        $std_dev += ($time - $mean)**2;
    }
    $std_dev = sqrt($std_dev);
    print "Std Dev: $std_dev\n"
}

my @input_defs=("TPCH_Q19_SF001.json", "TPCH_Q19_SF002.json", "TPCH_Q19_SF003.json");
my @input_configs=("benchmark_config.ini");

my $warm_up_runs = 3;
my $regular_runs = 20;

foreach my $input_def ( @input_defs ) {
    foreach my $input_config ( @input_configs ) {
        print "dbmstodspi -c $input_config -i $input_def\n";
        print "$warm_up_runs WARM UP RUNS\n";
        for (my $i = 0; $i < $warm_up_runs; $i++){
            system("sudo", "./dbmstodspi", "-c", "$input_config", "-i", "$input_def", "-q");
        } 
        
        my @accumulated_times = ();
        my @module_times = ();
        my $module_count = 0;
        
        print "$regular_runs BENCHMARK RUNS\n";
        for (my $i = 0; $i < $regular_runs; $i++){
            my @current_run_module_times = ();
            my $run_time = 0;
            my $output = `sudo ./dbmstodspi -c $input_config -i $input_def`;
            foreach my $line (split /[\r\n]+/, $output) {
                if ($line =~ /.*Execution time = (\d+).*/) {
                    push @current_run_module_times, $1;
                }
            }
            $module_count = @current_run_module_times;
            foreach my $time (@current_run_module_times) {
                $run_time += $time;
                push @module_times, $time;
            }
            push @accumulated_times, $run_time;
        }
        print "\nAccumulated times:\n";
        print_mean_and_dev(@accumulated_times);

        for(my $i = 0; $i < $module_count; $i++){
            my @current_module_times = ();
            for(my $j = 0; $j <= $#accumulated_times; $j++){
                push @current_module_times, @module_times[$i + $module_count * $j];
            }
            print "\nConfiguration $i:\n";
            print_mean_and_dev(@current_module_times);
        }
    }
}

