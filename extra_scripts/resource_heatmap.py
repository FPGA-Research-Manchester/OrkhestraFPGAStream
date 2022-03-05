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

import sys
import statistics
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM"
hw_library = {
    "Filter": {
        "bitstreams": {
            "filter81.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (8, 1), "string": "MMDM",
                             "is_backwards": False},
            "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMDBMMD",
                             "is_backwards": False},
            "filter162.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (16, 2), "string": "MMDMD",
                              "is_backwards": False},
            "filter164.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (16, 4), "string": "MMDMDBM",
                              "is_backwards": False},
            "filter321.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (32, 1), "string": "MMDMD",
                              "is_backwards": False},
            "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMDBMMD",
                              "is_backwards": False},
            "filter324.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (32, 4), "string": "MMDMDBMMDB",
                              "is_backwards": False}},
        "start_locations": [['filter81.bit', 'filter162.bit', 'filter164.bit', 'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [], []],
    }, "Linear Sort": {
        "bitstreams": {
            "linear512.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (512,), "string": "MMDMDBMMDB",
                              "is_backwards": False},
            "linear1024.bit": {"locations": (6, 16), "length": 14, "capacity": (1024,), "string": "MMDBMMDMDBMMDB",
                               "is_backwards": False}},
        "start_locations": [['linear512.bit'], [], [], [], [], [], ['linear1024.bit'], [], [], [],
                            ['linear512.bit'], [],
                            [], [], [], [], ['linear1024.bit'], [], [], [], [
                                'linear512.bit'], [], [], [], [], [],
                            [],
                            [], [], [], []],
    }, "Merge Sort": {
        "bitstreams": {
            "mergesort32.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (32,), "string": "MMDMDBM",
                                "is_backwards": False},
            "mergesort64.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (64,), "string": "MMDMDBMMDB",
                                "is_backwards": False},
            "mergesort128.bit": {"locations": (8, 18), "length": 12, "capacity": (128,), "string": "DBMMDMDBMMDB",
                                 "is_backwards": False}},
        "start_locations": [['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [],
                            ['mergesort128.bit'], [],
                            ['mergesort32.bit', 'mergesort64.bit'], [
        ], [], [], [], [], [], [],
            ['mergesort128.bit'], [],
            ['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Merge Join": {
        "bitstreams": {
            "join.bit": {"locations": (0, 10, 20), "length": 6, "capacity": (), "string": "MMDMDB",
                         "is_backwards": False}},
        "start_locations": [['join.bit'], [], [], [], [], [], [], [], [], [], ['join.bit'], [], [], [], [], [], [],
                            [],
                            [], [], ['join.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Addition": {
        "bitstreams": {
            "addition.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (), "string": "MMDM",
                             "is_backwards": False}},
        "start_locations": [['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], [], [],
                            [], [], [], [],
                            [], ['addition.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "Multiplier": {
        "bitstreams": {
            "multiplier.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (), "string": "DBMMDBM",
                               "is_backwards": False}},
        "start_locations": [[], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [],
                            ['multiplier.bit'], [], [],
                            [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], []],
    }, "Global Sum": {
        "bitstreams": {
            "globalsum.bit": {"locations": (2, 12, 22), "length": 3, "capacity": (), "string": "DMD",
                              "is_backwards": False}},
        "start_locations": [[], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [],
                            [], [], [], [], [],
                            [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], []],
    }
}
hw_library_small = {
    "Filter": {
        "bitstreams": {
            "filter81.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (8, 1), "string": "MMDM",
                             "is_backwards": False},
            "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMDBMMD",
                             "is_backwards": False},
            "filter162.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (16, 2), "string": "MMDMD",
                              "is_backwards": False},
            "filter164.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (16, 4), "string": "MMDMDBM",
                              "is_backwards": False},
            "filter321.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (32, 1), "string": "MMDMD",
                              "is_backwards": False},
            "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMDBMMD",
                              "is_backwards": False},
            "filter324.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (32, 4), "string": "MMDMDBMMDB",
                              "is_backwards": False}},
        "start_locations": [['filter81.bit', 'filter162.bit', 'filter164.bit', 'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [], []],
    }, "Linear Sort": {
        "bitstreams": {
            "linear512.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (512,), "string": "MMDMDBMMDB",
                              "is_backwards": False},
            "linear1024.bit": {"locations": (6, 16), "length": 14, "capacity": (1024,), "string": "MMDBMMDMDBMMDB",
                               "is_backwards": False}},
        "start_locations": [['linear512.bit'], [], [], [], [], [], ['linear1024.bit'], [], [], [],
                            ['linear512.bit'], [],
                            [], [], [], [], ['linear1024.bit'], [], [], [], [
                                'linear512.bit'], [], [], [], [], [],
                            [],
                            [], [], [], []],
    }, "Merge Sort": {
        "bitstreams": {
            "mergesort32.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (32,), "string": "MMDMDBM",
                                "is_backwards": False}},
        "start_locations": [['mergesort32.bit'], [], [], [], [], [], [], [],
                            [], [],
                            ['mergesort32.bit'], [
        ], [], [], [], [], [], [],
            [], [],
            ['mergesort32.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "mjoin": {
        "bitstreams": {
            "mjoin_0": {
                "string": "MMDMDB",
                "is_backwards": False,
                "length": 6,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_1": {
                "string": "MMDMDBMMDB",
                "is_backwards": False,
                "length": 10,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_2": {
                "string": "DBMMDMDBMMDB",
                "is_backwards": False,
                "length": 12,
                "capacity": (),
                "locations": [
                    8,
                    18
                ]
            }
        },
        "start_locations": [
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            []
        ]
    },
    # "Addition": {
    #     "bitstreams": {
    #         "addition.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (), "string": "MMDM",
    #                          "is_backwards": False}},
    #     "start_locations": [['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], [], [],
    #                         [], [], [], [],
    #                         [], ['addition.bit'], [], [], [], [], [], [], [], [], [], []],
    # }, "Multiplier": {
    #     "bitstreams": {
    #         "multiplier.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (), "string": "DBMMDBM",
    #                            "is_backwards": False}},
    #     "start_locations": [[], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [],
    #                         ['multiplier.bit'], [], [],
    #                         [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], []],
    # }, "Global Sum": {
    #     "bitstreams": {
    #         "globalsum.bit": {"locations": (2, 12, 22), "length": 3, "capacity": (), "string": "DMD",
    #                           "is_backwards": False}},
    #     "start_locations": [[], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [],
    #                         [], [], [], [], [],
    #                         [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], []],
    # }
}
hw_library_and_small_merge_sort = {
    "Filter": {
        "bitstreams": {
            "filter81.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (8, 1), "string": "MMDM",
                             "is_backwards": False},
            "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMDBMMD",
                             "is_backwards": False},
            "filter162.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (16, 2), "string": "MMDMD",
                              "is_backwards": False},
            "filter164.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (16, 4), "string": "MMDMDBM",
                              "is_backwards": False},
            "filter321.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (32, 1), "string": "MMDMD",
                              "is_backwards": False},
            "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMDBMMD",
                              "is_backwards": False},
            "filter324.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (32, 4), "string": "MMDMDBMMDB",
                              "is_backwards": False}},
        "start_locations": [['filter81.bit', 'filter162.bit', 'filter164.bit', 'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [], []],
    }, "Linear Sort": {
        "bitstreams": {
            "linear512.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (512,), "string": "MMDMDBMMDB",
                              "is_backwards": False},
            "linear1024.bit": {"locations": (6, 16), "length": 14, "capacity": (1024,), "string": "MMDBMMDMDBMMDB",
                               "is_backwards": False}},
        "start_locations": [['linear512.bit'], [], [], [], [], [], ['linear1024.bit'], [], [], [],
                            ['linear512.bit'], [],
                            [], [], [], [], ['linear1024.bit'], [], [], [], [
                                'linear512.bit'], [], [], [], [], [],
                            [],
                            [], [], [], []],
    }, "Merge Sort": {
        "bitstreams": {
            "mergesort32.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (32,), "string": "MMDMDBM",
                                "is_backwards": False},
            "mergesort64.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (64,), "string": "MMDMDBMMDB",
                                "is_backwards": False}},
        "start_locations": [['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [],
                            [], [],
                            ['mergesort32.bit', 'mergesort64.bit'], [
        ], [], [], [], [], [], [],
            [], [],
            ['mergesort32.bit', 'mergesort64.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "mjoin": {
        "bitstreams": {
            "mjoin_0": {
                "string": "MMDMDB",
                "is_backwards": False,
                "length": 6,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_1": {
                "string": "MMDMDBMMDB",
                "is_backwards": False,
                "length": 10,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_2": {
                "string": "DBMMDMDBMMDB",
                "is_backwards": False,
                "length": 12,
                "capacity": (),
                "locations": [
                    8,
                    18
                ]
            }
        },
        "start_locations": [
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            []
        ]
    },
    # "Addition": {
    #     "bitstreams": {
    #         "addition.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (), "string": "MMDM",
    #                          "is_backwards": False}},
    #     "start_locations": [['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], [], [],
    #                         [], [], [], [],
    #                         [], ['addition.bit'], [], [], [], [], [], [], [], [], [], []],
    # }, "Multiplier": {
    #     "bitstreams": {
    #         "multiplier.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (), "string": "DBMMDBM",
    #                            "is_backwards": False}},
    #     "start_locations": [[], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [],
    #                         ['multiplier.bit'], [], [],
    #                         [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], []],
    # }, "Global Sum": {
    #     "bitstreams": {
    #         "globalsum.bit": {"locations": (2, 12, 22), "length": 3, "capacity": (), "string": "DMD",
    #                           "is_backwards": False}},
    #     "start_locations": [[], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [],
    #                         [], [], [], [], [],
    #                         [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], []],
    # }
}
hw_library_and_big_merge_sort = {
    "Filter": {
        "bitstreams": {
            "filter81.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (8, 1), "string": "MMDM",
                             "is_backwards": False},
            "filter84.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (8, 4), "string": "DMDBMMD",
                             "is_backwards": False},
            "filter162.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (16, 2), "string": "MMDMD",
                              "is_backwards": False},
            "filter164.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (16, 4), "string": "MMDMDBM",
                              "is_backwards": False},
            "filter321.bit": {"locations": (0, 10, 20), "length": 5, "capacity": (32, 1), "string": "MMDMD",
                              "is_backwards": False},
            "filter322.bit": {"locations": (2, 12, 22), "length": 7, "capacity": (32, 2), "string": "DMDBMMD",
                              "is_backwards": False},
            "filter324.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (32, 4), "string": "MMDMDBMMDB",
                              "is_backwards": False}},
        "start_locations": [['filter81.bit', 'filter162.bit', 'filter164.bit', 'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [],
                            ['filter81.bit', 'filter162.bit', 'filter164.bit',
                             'filter321.bit', 'filter324.bit'],
                            [], ['filter84.bit', 'filter322.bit'], [], [], [], [], [], [], [], []],
    }, "Linear Sort": {
        "bitstreams": {
            "linear512.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (512,), "string": "MMDMDBMMDB",
                              "is_backwards": False},
            "linear1024.bit": {"locations": (6, 16), "length": 14, "capacity": (1024,), "string": "MMDBMMDMDBMMDB",
                               "is_backwards": False}},
        "start_locations": [['linear512.bit'], [], [], [], [], [], ['linear1024.bit'], [], [], [],
                            ['linear512.bit'], [],
                            [], [], [], [], ['linear1024.bit'], [], [], [], [
                                'linear512.bit'], [], [], [], [], [],
                            [],
                            [], [], [], []],
    }, "Merge Sort": {
        "bitstreams": {
            "mergesort32.bit": {"locations": (0, 10, 20), "length": 7, "capacity": (32,), "string": "MMDMDBM",
                                "is_backwards": False},
            "mergesort128.bit": {"locations": (8, 18), "length": 12, "capacity": (128,), "string": "DBMMDMDBMMDB",
                                 "is_backwards": False}},
        "start_locations": [['mergesort32.bit'], [], [], [], [], [], [], [],
                            ['mergesort128.bit'], [],
                            ['mergesort32.bit'], [
        ], [], [], [], [], [], [],
            ['mergesort128.bit'], [],
            ['mergesort32.bit'], [], [], [], [], [], [], [], [], [], []],
    }, "mjoin": {
        "bitstreams": {
            "mjoin_0": {
                "string": "MMDMDB",
                "is_backwards": False,
                "length": 6,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_1": {
                "string": "MMDMDBMMDB",
                "is_backwards": False,
                "length": 10,
                "capacity": (),
                "locations": [
                    0,
                    10,
                    20
                ]
            },
            "mjoin_2": {
                "string": "DBMMDMDBMMDB",
                "is_backwards": False,
                "length": 12,
                "capacity": (),
                "locations": [
                    8,
                    18
                ]
            }
        },
        "start_locations": [
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [
                "mjoin_2"
            ],
            [],
            [
                "mjoin_0",
                "mjoin_1"
            ],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            [],
            []
        ]
    },
    # "Addition": {
    #     "bitstreams": {
    #         "addition.bit": {"locations": (0, 10, 20), "length": 4, "capacity": (), "string": "MMDM",
    #                          "is_backwards": False}},
    #     "start_locations": [['addition.bit'], [], [], [], [], [], [], [], [], [], ['addition.bit'], [], [], [], [],
    #                         [], [], [], [],
    #                         [], ['addition.bit'], [], [], [], [], [], [], [], [], [], []],
    # }, "Multiplier": {
    #     "bitstreams": {
    #         "multiplier.bit": {"locations": (4, 14, 24), "length": 7, "capacity": (), "string": "DBMMDBM",
    #                            "is_backwards": False}},
    #     "start_locations": [[], [], [], [], ['multiplier.bit'], [], [], [], [], [], [], [], [], [],
    #                         ['multiplier.bit'], [], [],
    #                         [], [], [], [], [], [], [], ['multiplier.bit'], [], [], [], [], [], []],
    # }, "Global Sum": {
    #     "bitstreams": {
    #         "globalsum.bit": {"locations": (2, 12, 22), "length": 3, "capacity": (), "string": "DMD",
    #                           "is_backwards": False}},
    #     "start_locations": [[], [], ['globalsum.bit'], [], [], [], [], [], [], [], [], [], ['globalsum.bit'], [],
    #                         [], [], [], [], [],
    #                         [], [], [], ['globalsum.bit'], [], [], [], [], [], [], [], []],
    # }
}
just_big_merge_sort = {
    "Merge Sort": {
        "bitstreams": {
            "mergesort128.bit": {"locations": (8, 18), "length": 12, "capacity": (128,), "string": "DBMMDMDBMMDB",
                                 "is_backwards": False}},
        "start_locations": [[], [], [], [], [], [], [], [],
                            ['mergesort128.bit'], [],
                            [], [
        ], [], [], [], [], [], [],
            ['mergesort128.bit'], [],
            [], [], [], [], [], [], [], [], [], [], []],
    }
}

