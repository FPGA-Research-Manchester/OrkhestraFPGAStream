# Input data

Currently the input CSV data files look something like this:

[]()  |[]()  |[]() |[]() 
-|-|-|-
1|Mitsubishi|Galant|54708
2|Porsche|911|54289
3|Subaru|Forester|47063
4|Mercury|Sable|70813
5|GMC|Yukon XL 1500|43758
6|Suzuki|Kizashi|54328
7|GMC|Yukon XL 1500|93870
8|Mazda|MX-6|92677
9|Volkswagen|CC|56205
10|Toyota|Camry|46123

This is how the [CAR_DATA](../resources/data/CAR_DATA.csv) file starts. The separator character is defined in the config. The data types of each column are defined in the input definition file as described [here](./program_input.md). This input shows how much of the given data type is present in the table but the sizes of the data types can be configured in the [data_type_sizes.json](../resources/data_type_sizes.json) file. These two values are multiplied to find how many words are required to store the column of data.

Next the data types are described and how they are converted for the hardware can be seen from the [unit tests](../tests/types_converter_test.cpp).

## Decimal

The only decimal supported currently is 15,2. Which means that the total number of digits possible to store is 15 and that 2 of them are right of the decimal point. This value is stored as two 32 bit integers on the interface.

## Date

The date has to be supplied with hyphon "-" characters. These are removed and the resulting integer is stored.

## NULL

This is a data type where so to say meaningless or "grabage" data is needed or expected on the specified data column in all of the records.

## VarChar

Varchar elements can be surrounded by quotation marks in which those will be removed. These are needed if the string has a comma. 

## Integer

Integer values are stored directly. Currently only 32 bit integers are supported.


[Back to the main page](./README.md)