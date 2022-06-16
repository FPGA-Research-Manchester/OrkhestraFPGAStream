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
from os.path import exists
import os
import subprocess

def GetDataType(datatype_string):
    size_params = datatype_string.split("(")
    datatype_name = size_params[0]
    data_size = 1
    if (len(size_params) != 1):
        data_size = size_params[1][:-1]
    datatype_dict = {"bigint":"integer", "integer":"integer", "numeric":"decimal", "character":"varchar", "date":"date", "character varying":"varchar"}
    return (datatype_dict[datatype_name], data_size)

def main(argv):
    command = f"echo '\d {argv[0]}' | psql tpch_001"
    output_filename = f"{argv[0]}.txt"
    os.system(f"{command} > {output_filename}")

    #csv_filename = argv[0].split('.')[0] + ".csv"
    csv_filename = argv[0] + ".csv"
    if (not exists(csv_filename)):
        pass
        #raise RuntimeError(f"{csv_filename} doesn't exist!")

    with open(output_filename) as f:
        lines = f.readlines()[3:]

    print("columns =")
    for line in lines:
        values = line.split("|")[:2]
        if (len(values)>1):
            data_type,data_size = GetDataType(values[1].strip())
            print(f"{data_type}; {data_size}; {values[0].strip().upper()}")

    column_count_command = f"echo 'SELECT count(*) FROM {argv[0]}' | psql tpch_001"
    count_output = subprocess.check_output(column_count_command, shell=True, text=True)
    count_lines = count_output.split("\n")
    row_count = count_lines[2].strip()

    print(f"RegisterTable({csv_filename}, columns, {row_count})")

if __name__ == '__main__':
    main(sys.argv[1:])
