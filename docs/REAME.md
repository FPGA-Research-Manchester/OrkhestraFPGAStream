# How do you configure the crossbars.

First things first, the interface is setup in a way where it makes sense to use 32bits as the smallest data size to describe the system. We mostly use 32bit integers so I'll be refering to integers mostly in the following description to make it easier to follow.

## Configuration data size

The crossbars are between the buffer and the interface. First, what does the data in the buffers look like? The data is pumped to the buffers using AXI bursts which in the current design can last up to 128 cycles where each cycle can transfer 4 integers of data (4 times 32bits). The data in the buffers is consecutive with no garbage data in between the records. 

### Example introduction

Now how do we configure the crossbar to move the data from the buffers to the interface wires? Let's go through a simple example. We have a table where each record is 32 integers large. You can see the columns and their sizes below:

ID - 1 | Sentence - 29 | Length - 1 | Rating - 1
------------ | ------------- | ------------- | -------------
1|Far far away, behind the word mountains, far from the countries Vokalia and Consonantia, there live the blind texts.|116|5
2|Boring sentence|15|1

Let's say we want to see the whole table in the original order on the interface as well as on Figure X. 

[Figure]

To do that we need to understand how much we need to configure. Each buffer can have 32 chunks of data in it where each chunk is 16 integers large. Thus one buffer can currently hold 2KB of data at a time. The crossbar has to be configured to map each integer in time and space to the interface wires. Do we then write more than 2KB of configuration data to give coordinates for each integer? No, in our case we configure an AXI burst size worth of records. Each burst has a full number of records transferred. So we need to configure records per burst amount of data. In our example the system would be configured to transfer 16 (512/32) records per burst and each record is fit into 2 (32/16) chunks of data. So we would need to configure 32 chunks of data which is 512 integers. 

As a side note, the system can only transfer power of 2 number of records. And it can't transfer more than 32 records. In our case 16 is a power of 2 and it is less than 32. But it if was more than 32 then we'd just choose 32 instead and if it wasn't a power of 2 we'd choose the next available power of 2 number.

## Input crossbar configuration

Now, the configuration is done in 2 phases. First we configure the chunk selection and then the position within the chunk selection. 

0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0
1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1
2|2|2|2|2|2|2|2|2|2|2|2|2|2|2|2
3|3|3|3|3|3|3|3|3|3|3|3|3|3|3|3
4|4|4|4|4|4|4|4|4|4|4|4|4|4|4|4
5|5|5|5|5|5|5|5|5|5|5|5|5|5|5|5
6|6|6|6|6|6|6|6|6|6|6|6|6|6|6|6
7|7|7|7|7|7|7|7|7|7|7|7|7|7|7|7
8|8|8|8|8|8|8|8|8|8|8|8|8|8|8|8
9|9|9|9|9|9|9|9|9|9|9|9|9|9|9|9
10|10|10|10|10|10|10|10|10|10|10|10|10|10|10|10
11|11|11|11|11|11|11|11|11|11|11|11|11|11|11|11
12|12|12|12|12|12|12|12|12|12|12|12|12|12|12|12
13|13|13|13|13|13|13|13|13|13|13|13|13|13|13|13
14|14|14|14|14|14|14|14|14|14|14|14|14|14|14|14
15|15|15|15|15|15|15|15|15|15|15|15|15|15|15|15
16|16|16|16|16|16|16|16|16|16|16|16|16|16|16|16
17|17|17|17|17|17|17|17|17|17|17|17|17|17|17|17
18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18
19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19
20|20|20|20|20|20|20|20|20|20|20|20|20|20|20|20
21|21|21|21|21|21|21|21|21|21|21|21|21|21|21|21
22|22|22|22|22|22|22|22|22|22|22|22|22|22|22|22
23|23|23|23|23|23|23|23|23|23|23|23|23|23|23|23
24|24|24|24|24|24|24|24|24|24|24|24|24|24|24|24
25|25|25|25|25|25|25|25|25|25|25|25|25|25|25|25
26|26|26|26|26|26|26|26|26|26|26|26|26|26|26|26
27|27|27|27|27|27|27|27|27|27|27|27|27|27|27|27
28|28|28|28|28|28|28|28|28|28|28|28|28|28|28|28
29|29|29|29|29|29|29|29|29|29|29|29|29|29|29|29
30|30|30|30|30|30|30|30|30|30|30|30|30|30|30|30
31|31|31|31|31|31|31|31|31|31|31|31|31|31|31|31

0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15
0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15

This is a simple example but we can see that for the first integer we select it to be from the 1st (i=0) chunk. And we do the same for all other integers. After the first phase we start looking for positions within the chunk. Again for the first integer we select it to be from the first position within the chunk. We can think of phase one shuffling data vertically and then afterward phase two shuffles data horisontally.

## Configuration parameters and shuffling

But that was a trivial case with no actual shuffling. What if we don't want all of the columns on the interface? Then we can start ignoring some of the data and shifting the data we want. The way we can configure the crossbar configuration itself is by specifying the size of the initial record and then a vector of integer indexes. For the previous configuration the input would have been:

```
record_size = 32;
selected_columns = {0,1,2,3,...,30,31};
```

Let's say we only want the string data and none of the integer data from our table. In that case the input will be:

```
record_size = 32;
selected_columns = {1,2,3,...,28,29};
```

We can see that we are not interested in the first and the last two integers for each record. To show which configuration inputs mark the garbage data I use X but in reality the FPGA will recieve chunk ID 31 and position 0 for those.  

[Chunk selection]

[Position selection]

Here we can see some of the shuffling taking place. For example, [Some example]

## Output and non-aligned data

There's another crossbar to worry about as well. The one which places data at the other end of the stream. Output configuration is the other way around in terms of the 2 phases. The position is chosen first and the chunk is chosen second.

Let's continue with the example we ended with at the input side. Here we can start seeing a slight complication. Since we don't want any garbage data between the records we won't have aligned data in the buffers any more after removing some of the data columns from the original table. Every record won't start from position 0 in the chunk which ID is equal to element/16. 

We can see first the position selection and then the chunk selection configuration tables below. One thing you can notice is that the chunk selection table would be the same for the input if the table originally started with records which size is 29 integers. And we are still configuring 512 integers since 512/29 = ~17.7. Log2(17) = ~4.1 and 2^4 = 16. 16 X 2 X 16 = 512.

[Position selection]

[Chunk selection]
 
But the position selection is different since the order of the phases is reversed.

Non-aligned data isn't that big of a problem once you calculate how many records does it take to reach an aligned record again. Then this number of records can be iterated over and different cases have been tested against in this unit test[LINK TO TEST] But shuffling causes a lot more problems even on its own. This is discussed in the next section.

# The issues of modifying data with the crossbars

Let's look at TPC-H part. This is how a record looks like.

Before we go into modifying the data we also need to discuss one additional quirk of the system.

## Number of chunks_per_record

...