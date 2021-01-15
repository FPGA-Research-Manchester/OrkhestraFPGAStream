# DBMStoDSPI - Accelerated database operations on an FPGA with a dynamic stream processing interface

This software stack is meant to be transparently integrated with a proven DBMS for increased performance. The software stack would accelerate specific query operations with an available FPGA. How the data flows through the FPGA stack is shown in the graph below:

<center><img src="./docs/DBMStoDSPI_graph.svg" width=80%></center>

As you can see from the image the input would be a query plan graph form a DBMS to the software stack which will use [FOS](https://github.com/FPGA-Research-Manchester/fos) and the modules described below to accelerate the query.

## Currently supported data types:

- 32 bit integer
- Fixed length string
- SQL Date
- SQL Decimal(15,2)

## Currently supported modules:

- [Filter](https://docs.google.com/document/d/1aYy9Etr1Ixwe3E7jI4mP0RaP6_JUYoRPTvVph6-HNGM/view)
- [Join](https://docs.google.com/document/d/1r0RVhj606VpfFN-_qD-pGoYDA3DNoBDvr2w0ZUhhkLU/view)
- [Linear Sort](https://docs.google.com/document/d/1rDDLILdMLcuyK8YAFJgvJH6Eq8vFAtm5XBoBC7NX44E/view)
- [Mege Sort](https://docs.google.com/document/d/1PdNX-QX6q9c99VxUFVUgqroxa9sadZm0mU3asQ_sdqQ/view)
- [DMA](https://docs.google.com/document/d/1cxJLcjkrTCrByOmtiYwsu4Ptbp78npZbajhQ878ixp0/view)

The *DMA* is a compulsory module which will set up the interface.
The *sort* modules are meant to be used together.
The *join* module is for sort-merge join operations. So it'l only join sorted streams.

The interface specification documentation will be available later.