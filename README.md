# thinfile and make_example_thinfile

# make_example_thinfile  
Create an example binary file to be chopped up by thinfile. It establishes a minor counter, which counts up to the number of 
minor samples per major counter, and also a major counter for keeping track of the number of time the minor counter has reached
the number of minor sample counts per major counter.
NOTE: At the beginning of each major frame, the value of the first sample is the value of the major counter.
The structure of the file produced is as follows:                      

```
[SAMPLE TYPE]                                :  [SAMPLE VALUE]         
Header 0 (optional)                          :  [0x48 (ASCII "H")]                 
...                                          :  ...                    
Header HBYTES-1                              :  [0x48 (ASCII "H")]                 
Sample 0                                     :  [Major_count]          
Sample 1                                     :  [1]                    
Sample 2                                     :  [2]                    
...                                          :  ...                    
Sample (N_MINOR_PER_MAJOR - 2)               :  [N_MINOR_PER_MAJOR - 1]          
Sample (N_MINOR_PER_MAJOR - 1)               :  [N_MINOR_PER_MAJOR]          
Sample (N_MINOR_PER_MAJOR)                   :  [Major_count]          
Sample (N_MINOR_PER_MAJOR + 1)               :  [1]                    
Sample (N_MINOR_PER_MAJOR + 2)               :  [2]                    
...                                          :  ...                    
Sample (N MOD N_MINOR_PER_MAJOR)             :  [Major_count]          
Sample (N + 1 MOD N_MINOR_PER_MAJOR)         :  [1]                    
Sample (N + 2 MOD N_MINOR_PER_MAJOR)         :  [2]                    
Footer 0  (optional)                         :  [0x46 (ASCII "F")]                 
...                                          :  ...                    
Footer FBYTES-1                              :  [0x46 (ASCII "F")]              
```
                                                                       
The total file size is HBYTES + (N_MINOR_PER_MAJOR x N_MAJOR ) x SAMPLE_SIZE + FBYTES

###Version history
==================
2015/11/16		v0.1	make_example_thinfile.c written, thinfile tested with example output

##Examples
==========

###Quick example (detailed example below)
=========================================

####Make an example file to thin:
=================================
```
./make_example_thinfile test_1.out -M 1000 -N 2000 -H 130 -F 150 -s 4
```

####Thin the file
=================
```
./thinfile test_1.out test_1_thinned.out -s 4 -S 1000 -t 1000 -n 20 -H 130 -F 150
```

####Inspect the file
=====================
```
od -t x2z --address-radix=d test_1_thinned.out | less
```

###Example with explanation
===========================
The included program, aptly titled "make_example_thinfile", is meant to be used to create an example file to be thinned to verify functionality.
Let's make a file that has samples which are 4 bytes each, has a header that is 130 bytes, and a footer that is 250 bytes. 

```
./make_example_thinfile test_1.out -M 1000 -N 2000 -H 130 -F 150 -s 4
```

The program output is 


```
N header bytes        : 130
N footer bytes        : 150
Sample size           : 4 bytes
N minor per major     : 1000
N major               : 2000
Output file           : test_1.out
Wrote 2000000 samples (8000000 bytes)
******************************
Wrote 8000280 bytes in total to test_1.out.
```

Now, let's get a subset of these samples with thinfile. We'll tell thinfile there are 4 bytes per sample, that the sample rate is 1000 S/s, and that 
we want to grab 20 samples every 1000 milliseconds. Given the structure of the example thinfile made above, we provide the following command-line options:
-s    : 4 bytes/sample
-S    : 1000 samples/s
-t    : 1000 ms thinning interval
-n    : Grab the first 20 samples at the beginning of the thinning interval
-H    : Skip an initial 130 header bytes
-F    : Exclude the last 150 footer bytes

```
./thinfile test_1.out test_1_thinned.out -s 4 -S 1000 -t 1000 -n 20 -H 130 -F 150
```

To inspect the file, do a hex dump:

```
od -t x2z --address-radix=d test_1_thinned.out | less
```

Every 80 bytes (20 samples), you should see the major frame counter increase:

```
0000000 0000 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 0
0000016 0004 0000 0005 0000 0006 0000 0007 0000  >................<
0000032 0008 0000 0009 0000 000a 0000 000b 0000  >................<
0000048 000c 0000 000d 0000 000e 0000 000f 0000  >................<
0000064 0010 0000 0011 0000 0012 0000 0013 0000  >................<
0000080 0001 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 1
0000096 0004 0000 0005 0000 0006 0000 0007 0000  >................<
0000112 0008 0000 0009 0000 000a 0000 000b 0000  >................<
0000128 000c 0000 000d 0000 000e 0000 000f 0000  >................<
0000144 0010 0000 0011 0000 0012 0000 0013 0000  >................<
0000160 0002 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 2
0000176 0004 0000 0005 0000 0006 0000 0007 0000  >................<
0000192 0008 0000 0009 0000 000a 0000 000b 0000  >................<
0000208 000c 0000 000d 0000 000e 0000 000f 0000  >................<
0000224 0010 0000 0011 0000 0012 0000 0013 0000  >................<
0000240 0003 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 3
...
0079840 03e6 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 998
0079856 0004 0000 0005 0000 0006 0000 0007 0000  >................<
0079872 0008 0000 0009 0000 000a 0000 000b 0000  >................<
0079888 000c 0000 000d 0000 000e 0000 000f 0000  >................<
0079904 0010 0000 0011 0000 0012 0000 0013 0000  >................<
0079920 03e7 0000 0001 0000 0002 0000 0003 0000  >................<  <---- Major counter is 999
0079936 0004 0000 0005 0000 0006 0000 0007 0000  >................<
0079952 0008 0000 0009 0000 000a 0000 000b 0000  >................<
0079968 000c 0000 000d 0000 000e 0000 000f 0000  >................<
0079984 0010 0000 0011 0000 0012 0000 0013 0000  >................<
```
