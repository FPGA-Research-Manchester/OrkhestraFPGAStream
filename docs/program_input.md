# Program input

This page describes how to use the software and what kind of input is expected. The types of input needed are:

- Query plan
  - Define query nodes
- Input table data
  - Comma separated CSV files containing exported tables from the DBMS
- Data types configuration
  - Define supported data types and their sizes
- Module library
  - Define supporting bitstreams
- Bitstream files
  - Files containing module bitstreams
- Bitstream repository
  - JSON file defining available bitstreams for FOS

These are given currently in the two inputs required to run the program:

```
dbmstodspi -i INPUT_DEF.json -c CONFIG.ini
```

The config.ini also includes an additional file for defining bitstream memory mapped registers memory space and a data separator character for reading CSV files. This page documentation page is divided into 2 parts.

1. Query plan
2. Module library

The rest of the inputs can be seen from the examples given in the main README file.

## Query nodes json

Briefly the query plan nodes have to contain the following information:

- Input and expected tables
- Query operation type
- Previous and next query nodes
- Input and output crossbar configuration
- Query operation parameters

To simplify it even further just the following in some form is needed: 

- Data specification (input/output record size, memory usage)
- Operation and its parameters (same operation can be used in different ways)
- Global information (what operation comes next or before)

Currently the input defining query plan JSON looks like the following:

```JSON
{
  "filter_node": {
    "input": [ "CAR_DATA.csv" ],
    "output": [ null ],
    "operation": "kFilter",
    "previous_nodes": [ null ],
    "next_nodes": [ "join_node" ],
    "operation_parameters": {
      "input_stream_params": [
        [],
        [ 0, 1, 1, 0 ],
        [ 1, 32, 32, 1 ],
        []
      ],
      "output_stream_params": [
        [],
        [ 0, 1, 1, 0 ],
        [ 1, 32, 32, 1 ],
        [ 2 ]
      ],
      "operation_params": [
        [ 1, 14, 1 ],
        [ 0 ],
        [ 12000 ],
        [ 1 ],
        [ 0 ]
      ]
    }
  },
  "join_node": {
    "input": [ null, "CUSTOMER_DATA.csv" ],
    "output": [ "FILTERED_JOIN_DATA.csv" ],
    "operation": "kJoin",
    "previous_nodes": [ "filter_node", null ],
    "next_nodes": [ null ],
    "operation_parameters": {
      "input_stream_params": [
        [],
        [ 0, 1, 1, 0 ],
        [ 1, 32, 32, 1 ],
        [],
        [ 0, -1, 1, 2, 3, 4, 5, 6 ],
        [ 0, 1 ],
        [ 1, 24 ],
        []
      ],
      "output_stream_params": [
        [],
        [ 0, 1, 1, 0, 1 ],
        [ 1, 32, 32, 1, 24 ],
        [ 2 ]
      ],
      "operation_params": [
        [ 2 ]
      ]
    }
  }
}
```

Let's go through the different fields one at a time. 

### Input data files

These "*input*" & "*output*" fields take exported DBMS table data files. They are CSV files using a separator defined in the config file. More details about these files can be found in the [input data configuration documentation](./input_data_configuration.md).

If the "*output*" field has values, then the node can't be pipelined with any following nodes as the results have to be written to memory and compared with the given golden output data. On the other hand, if the "*output*" is NULL, then one of the following can happen:

1. The output data pointer is given to the dependent runs if enough memory is available to keep the data.
2. The data is written to a file with the node name if there isn't enough DDR memory or no following query nodes.
3. The data only needs to be forwarded within a run to the pipelined modules, and nothing gets written to the memory.

In addition, the pipelined nodes can't have an "*input*" defined either if it is directly coming from another node. 

### Supported operations

The next field, "*operation*", is used to make the scheduler choose the correct bitstream to execute the query node. Currently, its value can only be an operation type enum value. The supported operations are described in the [acceleration modules documentation](./acceleration_modules.md). 

