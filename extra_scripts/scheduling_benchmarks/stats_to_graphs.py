# Copyright 2023 University of Manchester
#
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http:  // www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import csv
import sys
import copy
import numpy as np
import math
import json
import pandas as pd
import matplotlib.pyplot as plt

def main(argv):
    if len(argv) != 2:
        print(argv)
        raise ValueError("Wrong amount of args given!")

    input_stats_filename = argv[0]
    graph_type = argv[1]

    if graph_type == 0:
        with open(input_stats_filename, 'r') as file:
            reader = csv.reader(file)
            for row in reader:
                print(row)
    else:
        show_bar_chart_heuristics(input_stats_filename)


def show_bar_chart_heuristics(input_stats_filename):
    # df = pd.read_csv(input_stats_filename, dtype={'heuristic': 'category'})
    # df_filtered = df[df['aggregation_func'] == 'Avg']
    #
    # df_exec_time = df_filtered.pivot(index='heuristic', columns='exec_time', values='exec_time')
    # df_config_time = df_filtered.pivot(index='heuristic', columns='config_time', values='config_time')
    # df_plan_count = df_filtered.pivot(index='heuristic', columns='plan_count', values='plan_count')
    # df_exec_and_config = df_filtered.pivot(index='heuristic', columns='exec_and_config', values='exec_and_config')
    # df_performance_s = df_filtered.pivot(index='heuristic', columns='performance_s', values='performance_s')
    #
    # ax = df_exec_time.plot(kind='bar', stacked=True)
    # df_config_time.plot(kind='bar', stacked=True, ax=ax)
    # df_plan_count.plot(kind='bar', stacked=True, ax=ax)
    # df_exec_and_config.plot(kind='bar', stacked=True, ax=ax)
    # df_performance_s.plot(kind='bar', stacked=True, ax=ax)
    #
    # plt.show()

    df = pd.read_csv(input_stats_filename, dtype={'heuristic': 'category'})
    df_filtered = df[df['aggregation_func'] == 'Avg']

    df_pivot = df_filtered.pivot(index='heuristic', columns='exec_time',
                                 values=['exec_time', 'config_time', 'plan_count', 'exec_and_config', 'performance_s'])
    df_pivot.plot(kind='bar', stacked=True)

    plt.show()

#     Close enough for now!


# 1: stat file 2: extra options
if __name__ == '__main__':
    main(sys.argv[1:])