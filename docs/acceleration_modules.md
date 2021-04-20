# Modules for DBMS acceleration

Here we will discuss what type of operations there are in SQL queries and what type of hardware is needed. Below the different characteristics of the modules are highlighted

## Stream count

[]()  |1 input|1 output|2 inputs|2 outputs|N inputs|N outputs
:--|:-:|:-:|:-:|:-:|:-:|:-:
**Filter**|✔️|❌|❌|✔️|❌|❌
**Join**|❌|✔️|✔️|❌|❌|❌
**Linear Sort**|✔️|✔️|❌|❌|❌|❌
**Merge Sort**|✔️|✔️|❌|❌|❌|❌
**Addition**|✔️|✔️|❌|❌|❌|❌
**Multiplication**|❌|❌|❌|❌|✔️|✔️
**Global Sum**|✔️|✔️|❌|❌|❌|❌

The filter can also output a single stream. Either true or false one.
The global sum can also completely destroy the stream and output nothing.

A simplified classification here could be 1) **single input, single output** and 2) **two inputs, one output** modules. Or this can be extended to all 4 cases covered by the current modules.

## Data columns

[]()  |no change|change columns|first column important|strict column placement
:--|:-:|:-:|:-:|:-:
**Filter**|✔️|❌|❌|❌
**Join**|❌|✔️|✔️|❌
**Linear Sort**|✔️|❌|✔️|❌
**Merge Sort**|✔️|❌|✔️|❌
**Addition**|✔️|❌|❌|✔️
**Multiplication**|✔️|❌|❌|✔️
**Global Sum**|✔️|❌|❌|✔️

Strict column placement means that the data positioning has some constraints. For example, a 64 bit value has to be in positions 0 and 1 instead of 1 and 2. The filtering module can also be changed to work with 64 bit data and therefore the data positioning would have also the same constraints as the arithmetic modules.
Here more classifications are possible:

- **64 bit** modules
- **First column** modules
- **Crossbar** modules

And the filter can have no additional classification showing the default behaviour in terms of how the data should be placed. Everything is configurable and the data positioning will stay the same after the module.

## Reduce/increase record count

[]()  |increase|reduce
:--|:-:|:-:
**Filter**|❌|✔️
**Join**|✔️|✔️
**Linear Sort**|❌|❌
**Merge Sort**|❌|❌
**Addition**|❌|❌
**Multiplication**|❌|❌
**Global Sum**|❌|❌

This classification is self explanatory. There are module which can increase the size of a stream and reduce the size of a stream. But these stream size changes are data dependent. 

## Configuration data

[]()  |define stream IDs|define chunk IDs|define record size|start prefetching|bulk configuration memory|additional parameters
:--|:-:|:-:|:-:|:-:|:-:|:-:
**Filter**|✔️|✔️|❌|❌|✔️|✔️
**Join**|✔️|❌|✔️|✔️|✔️|❌
**Linear Sort**|✔️|❌|✔️|✔️|❌|❌
**Merge Sort**|✔️|❌|✔️|✔️|❌|✔️
**Addition**|✔️|✔️|❌|❌|✔️|✔️
**Multiplication**|✔️|❌|❌|❌|✔️|✔️
**Global Sum**|✔️|✔️|❌|✔️|✔️|✔️

Record size means how many chunks are used per record. 

Prefetching modules are active modules which drive the data on the interface. Passive modules just operate on data as it passes through.

The bulk memory configuration space is not as large for the arithmetic modules as it is for the filter and join modules. The chunk IDs will get configured for multiplication with additional parameters since multiple chunk IDs can be used. Join module needs additional parameters as well to tell how the mass configuration should look like. 

For multiplication module the chunk IDs are used but not in the same way as configuring different operations with different IDs. For multiplication the chunk IDs are used to tell which chunks have multiplication operations. 