<!--In the future this field should be extended to further include operation parameters to make the scheduler choose the correct bitstream since there will be multiple bitstreams containing different modules accelerating the same operation due to resource elastic module implementations. These requirements like into how long sequences the data is sorted should be noted here to select the correct **merge sorter** module combination.-->

### Pipelined query nodes

The "*previous_node*" & "*next_node*" fields will just contain pointers to linked query nodes. Since the interface can process independent and dependent query nodes in parallel, it is important to note which nodes have dependencies such that they get assigned the appropriate stream IDs. The stream IDs are used to configure the HW modules to only operate on the correct streams and pass the other ones through. The dependencies are also important for the scheduler to ensure that all of the required data is available for the next considered bitstream run.

In the future, one of the fields could be dropped, and the dropped link could be added inside the software after processing the input query plan.

### Operation params

Next up is the "operation_parameters" field. This one contains further fields which have two-dimensional vectors of integers as valid values. These help fill different parameters as required. The first two fields are "*input_stream_params*" & "*output_stream_params*". They have 4 vectors per input or output stream:

1. Projection
2. Data types
3. Data sizes
4. Chunk count

#### Projection
The first vector for both of these fields contain the crossbar configuration. This is described in the [crossbar documentation](./crossbar_configuration.md). In short this defines where each integer worth of data gets placed and where the data can be left undefined (-1). 

Different modules may change how the data looks on the wires. Because we can't know in advance how the scheduler places these modules into runs we can't define the projection operation for each query node as we don't currently have a crossbar module. Therefore a query node can't have a prerequisite module in the same run if the node has its input projection operation defined. And by the same logic, a node with an output projection operation can't have any modules pipelined after it in the same run.

#### Data types
Currently supported data types:

0. Integer
1. Varchar
2. NULL
3. Decimal(15,2)
4. Date

#### Data sizes
This is a vector containing multipliers for each of the data types defined in hte previous vector. If there are maximum 10 characters in a field then the previous vecotor should contain a 1 and this one a 10.

#### Chunk count
This vector is only required for the output streams to note how long is the output stream before any projection operations.

#### Operation parameters
Lastly, there are the actual operation parameters for different modules. How the parameters should look for each module is documented at the corresponding module setup classes. But brief overview is the following:

- **Filter**:
  - Each clause literal defined by operation and data location
- **Join**:
  - How much is the second stream shifted
- **Linear Sort**:
  - Nothing
- **Merge Sort**:
  - Max channel count 
  - Sorted sequence length
- **Addition**:
  - Chunk ID
  - Negation bitset
  - Literal value
- **Multiplication**:
  - N amount of vectors
    - Chunk ID
    - Position 
- **Global Sum**:
  - Chunk ID
  - Position

##### Filter
For each word of data which is checked for filtering there is a header vector containing 3 integers: chunk ID, position index and how many comparison operations are done on the word of data.
Then for each operation there are the following vectors of data:

1. Operation type
    0. 32 bit less than 
    1. 32 bit less than or equal
    2. 32 bit equal
    3. 32 bit greater than or equal
    4. 32 bit greater than
    5. 32 bit not equal
    8. 64 bit less than 
    9. 64 bit less than or equal
    10. 64 bit equal
    11. 64 bit greater than or equal
    12. 64 bit greater than
    13. 64 bit not equal
2. Comparison literal 
3. Clause literal type - If none are given it is automatically set to positive. If there are no operations on a word it defaults to don't care.
    0. Don't care
    1. Positive
    2. Negative
4. DNF clause IDs where the literal is included

### How to get query plan graph?