just_smaller_merge_sort = {
    "Merge Sort": {
        "bitstreams": {
            "mergesort64.bit": {"locations": (0, 10, 20), "length": 10, "capacity": (64,), "string": "MMDMDBMMDB",
                                "is_backwards": False}},
        "start_locations": [['mergesort64.bit'], [], [], [], [], [], [], [],
                            [], [],
                            ['mergesort64.bit'], [
        ], [], [], [], [], [], [],
            [], [],
            ['mergesort64.bit'], [], [], [], [], [], [], [], [], [], []],
    }
}


def get_global_start_location(hw_library, pr_region_length):
    global_start_locations = [[] for position in range(pr_region_length)]
    # Assuming all bitstream names are different.
    for operator in hw_library.values():
        for location in range(pr_region_length):
            for bitstream in operator["start_locations"][location]:
                global_start_locations[location].append(bitstream)
    return global_start_locations


def get_bitstream_operators(hw_library):
    all_bitstream_operators_map = dict()
    for operator in hw_library.keys():
        for bitstream in hw_library[operator]["bitstreams"].keys():
            all_bitstream_operators_map[bitstream] = operator
    return all_bitstream_operators_map


# How do I do a heatmap?
# Global & Per module: Count how many times the resource is covered.
# You can normalize between max and min hits to percentages.
#
def main(argv):
    sns.set_theme()
    sns.set(font_scale=2)
    set_of_heatmaps = []
    std_devs = []
    # create_heatmap(hw_library, set_of_heatmaps)
    create_heatmap(hw_library_small, set_of_heatmaps, std_devs)
    create_heatmap(just_smaller_merge_sort, set_of_heatmaps, std_devs)
    create_heatmap(hw_library_and_small_merge_sort, set_of_heatmaps, std_devs)
    create_heatmap(just_big_merge_sort, set_of_heatmaps, std_devs)
    create_heatmap(hw_library_and_big_merge_sort, set_of_heatmaps, std_devs)
    # data = np.asarray(global_resource_hit_counts).reshape(
    #     1, len(global_resource_hit_counts))
    pd_data = pd.DataFrame(set_of_heatmaps)
    pd_data.columns = [char for char in resource_string]
    pd_data.index = [
        # "Full",
        f"0)\nσ={std_devs[0]:.3f}",
        f"1)\nσ={std_devs[1]:.3f}",
        f"2)\nσ={std_devs[2]:.3f}",
        f"3)\nσ={std_devs[3]:.3f}",
        f"4)\nσ={std_devs[4]:.3f}"]
    # pd_data.index = [
    #     # "Full",
    #     f"Original\nσ={std_devs[0]:.4f}",
    #     f"Only smaller mergesort\nσ={std_devs[1]:.4f}",
    #     f"With smaller mergesort\nσ={std_devs[2]:.4f}",
    #     f"Only bigger mergesort\nσ={std_devs[3]:.4f}",
    #     f"With bigger mergesort\nσ={std_devs[4]:.4f}"]
    # sns.heatmap(pd_data, xticklabels=1, linewidths=.3,
    #             center=pd_data.loc["Original"][-1], cmap="viridis")
    sns.heatmap(
        pd_data,
        xticklabels=1,
        linewidths=.3,
        center=pd_data.loc[f"1)\nσ={std_devs[1]:.3f}"][3],
        cmap="Spectral_r").set(
        title='Heatmap of database operators` library')
    plt.xticks(rotation=30)
    plt.show()


