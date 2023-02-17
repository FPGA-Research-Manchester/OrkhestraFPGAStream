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
import matplotlib
from matplotlib.ticker import FuncFormatter


def main(argv):
    if len(argv) != 2:
        print(argv)
        raise ValueError("Wrong amount of args given!")

    input_stats_filename = argv[0]
    graph_type = int(argv[1])

    # 0 just print
    # 1 bar chart
    # 2 Scatter graph
    # 3 Scatter graph with lines

    if graph_type == 0:
        with open(input_stats_filename, 'r') as file:
            reader = csv.reader(file)
            for row in reader:
                print(row)
    elif graph_type == 1:
        show_bar_chart_heuristics(input_stats_filename)
    elif graph_type == 2:
        show_scatter_graph(input_stats_filename)
    elif graph_type == 3:
        show_scatter_graph_with_line(input_stats_filename)
    else:
        raise ValueError("Wrong graph type given: " + str(graph_type))


def plot_column(ax, column_name, df_filtered, markers):
    for i,heuristic in enumerate(df_filtered.index.unique()):
        df_heuristic = df_filtered[df_filtered.index == heuristic]
        sc = ax.scatter(df_heuristic['time_limit'], df_heuristic[column_name], label=heuristic, marker=markers[i % len(markers)])
        ax.plot(df_heuristic['time_limit'], df_heuristic[column_name], label='_nolegend_', color=sc.get_facecolors()[0], alpha=0.7)
    ax.set_ylabel(column_name)
    ax.set_xlabel('Time Limit (s)')
    ax.set_axisbelow(True)
    ax.grid(True, linestyle='--', zorder=1)


def show_scatter_graph_with_line(input_stats_filename):
    df = pd.read_csv(input_stats_filename, dtype={'heuristic': 'category'})
    df_filtered = df[df['aggregation_func'] == 'Avg']
    df_filtered = df_filtered.drop(["Split: ", "aggregation_func"], axis=1)
    df_filtered = df_filtered.set_index("heuristic")
    columns = df_filtered.columns.tolist()
    columns.remove('time_limit')

    markers = ['o', 'v', '*', '+', 'x', 'D', 's', 'p', '^', '.']
    fig, axs = plt.subplots(len(columns), sharex=True, figsize=(10, 30))
    axs[-1], axs[-2] = axs[-2], axs[-1]
    for i, ax in enumerate(axs):
        plot_column(ax, columns[i], df_filtered, markers)
        ax.get_yaxis().get_major_formatter().set_scientific(True)
        ax.get_yaxis().get_major_formatter().set_useMathText(True)
        ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 1))

    plt.xlabel('Time Limit (s)')
    axs[-1].legend(bbox_to_anchor=(1.02, 2.2), loc='upper left', borderaxespad=0)
    axs[0].set_ylabel("Configuration time (s)")
    axs[1].set_ylabel("Streaming time (s)")
    axs[2].set_ylabel("Config + Exec time (s)")
    axs[3].set_ylabel("Plans considered")
    fig.suptitle("Schedulers Quality with Time Limits")
    plt.show()


def show_scatter_graph(input_stats_filename):
    # Load the csv file into a pandas dataframe
    df = pd.read_csv(input_stats_filename)

    columns_to_keep = ["final_query_count", "global_node_count", "merge_join_global_count", "table_global_count", "table_size_mean", "performance_s"]
    rename_dict = {"final_query_count":"Parallel Query Count", "global_node_count":"Operation Count", "merge_join_global_count":"Merge Join Operation Count", "table_global_count":"Input Table Count", "table_size_mean":"Average Table Row Count"}
    df = df[columns_to_keep]

    # Select the columns you want to plot
    #cols = [col for col in df.columns if col != "performance_s"]

    # Create a subplot for each column
    fig, axs = plt.subplots(nrows=int(len(columns_to_keep)/3), ncols=int(len(columns_to_keep)/2), figsize=(15, 25))
    axs = axs.flatten()
    fig.subplots_adjust(hspace=0.2, wspace=0.22)
    plt.subplots_adjust(left=0.05, right=0.95, bottom=0.1, top=0.9)

    # Plot each column against "performance_s"
    for i, col in enumerate(columns_to_keep):
        axs[i].set_axisbelow(True)
        axs[i].grid(True, linestyle='--')
        if col == "performance_s":
            axs[i].hist(df[col], bins=20, alpha=0.5)
            axs[i].set_xlabel("Scheduling performance (sec)")
            axs[i].set_ylabel("Frequency")
            # axs[i].set_title(col)
        else:
            axs[i].scatter(df[col], df["performance_s"], marker='o', alpha=0.2)
            axs[i].set_xlabel(rename_dict[col])
            # if i == 2:
            #     axs[i].set_ylabel("Scheduling performance (sec)")
            axs[i].set_ylabel("Scheduling performance (sec)")
            # axs[i].set_title(col)

    # Show the plot
    fig.suptitle("Scheduler's Runtime with Complex Requests")
    # plt.tight_layout()
    plt.show()


def show_bar_chart_heuristics(input_stats_filename):
    df = pd.read_csv(input_stats_filename, dtype={'heuristic': 'category'})

    # Filter for rows with aggregation function equal to 'Avg'
    df_filtered = df[df['aggregation_func'] == 'Avg']

    # Drop unnecessary columns
    df_filtered = df_filtered.drop(["Split: ", "time_limit", "aggregation_func"], axis=1)
    df_filtered = df_filtered[['heuristic', 'plan_count', 'performance_s', 'exec_time', 'config_time', 'exec_and_config']]

    # Set the 'heuristic' column as the index
    df_filtered = df_filtered.set_index("heuristic")

    # Normalize values by H0 values
    df_filtered = df_filtered.div(df_filtered.loc["H0"])

    # Plot the normalized values as a clustered bar chart
    colors = ["red", "blue", "green", "purple", "orange"]
    colors = [matplotlib.colors.to_rgba(color, alpha=0.5) for color in colors]

    fig, ax = plt.subplots(figsize=(12, 6), subplotpars=matplotlib.figure.SubplotParams(left=0.07, right=0.79))
    df_filtered.plot(kind="bar", ax=ax, width=0.8, color=colors, edgecolor="black")
    # ax = df_filtered.plot(kind="bar", figsize=(15, 6), width=0.8, color=colors, edgecolor="black")
    ax.set_axisbelow(True)
    ax.grid(True, linestyle='--')
    ax.set_ylim(0, 1.8)

    hatches = ["//", "++", "xx", "--", "oo"]

    for i, bar in enumerate(ax.containers):
        for j, rect in enumerate(bar):
            rect.set_hatch(hatches[i % 5])
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width() / 2, height + 0.05, "{:.4f}".format(height), ha='center',
                    va='bottom',
                    rotation=90, fontweight='bold')

    # Add title, axis labels, and legend
    # plt.title("Normalized Clustered Bar Chart")
    plt.xlabel("Heuristics")
    plt.ylabel("Compared Against H0 - The Best Plan")
    plt.legend(title="Legend", loc='center left', bbox_to_anchor=(1.05, 0.5),
               labels=["Plan Count", "Scheduling Time", "Exec Time", "Config Time", "Exec + Config Time"])
    # plt.tight_layout()
    # fig = plt.gcf()
    fig.suptitle("Scheduler's Performance with Heuristics")
    plt.show()

    # 1: stat file 2: extra options
if __name__ == '__main__':
    main(sys.argv[1:])