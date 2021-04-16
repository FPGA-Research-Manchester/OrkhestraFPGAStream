# Modules for DBMS acceleration

Here we will discuss what type of operations there are in SQL queries and what type of hardware is needed. Below the different characteristics of the modules are highlighted

## Stream count.

|1 input|1 output|2 inputs|2 outputs|N inputs|N outputs
:--|:-:|:-:|:-:|:-:|:-:|:-:
*Filter*|✔️|❌|❌|✔️|❌|❌
*Join*|❌|✔️|✔️|❌|❌|❌
*Linear Sort*|✔️|✔️|❌|❌|❌|❌
*Merge Sort*|✔️|✔️|❌|❌|❌|❌
*Addition*|✔️|✔️|❌|❌|❌|❌
*Multiplication*|❌|❌|❌|❌|✔️|✔️
*Global Sum*|✔️|✔️|❌|❌|❌|❌

The filter can also output a single stream.
The global sum can also completely destroy the stream and output nothing.

## Data columns

|no change|change columns|first column important|strict column placement
:--|:-:|:-:|:-:|:-:
*Filter*|✔️|❌|❌|❌
*Join*|❌|✔️|✔️|❌
*Linear Sort*|✔️|❌|✔️|❌
*Merge Sort*|✔️|❌|✔️|❌
*Addition*|✔️|❌|❌|✔️
*Multiplication*|✔️|❌|❌|✔️
*Global Sum*|✔️|❌|❌|✔️

## Reduce/increase record count

## Configuration data

## Resource elasticity

## Multi-Channel

## Read back

##DMA

The DMA module is also an acceleration module like all the other modules in the list which is configured exactly the same. The difference is that this module must be always used and it must be on the first position.

[Back to the main page](./README.md)