As a side note, currently the merge sort module can only sort streams with ID 0. So in, it doesn't need stream ID defined by the query. Only with different DMA modules it can operate on different streams. Other DMA modules haven't been tested yet.

Here the classification can be as follows:

- **additional parameters**
- **requires record size**
- **prefetching modules**

The chunk ID can be given with additional parameters and the bulk configuration is noteworthy in terms of generalising modules from the development view point.

## Resource elasticity

[]()  |different modules|multiple modules
:--|:-:|:-:
**Filter**|✔️|✔️
**Join**|❌|❌
**Linear Sort**|✔️|❌
**Merge Sort**|❌|✔️
**Addition**|❌|❌
**Multiplication**|❌|❌
**Global Sum**|❌|❌

The filter module can have different amounts of dnf clauses and comparison lanes. The linear sort can produce different length sorted sequences. More modules can be used to process larger streams. 

## Special characteristics

[]()  |read back|multi-channel|half-sorted input|sorted input
:--|:-:|:-:|:-:|:-:
**Filter**|❌|❌|❌|❌
**Join**|❌|❌|❌|✔️
**Linear Sort**|❌|❌|❌|❌
**Merge Sort**|❌|✔️|✔️|❌
**Addition**|❌|❌|❌|❌
**Multiplication**|❌|❌|❌|❌
**Global Sum**|✔️|❌|❌|❌

These special characteristics also make self-explanatory categories. The sorted and half-sorted input only requires the first data element (column) to be in a certain order.

## DMA

The DMA module is also an acceleration module like all the other modules in the list which is configured exactly the same. The difference is that this module must be always used and it must be on the first position.

## All of the classifications

To sum all of this information the following abstract module ideas are present which different modules implement:

[]()  |Filter|Join|Linear Sort|Merge Sort|Addition|Multiplication|Global Sum
:--|:-:|:-:|:-:|:-:|:-:|:-:|:-:
**1 1-in-1-out**|✔️|❌|✔️|✔️|✔️|✔️|✔️
**2 2-in-1-out**|❌|✔️|❌|❌|❌|❌|❌
**3 64 bit**|❌|❌|❌|❌|✔️|✔️|✔️
**4 First column**|❌|✔️|✔️|✔️|❌|❌|❌
**5 Crossbar**|❌|✔️|❌|❌|❌|❌|❌
**6 Increase data**|❌|✔️|❌|❌|❌|❌|❌
**7 Reduce data**|✔️|✔️|❌|❌|❌|❌|❌
**8 Extra params**|✔️|✔️|❌|✔️|✔️|✔️|✔️
**9 Size info**|❌|✔️|✔️|✔️|❌|❌|❌
**10 Prefetch**|❌|✔️|✔️|✔️|❌|❌|✔️
**11 Different modules**|✔️|❌|✔️|❌|❌|❌|❌
**12 Multiple modules**|✔️|❌|❌|✔️|❌|❌|❌
**13 Read-back**|❌|❌|❌|❌|❌|❌|✔️
**14 Multi-channel**|❌|❌|❌|✔️|❌|❌|❌
**15 Partially sorted**|❌|❌|❌|✔️|❌|❌|❌
**16 Fully sorted**|❌|✔️|❌|❌|❌|❌|❌

Module classifications 1, 2 are important for assigning stream IDs.

Module classifications 3, 4, 5 are important for crossbar configuration.

Module classifications 6, 7 are important for memory allocation.

Module classifications 8, 9 are imprtant for query node defining. In the case of size info sometimes this data has to come from the query node or the scheduler which depends on the previous nodes.

Module classifications 9, 10 are important for setting up modules. And 13 is important for reading the result from the module. This needs additional setup as well.

Module classifications 11, 12 are important for resource elastic aware scheduling.

Module classifications 14, 15, 16 are important for the scheduler to consider. Multi-channel modules have to be first modules in the pipeline and sorting requirements are important for query node ordering.

[Back to the main page](./README.md)