def create_heatmap(target_hw_library, set_of_heatmaps, std_devs):
    global_start_locations = get_global_start_location(
        target_hw_library, len(resource_string))
    all_bitstream_operators_map = get_bitstream_operators(target_hw_library)
    global_resource_hit_counts = get_resource_hit_counts(global_start_locations, target_hw_library,
                                                         all_bitstream_operators_map, len(resource_string))
    global_resource_hit_counts = normalize_counts(global_resource_hit_counts)
    print("Mean:")
    print(f"{sum(global_resource_hit_counts) / len(global_resource_hit_counts):.3f}")
    print("Global heatmap:")
    print_heatmap(global_resource_hit_counts)
    set_of_heatmaps.append(global_resource_hit_counts)
    std_devs.append(statistics.stdev(global_resource_hit_counts))
    # Do the same for all other
    # for module in target_hw_library.keys():
    #     global_resource_hit_counts = get_resource_hit_counts(target_hw_library[module]["start_locations"], target_hw_library,
    #                                                          all_bitstream_operators_map, len(resource_string))
    #     global_resource_hit_counts = normalize_counts(
    #         global_resource_hit_counts)
    #     print(module + " heatmap:")
    #     print_heatmap(global_resource_hit_counts)


def get_resource_hit_counts(
        global_start_locations, hw_library, all_bitstream_operators_map, region_size):
    hit_counts = [0 for i in range(region_size)]
    for location in range(len(global_start_locations)):
        for bitstream in global_start_locations[location]:
            for hit_location in range(location, location +
                                      hw_library[all_bitstream_operators_map[bitstream]]["bitstreams"][
                                          bitstream]["length"]):
                hit_counts[hit_location] += 1
    return hit_counts


def normalize_counts(global_resource_hit_counts):
    return [float(i) / sum(global_resource_hit_counts)
            for i in global_resource_hit_counts]


def print_heatmap(global_resource_hit_counts):
    print(["{0:0.2f}".format(i) for i in global_resource_hit_counts])
    print(f"Std dev: {statistics.stdev(global_resource_hit_counts):.4f}")
    # For modules that have a high std dev (More than 1/3 MEAN) it is better to try and add more modules.
    # Then Find rare and can common combinations and make some HW library out of them
    # Then check the heatmap of them
    # Then if heatmap is bad increase it to see if heatmap improves
    # Then the last thing to check is with the density - Does more evenly
    # distributed density equal to better density?


if __name__ == '__main__':
    main(sys.argv[1:])
