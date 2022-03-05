# Copyright 2022 University of Manchester
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

from turtle import width
import pandas as pd
import matplotlib.pyplot as plt
import sys
import seaborn as sns
from matplotlib import cm
import numpy as np


def main(argv):
    # Hardcoded for now
    argv = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"
    all_substrings = collect_all_substrings(argv)
    counts = pd.Series(all_substrings).value_counts()
    sorted_patterns = dict()
    for key in counts.keys():
        if counts[key] in sorted_patterns:
            sorted_patterns[counts[key]].append(key)
        else:
            sorted_patterns[counts[key]] = [key]
    for key in sorted_patterns.keys():
        sorted_patterns[key] = sorted(sorted_patterns[key], key=lambda x: len(x))
        print(f"{key}:{sorted_patterns[key]}")
    # plot = counts.plot.hist(
    #     bins=16,
    #     xticks=range(16))
    count_of_counts = pd.Series(counts).value_counts()
    color = cm.inferno_r(np.linspace(.2, .8, 10))
    plot = count_of_counts.plot(kind='bar', rot=0, width=0.9, color=color)

    # sns.barplot(data=count_of_counts, palette="Blues_d")

    plot.bar_label(plot.containers[0])

    sns.set_style("white")
    sns.despine()

    plt.title(
        "Histogram of substring occurence counts in a single PR region on a ZU9EG")
    plt.ylabel("Quantity of substrings with the specified count")
    plt.xlabel("Substring appearance count value")

    plt.show()


def collect_all_substrings(input_str):
    substrings = []
    for i in range(len(input_str)):
        current_string = ""
        for j in range(i, len(input_str)):
            current_string += input_str[j]
            substrings.append(current_string)
    return substrings


if __name__ == '__main__':
    main(sys.argv[1:])
