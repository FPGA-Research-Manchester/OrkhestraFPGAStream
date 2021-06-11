# DBMStoDSPI - Accelerated database operations on an FPGA with a dynamic stream processing interface

This software stack is meant to be transparently integrated with a proven DBMS for increased performance. The software stack would accelerate specific query operations with an available FPGA. How the data flows through the FPGA stack is shown in the graph below:

![DBMStoDSPI source layout](./docs/DBMStoDSPI_graph.svg)

As you can see from the image the input would be a query plan graph form a DBMS to the software stack which will use [FOS](https://github.com/FPGA-Research-Manchester/fos) and the modules described below to accelerate the query. The image is slightly out of date as the gray operation parameters are now used and there is an additional class for checking resource elastic module constraints.

## Currently supported data types:

- 32 bit integer
- Fixed length string
- SQL Date
- SQL Decimal(15,2)
- NULL

## Currently supported modules:

- [Filter](https://docs.google.com/document/d/1aYy9Etr1Ixwe3E7jI4mP0RaP6_JUYoRPTvVph6-HNGM/view)
- [Join](https://docs.google.com/document/d/1r0RVhj606VpfFN-_qD-pGoYDA3DNoBDvr2w0ZUhhkLU/view)
- [Linear Sort](https://docs.google.com/document/d/1rDDLILdMLcuyK8YAFJgvJH6Eq8vFAtm5XBoBC7NX44E/view)
- [Merge Sort](https://docs.google.com/document/d/1PdNX-QX6q9c99VxUFVUgqroxa9sadZm0mU3asQ_sdqQ/view)
- [Addition](https://docs.google.com/document/d/1z2pN-B5mMqBWMHZfsHWlNOEh4y0oQSoJJaQwMiRgKD8/view)
- [Multiplication](https://docs.google.com/document/d/13FvDpvQOcqsJmrKadfZ0wl7sOHxVujxV2PWCQ0EDBH0/view)
- [Global Aggregation Sum](https://docs.google.com/document/d/17INhz4SAK0X97FyJYAxEcWI2qQLcixy8q_eU_KDbDEw/view)
- [*DMA*](https://docs.google.com/document/d/1cxJLcjkrTCrByOmtiYwsu4Ptbp78npZbajhQ878ixp0/view)

The **DMA** is a compulsory module which will set up the interface.
The **sort** modules are meant to be used together.
The **join** module is for sort-merge join operations. So it will only join sorted streams.
The **addition**, **multiplication** and **sum** modules work with 64 bit decimal values.

The interface specification documentation will be available later.

## How does it work?

To run the program use the following command:

```
dbmstodspi -i INPUT_DEF.json -c CONFIG.ini
```

The first file defines the query plan. The second file contains json filepaths to define which bitstreams are available with their respective constraints. Examples can be found here:

* [INPUT_DEF.json](./resources/input_defs/TPCH_Q19_SF01.json)
* [CONFIG.ini](./resources/config.ini)

More info on the how the data is mapped to the interface and how the data is processed can be found [here](./docs/README.md).

## How to use the source?

The documentation for this project is created using [CMake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) with [Doxygen](https://www.doxygen.nl/manual/starting.html). To turn the documentation build option on you need to change the main [CMakeLists.txt](./CMakeLists.txt) file and then you can find the output in the build directory under the *doc_doxygen* folder. The FOS library and test library aren't included in the generated documentation.

Formatting is done according to the [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) options you can find [here](./.clang-tidy).

Testing for this code is done using [googletest](https://github.com/google/googletest) and is automatically downloaded when the code is built with testing options turned on.