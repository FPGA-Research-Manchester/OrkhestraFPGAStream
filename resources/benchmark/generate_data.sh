#!/bin/bash
# https://github.com/databricks/tpch-dbgen
for i in 0.01 0.02 0.03 0.04 0.05 0.06 0.07 0.08 0.09 0.1 0.2 0.3; do
    ./dbgen -vf -s $i -T P
    ./dbgen -vf -s $i -T L
    for j in `ls *.tbl`; do cat $j > ${j/\.tbl/$i\.csv}; echo $j; done;
done
zip lineitem.zip lineitem*.csv
zip part.zip part*.csv