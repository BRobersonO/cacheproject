# A Flexible Cache Simulator

This executable, which runs in the console, is named SIM and takes the following inputs:

./SIM <CACHE_SIZE> <_ASSOC> <_REPLACEMENT> <_WB> <TRACE_FILE>
  
  where: 
  
- <CACHE_SIZE> is the size of the simulated cache in bytes
- <_ASSOC> is the associativity
- <_REPLACEMENT> replacement policy: 0 means Least Recently Used (LRU), 1 means First in First Out (FIFO)
- <_WB> Write-back policy: 0 means write-through, 1 means write-back
- <TRACE_FILE> trace file name with full path
  
Trace files contain lines consististing of two part: operation type (read or write) and a byte address. The operation type is designation by a single uppercase character, 'R' or 'W', and the byte address is in hexideciaml format.
  An example couple of lines from a trace file:
  
    R 0x2356257
    
    W 0x257777

For example, `./SIM 32768 8 1 1 /home/TRACES/MCF.t` will simulate a 32KB write-back cache with 8-way associativity, and a FIFO replacement
policy. The memory trace will be read from /home/TRACES/MCF.t.

A 64 byte block size is assumed for all configurations. 

The simulator will output:

- The miss ratio
- The number of writes to memory
- The number of reads to memory
