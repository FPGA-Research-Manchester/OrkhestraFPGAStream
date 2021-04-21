# Program input

This page describes how to use the software and what kind of input is expected. The types of input needed are:

- Query plan
  - JSON file with defined query nodes
- Input table data
  - Comma separated CSV files containing exported tables from the DBMS
- Data types configuration
  - INI file defining supported data types and their sizes
- Module library
  - JSON file defining supporting bitstreams
- Bitstream files
  - BIN & JSON files containing module bitstreams
- Bitstream repository
  - JSON file defining available bitstreams for FOS

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

Currently the input is hard-coded but in the near future the input will look like the following

```JSON
[{
	"first_filter_node": {
		"input": "first_input_table.csv",
		"output": "expected_filtered_table.csv",
		"operation": "Filtering",
		"previous_node": null,
		"next_node": "second_sum_node",
		"operation_parameters": {
			"input_stream_params": [
				[0, 1, 1, 2, 3]
			],
			"output_stream_params": [
				[0, 1, 3, 4],
				[1]
			],
			"operation_params": [
				[1]
			]
		}
	}
}, {
	"second_sum_node": {
		"input": "expected_filtered_table.csv",
		"output": "expected_filtered_table.csv",
		"operation": "Sum",
		"previous_node": "first_filter_node",
		"next_node": null,
		"operation_parameters": {
			"input_stream_params": [
				[3, 4]
			],
			"output_stream_params": [
				[0, 1],
				[1]
			],
			"operation_params": [
				[0],
				[0]
			]
		}
	}
}]
```

Let's go through the different fields one at a time. 

### Input data files

These "*input*" & "*output*" fields take exported DBMS table data files. They are CSV files using a comma separator. More details about these files can be found in the [input data configuration documentation](./input_data_configuration.md). These files are used to find out how large is a record and how many records should be streamed to the FPGA. 

In the future the "*output*" should be removed and simply replaced with the record size parameter added to the "operation_parameters" field. In addition, the pipelined nodes don't need an "*input*" defined either if it is directly coming from another node. These fields can be used for naming purposes still when the output has to be written to a file or the intermediate table has to be stored in the file system. But these names could also be automatically generated. 

### Supported operations

The next field "*operation*" is used to make the scheduler choose the correct bitstream to execute the query node. Currently its value can only be an operation type enum value. The supported operations are described in the [acceleration modules documentation](./acceleration_modules.md). 

In the future this field should be extended to further include operation parameters to make the scheduler choose the correct bitstream since there will be multiple bitstreams containing different modules accelerating the same operation due to resource elastic module implementations. These requirements like into how long sequences the data is sorted should be noted here to select the correct **merge sorter** module combination.

### Pipelined query nodes

The "*previous_node*" & "*next_node*" fields will just contain pointers to linked query nodes. Since the interface can process independent and dependent query nodes in parallel it is important to note which nodes have dependencies such that they get assigned the appropriate stream IDs. The stream IDs are used to configure the HW modules to only operate on the correct streams and pass the other ones through. The dependencies are also important for the scheduler to make sure that all of the required data is available for the next considered bitstream run.

In the future one of the fields could be dropped and the dropped link could be added inside the software after the input query plan processing.

### Crossbar configuration

Next up is the "operation_parameters" field. This one contains further fields which have two-dimensional vectors of integers as valid values. These help fill different parameters as required. The first two fields are "*input_stream_params*" & "*output_stream_params*". The first vector for both of these fields contain the crossbar configuration. This is described in the [crossbar documentation](./crossbar_configuration.md). In short this defines where each integer worth of data gets placed and where the data can be left undefined (-1). 

In the future, an additional constraint might be put on the crossbar configuration. We can't know in advance how the scheduler will place different nodes into HW runs. If multiple nodes get scheduled to run in the same run then the crossbars at the edges of the interface won't be used. Then the data will end up in different locations than expected. And as a result the module configuration will be incorrect.

There are multiple ways to solve this problem:

1. Make it possible for the scheduler to amend the operation parameters - Difficult to make it scalable and generalisable for future operations and modules
2. Have crossbar modules inserted between the modules to fix data positions - Resource expensive
3. Don't allow any crossbar shuffling operations at intermediate stages of a query - Only at the beginning of the first nodes and at the end of the last nodes there can be a projection operation. This means that unnecessary amount of data could be streamed to the FPGA if multiple smaller runs are used instead of one big run.

First option can be done if every operation fix is implemented individually into the scheduler. The problem is that this adds a lot more corner cases to the already non-uniform modules which make future scaling harder.

Second option can be done on larger FPGAs. But if the area of the FPGA can't be spared like for larger queries or smaller FPGAs then this would reduce the attractiveness of using this platform.

The third option is the easiest and therefore for now it will be implemented as well.

### Other operation params

The second vector in the "*output_stream_params*" field is for holding a single integer value. That integer tells how many chunks of data each record in the output stream has. This information is useful for configuring the output crossbar. This is required if a module changes the size of the stream like the join module does. 

In the future this value could be required for a different reason. For pipelined queries, if the previous query node is executed in the same run then there won't be an output and an input crossbar to change the data size to an expected one. Thus the record size value will be used for next node module configuration. Furthermore, this value doesn't need to come from the query plan as this can be synthesised based on the operation but for now it is given with the query plan.

Lastly, there are the actual operation parameters for different modules. How the parameters should look for each module is documented at the corresponding module setup classes. But brief overview is the following:

- **Filter**:
  - An integer choosing one of the hard coded configurations
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

```JSON
all:
{
  your_bitstream_name.bit
}
```

Then the .bif file can be given to the Bootgen tool with the following command:

```
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

[Back to the main page](./README.md)