Currently the input is hand crafted and automation is being looked into in issue [#91](https://github.com/FPGA-Research-Manchester/DBMStoDSPI/issues/91).

But in general how do we get an SQL query to something like this [example](../resources/input_defs/TPCH_Q19_SF01.json)?
<details>
  <summary>TPC-H Q19 in SQL</summary>
  
  ```sql
  SELECT Sum(l_extendedprice * ( 1 - l_discount )) AS revenue
  FROM   lineitem,
         part
  WHERE  ( p_partkey = l_partkey
         AND p_brand = 'Brand#12'
         AND p_container IN ( 'SM CASE', 'SM BOX', 'SM PACK', 'SM PKG' )
         AND l_quantity >= 1
         AND l_quantity <= 1 + 10
         AND p_size BETWEEN 1 AND 5
         AND l_shipmode IN ( 'AIR', 'AIR REG' )
         AND l_shipinstruct = 'DELIVER IN PERSON' )
        OR ( p_partkey = l_partkey
             AND p_brand = 'Brand#23'
             AND p_container IN ( 'MED BAG', 'MED BOX', 'MED PKG', 'MED PACK' )
             AND l_quantity >= 10
             AND l_quantity <= 10 + 10
             AND p_size BETWEEN 1 AND 10
             AND l_shipmode IN ( 'AIR', 'AIR REG' )
             AND l_shipinstruct = 'DELIVER IN PERSON' )
        OR ( p_partkey = l_partkey
             AND p_brand = 'Brand#34'
             AND p_container IN ( 'LG CASE', 'LG BOX', 'LG PACK', 'LG PKG' )
             AND l_quantity >= 20
             AND l_quantity <= 20 + 10
             AND p_size BETWEEN 1 AND 15
             AND l_shipmode IN ( 'AIR', 'AIR REG' )
             AND l_shipinstruct = 'DELIVER IN PERSON' ); 
  ```
  
</details>

We have two options:

1. Use a DBMS EXPLAIN function
2. Use an parsing tools

#### EXPLAIN

Let's look what kind of response can we get from PostgreSQL EXPLAIN function:

<details>
  <summary>PostgreSQL EXPLAIN</summary>
  
  ```json
  [{
    "Plan": {
      "Node Type": "Aggregate",
      "Strategy": "Plain",
      "Partial Mode": "Simple",
      "Parallel Aware": false,
      "Startup Cost": 32259.63,
      "Total Cost": 32259.64,
      "Plan Rows": 1,
      "Plan Width": 32,
      "Output": [
        "sum((lineitem.l_extendedprice * ('1'::numeric - lineitem.l_discount)))"
      ],
      "Plans": [
        {
          "Node Type": "Merge Join",
          "Parent Relationship": "Outer",
          "Parallel Aware": false,
          "Join Type": "Inner",
          "Startup Cost": 32201.57,
          "Total Cost": 32259.55,
          "Plan Rows": 11,
          "Plan Width": 12,
          "Output": [
            "lineitem.l_extendedprice",
            "lineitem.l_discount"
          ],
          "Inner Unique": false,
          "Merge Cond": "(lineitem.l_partkey = part.p_partkey)",
          "Join Filter": "(((part.p_brand = 'Brand#12'::bpchar) AND (part.p_container = ANY ('{\"SM CASE\",\"SM BOX\",\"SM PACK\",\"SM PKG\"}'::bpchar[])) AND (lineitem.l_quantity >= '1'::numeric) AND (lineitem.l_quantity <= '11'::numeric) AND (part.p_size <= 5)) OR ((part.p_brand = 'Brand#23'::bpchar) AND (part.p_container = ANY ('{\"MED BAG\",\"MED BOX\",\"MED PKG\",\"MED PACK\"}'::bpchar[])) AND (lineitem.l_quantity >= '10'::numeric) AND (lineitem.l_quantity <= '20'::numeric) AND (part.p_size <= 10)) OR ((part.p_brand = 'Brand#34'::bpchar) AND (part.p_container = ANY ('{\"LG CASE\",\"LG BOX\",\"LG PACK\",\"LG PKG\"}'::bpchar[])) AND (lineitem.l_quantity >= '20'::numeric) AND (lineitem.l_quantity <= '30'::numeric) AND (part.p_size <= 15)))",
          "Plans": [
            {
              "Node Type": "Sort",
              "Parent Relationship": "Outer",
              "Parallel Aware": false,
              "Startup Cost": 30940.23,
              "Total Cost": 30968.35,
              "Plan Rows": 11251,
              "Plan Width": 25,
              "Output": [
                "lineitem.l_extendedprice",
                "lineitem.l_discount",
                "lineitem.l_partkey",
                "lineitem.l_quantity"
              ],
              "Sort Key": [
                "lineitem.l_partkey"
              ],
              "Plans": [
                {
                  "Node Type": "Seq Scan",
                  "Parent Relationship": "Outer",
                  "Parallel Aware": false,
                  "Relation Name": "lineitem",
                  "Schema": "public",
                  "Alias": "lineitem",
                  "Startup Cost": 0,
                  "Total Cost": 30183.16,
                  "Plan Rows": 11251,
                  "Plan Width": 25,
                  "Output": [
                    "lineitem.l_extendedprice",
                    "lineitem.l_discount",
                    "lineitem.l_partkey",
                    "lineitem.l_quantity"
                  ],
                  "Filter": "((lineitem.l_shipmode = ANY ('{AIR,\"AIR REG\"}'::bpchar[])) AND (lineitem.l_shipinstruct = 'DELIVER IN PERSON'::bpchar) AND (((lineitem.l_quantity >= '1'::numeric) AND (lineitem.l_quantity <= '11'::numeric)) OR ((lineitem.l_quantity >= '10'::numeric) AND (lineitem.l_quantity <= '20'::numeric)) OR ((lineitem.l_quantity >= '20'::numeric) AND (lineitem.l_quantity <= '30'::numeric))))"
                }
              ]
            },
            {
              "Node Type": "Sort",
              "Parent Relationship": "Inner",
              "Parallel Aware": false,
              "Startup Cost": 1261.34,
              "Total Cost": 1261.46,
              "Plan Rows": 48,
              "Plan Width": 30,
              "Output": [
                "part.p_partkey",
                "part.p_brand",
                "part.p_container",
                "part.p_size"
              ],
              "Sort Key": [
                "part.p_partkey"
              ],
              "Plans": [
                {
                  "Node Type": "Seq Scan",
                  "Parent Relationship": "Outer",
                  "Parallel Aware": false,
                  "Relation Name": "part",
                  "Schema": "public",
                  "Alias": "part",
                  "Startup Cost": 0,
                  "Total Cost": 1260,
                  "Plan Rows": 48,
                  "Plan Width": 30,
                  "Output": [
                    "part.p_partkey",
                    "part.p_brand",
                    "part.p_container",
                    "part.p_size"
                  ],
                  "Filter": "((part.p_size >= 1) AND (((part.p_brand = 'Brand#12'::bpchar) AND (part.p_container = ANY ('{\"SM CASE\",\"SM BOX\",\"SM PACK\",\"SM PKG\"}'::bpchar[])) AND (part.p_size <= 5)) OR ((part.p_brand = 'Brand#23'::bpchar) AND (part.p_container = ANY ('{\"MED BAG\",\"MED BOX\",\"MED PKG\",\"MED PACK\"}'::bpchar[])) AND (part.p_size <= 10)) OR ((part.p_brand = 'Brand#34'::bpchar) AND (part.p_container = ANY ('{\"LG CASE\",\"LG BOX\",\"LG PACK\",\"LG PKG\"}'::bpchar[])) AND (part.p_size <= 15))))"
                }
              ]
            }
          ]
        }
      ]
    }
  }]
  ```
  
</details>

We get the the main operations of filtering, sort, merge join, aggregation and the projection operations. The problem we have is that the filtering conditions are still in string format and so are the arithmetic operations used for aggregation. 

If we use a DBMS which has a more detailed output like MonetDB and SQLite then the result is something close to bytecode (examples given in the linked issue). Interpreting that can be done but that implementation will take a lot of time.

#### Parsing tools

Independent parsing tools can be used as well like mo-sql-parsing:

<details>
  <summary>mo-sql-parsing Q19 parsing</summary>
  
  ```json
  {
    "from": [
        "lineitem",
        "part"
    ],
    "select": {
        "name": "revenue",
        "value": {
            "sum": {
                "mul": [
                    "l_extendedprice",
                    {
                        "sub": [
                            1,
                            "l_discount"
                        ]
                    }
                ]
            }
        }
    },
    "where": {
        "or": [
            {
                "and": [
                    {
                        "eq": [
                            "p_partkey",
                            "l_partkey"
                        ]
                    },
                    {
                        "eq": [
                            "p_brand",
                            {
                                "literal": "Brand#12"
                            }
                        ]
                    },
                    {
                        "in": [
                            "p_container",
                            {
                                "literal": [
                                    "SM CASE",
                                    "SM BOX",
                                    "SM PACK",
                                    "SM PKG"
                                ]
                            }
                        ]
                    },
                    {
                        "gte": [
                            "l_quantity",
                            1
                        ]
                    },
                    {
                        "lte": [
                            "l_quantity",
                            {
                                "add": [
                                    1,
                                    10
                                ]
                            }
                        ]
                    },
                    {
                        "between": [
                            "p_size",
                            1,
                            5
                        ]
                    },
                    {
                        "in": [
                            "l_shipmode",
                            {
                                "literal": [
                                    "AIR",
                                    "AIR REG"
                                ]
                            }
                        ]
                    },
                    {
                        "eq": [
                            "l_shipinstruct",
                            {
                                "literal": "DELIVER IN PERSON"
                            }
                        ]
                    }
                ]
            },
            {
                "and": [
                    {
                        "eq": [
                            "p_partkey",
                            "l_partkey"
                        ]
                    },
                    {
                        "eq": [
                            "p_brand",
                            {
                                "literal": "Brand#23"
                            }
                        ]
                    },
                    {
                        "in": [
                            "p_container",
                            {
                                "literal": [
                                    "MED BAG",
                                    "MED BOX",
                                    "MED PKG",
                                    "MED PACK"
                                ]
                            }
                        ]
                    },
                    {
                        "gte": [
                            "l_quantity",
                            10
                        ]
                    },
                    {
                        "lte": [
                            "l_quantity",
                            {
                                "add": [
                                    10,
                                    10
                                ]
                            }
                        ]
                    },
                    {
                        "between": [
                            "p_size",
                            1,
                            10
                        ]
                    },
                    {
                        "in": [
                            "l_shipmode",
                            {
                                "literal": [
                                    "AIR",
                                    "AIR REG"
                                ]
                            }
                        ]
                    },
                    {
                        "eq": [
                            "l_shipinstruct",
                            {
                                "literal": "DELIVER IN PERSON"
                            }
                        ]
                    }
                ]
            },
            {
                "and": [
                    {
                        "eq": [
                            "p_partkey",
                            "l_partkey"
                        ]
                    },
                    {
                        "eq": [
                            "p_brand",
                            {
                                "literal": "Brand#34"
                            }
                        ]
                    },
                    {
                        "in": [
                            "p_container",
                            {
                                "literal": [
                                    "LG CASE",
                                    "LG BOX",
                                    "LG PACK",
                                    "LG PKG"
                                ]
                            }
                        ]
                    },
                    {
                        "gte": [
                            "l_quantity",
                            20
                        ]
                    },
                    {
                        "lte": [
                            "l_quantity",
                            {
                                "add": [
                                    20,
                                    10
                                ]
                            }
                        ]
                    },
                    {
                        "between": [
                            "p_size",
                            1,
                            15
                        ]
                    },
                    {
                        "in": [
                            "l_shipmode",
                            {
                                "literal": [
                                    "AIR",
                                    "AIR REG"
                                ]
                            }
                        ]
                    },
                    {
                        "eq": [
                            "l_shipinstruct",
                            {
                                "literal": "DELIVER IN PERSON"
                            }
                        ]
                    }
                ]
            }
        ]
    }
  }
  ```
  
</details>

Here we get the filtering conditions and arithmetics parsed which we were missing in the PostgreSQL EXPLAIN output. These two options could be used in unison the get closer to the required query plan definition. The filtering parsing can be minimised with python scripts like [this](https://gist.github.com/joocer/9e939b2bda90d56bf48b8ff78eeba0e7) or more established scripts like [Espresso](https://pyeda.readthedocs.io/en/latest/2llm.html). 

What's missing is another script to generate the stream parameters. The main responsiblity of this last additional piece of code would be to keep track of where the words of data position to generate the chunk and position indexes required to configure modules.

Ideally with a lot more development time SQLite integration could be possible where the machine code could be parsed as the rest of the DBMS is extendable and allows memory management which is another important requirement for production ready versions.

## Available bitstream library

Currently the bitstream library is hardcoded in the [query_scheduling_data.hpp](../src/query_managing/query_scheduling_data.hpp). The library is a map of module combinations mapped to a bitstream file name which can be found in a [repo.json](../resources/repo.json) file. The map of combinations to bitstream file should be given as a JSON input file. 

There is a problem though. Different bitstreams can have the same operation accelerators but with different parameters. In the future, these parameters should be included in the module combinations. For example, how many comparison lanes and DNF clauses the specific **filter** bitstream has or how long sequences does the **linear sort** produce should be included in the map. Then it is possible to differentiate between two combinations with different parameters.

### Hardware bin files

How to use the bitstream files described in the bitstream library described above is to generate them using Xilinx Bootgen tool (documentation available online) and load the bitstreams in using [FOS](https://github.com/FPGA-Research-Manchester/fos). 

#### How to generate bitstream files?

Here is a brief description on how to generate the bitsteam files:

##### 1. Create a custom module using Vitis and/or Vivado
##### 2. Generate bitstream .bit file

The first two steps are also described in the [FOS tutorials](https://github.com/FPGA-Research-Manchester/fos/blob/fdac37e188e217293d296d9973c22500c8a4367c/compilation_flow/hls/README.md)

##### 3. Convert bitstream to .bin file

First of all a .bif file is required. The contents of the .bif file have to be the following:

```
all:
{
  your_bitstream_name.bit
}
```

Then the .bif file can be given to the Bootgen tool with the following command:

```Shell
bootgen -image bitstream.bif -arch zynqmp -o bitstream.bin -w
```

##### 4. Add the new .bin to the bitstream repository

FOS uses this [repo.json](../resources/repo.json) file. The new bitstream name just has to get added to the list. You can see how it currently looks like below:

```JSON
{
  "accelerators": [
    "DSPI_joining",
    "DSPI_filtering",
    "DSPI_merge_sorting",
    "DSPI_double_merge_sorting",
    "DSPI_linear_sorting",
    "DSPI_filter_join",
    "DSPI_empty",
    "DSPI_sort_join_filter",
    "DSPI_filtering_linear_sort",
    "DSPI_addition",
    "DSPI_multiplication",
    "DSPI_aggregation_sum"
  ]
}
```

More on how the bitstream files are used can be found in the [memory documentation](./memory_allocation.md).

## Running the program on a remote FPGA

This deserves a whole separate documentation page but for now it needs to be mentioned that all of these input files will have to be available on the Zynq MPSoC chip which is also running the Linux system where the program will be executed. 

It is recommended to do development on a separate machine. Therefore the input files won't be on the same system. Currently for remote execution and debugging Visual Studio is used and other workflows haven't been tested. To copy the input files over to the remote system the build process has to be modified to not only copy the built source over but also the input files. This can be done with CMake in this [CMakeLists.txt file](../apps/CMaakeLists.txt).

A brief description about the rest of the required steps is on the main repo README page.

[Back to the main page](./README.md)