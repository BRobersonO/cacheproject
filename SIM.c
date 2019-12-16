/*
 *  A Flexible Cach Simulator by Blake Oakley
 *
 * to compile: gcc SIM.c -o SIM.exe
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#define BLOCK_SIZE 64 //set by Project Description
int main(int argc, char *argv[])
{
	int cache_size = atoi(argv[1]); // a 'log2' number
	int assoc = atoi(argv[2]); // Number of blocks in set (cache columns)
	int replc = atoi(argv[3]); // 0 == LRU, 1 == FIFO
	int wb = atoi(argv[4]); // 0 == write-through, 1 == write-back
	FILE *trace_file = fopen(argv[5], "r"); // contains data as Operatio HexAddress
	char buff[100]; // a buffer to hold a line of data from the trace file
	int index_size = (cache_size / (BLOCK_SIZE * assoc)); // gives Number of sets
    /*
     *  Create and Allocate Space for three 2D Arrays: Cache, replacement data, dirty bits
    */
	unsigned long long **the_cache = (unsigned long long **)calloc(index_size, sizeof(unsigned long long));
	for (int i = 0; i < index_size; i++)
	{
		the_cache[i] = (unsigned long long *)calloc(assoc, sizeof(unsigned long long));
	}
	//same size as cache
	int **replc_policy = (int **)calloc(index_size, sizeof(int));
	for (int i = 0; i < index_size; i++)
	{
		replc_policy[i] = (int *)calloc(assoc, sizeof(int));
	}
    //same size as cache
	int **wb_dirty_bits = (int **)calloc(index_size, sizeof(int));
	for (int i = 0; i < index_size; i++)
	{
		wb_dirty_bits[i] = (int *)calloc(assoc, sizeof(int));
	}
	char oper; // Either 'R' for Read or 'W' for Write
	unsigned long long addr; // hexidecimal value
	int set; //zero through index size, rows in the cache
	unsigned long long tag; //idetifier of address, used as value in the cache
	int access_hit; // Was there a hit? 1 = True, 0 = False
	int index_of_hit; // Where did the hit occur in the cache?
	int hits = 0; //How many hits
	int misses = 0; //how many misses
	int write_throughs = 0; // Writes to memory for every 'W'
    int write_backs = 0; // Writes to memory upon eviction (Rs and Ws),
                        //  when dirty bit marked 1
    int reads_to_memory = 0; //Counts number of 'R' operatoins on MISS (Reads to Memory)
    //Initialize replc policy array to -1 (-1 = Empty Spot in Cache)
	for (int i = 0; i < index_size; i++)
	{
		for (int j = 0; j < assoc; j++)
		{
			replc_policy [i] [j] = -1;
		}
	}
	while (!feof(trace_file)) 
	{
		fgets(buff, sizeof(buff), trace_file); //Get operation (R or W) and Address
		oper = buff[0]; // Grabs the first char of the line, a 'R' or a 'W' 
		addr = strtoull(&buff[2], NULL, 16); //Grabs the hexademical address (0x0000...)
		access_hit = 0; //Initialize to False
		index_of_hit = -1; //Initialize to invalid value
		set = (addr / BLOCK_SIZE) % index_size; //Determintes which row in cache
		tag = addr / BLOCK_SIZE; //Determines value use in cache
        if (oper == 'W')
		{	
            write_throughs++; //In write-through, all 'W's will be written to memory
        }
        /*
         *  Check for a HIT
        */
		for (int i = 0; i < assoc; i++)
		{
			if (tag == the_cache[set][i])
			{
				if (oper == 'R')
                {
                    hits++;
                    access_hit = 1;
                    index_of_hit = i;
                    break;
                }
				if (oper == 'W')
                {
                    wb_dirty_bits [set] [i] == 1;
                    hits++;
                    access_hit = 1;
                    index_of_hit = i;
                    break;
                }
			}
		}
        /*
         *  We have a HIT; now index a replc priority value in the replc data array
        */
		if (access_hit)
		{
			if (replc == 0) //Redefine Replacement Stack Values on hit ONLY IF Replc Policy is LRU
			{
				for (int i = 0; i < assoc; i ++)
				{
					if (i == index_of_hit)
					{
						replc_policy[set][index_of_hit] = 0; //Make the hit location 0
					}
					else if (replc_policy[set][i] != -1) 
					{
						replc_policy[set][index_of_hit] ++; //Increment all non-Empty locations
					}
				}
			}
		}
        /*
         *  We have a MISS
        */
		else
		{
            misses ++; // tracks misses, used to compute miss ratio
			int flag = 0; //Flag will determine control flow for empty vs non-Empty spot
            reads_to_memory ++; //Read to Memory occurs with both 'R' and 'W' on a miss
            /*
             *  Check for an empty spot in the cache
            */
			for (int i = 0; i < assoc; i ++)
			{
				if (replc_policy [set] [i] == -1) //Empty spot found
				{
					for (int j = 0; j < assoc; j ++)
					{
						if (replc_policy [set] [j] != -1)
                        {
                            replc_policy [set] [j] ++; //Increment all non-Empty spots
                        }
					}
                    if (oper == 'W') //Mark Dirty Bit for Writes Filling Empty Spots
                    {    
                        wb_dirty_bits [set] [i] = 1;
                    }
					the_cache [set] [i] = tag; //put the tag value into the cache
                    replc_policy [set] [i] = 0; //most recent value has priority value 0
					flag = 1; //do not evict a block
					break; //don't keep looking for empty spots
				}
			}
            /*
             *  There are no empty spots; time to evict and replace a cache value
            */
			if (!flag)
			{ 			
				int maximum = replc_policy[set][0]; //initialize to the first location
				int max_location = 0;//initialize to 0
				for (int i = 1; i < assoc; i ++) //Find the block to be evicted
				{
					if (replc_policy [set] [i] > maximum)
					{
						maximum = replc_policy [set] [i]; //find highest value (lru)
						max_location = i; //set to location where lru was found
					}
				}
				for (int i = 0; i < assoc; i ++) //Adjust replc priority values
				{
                    //just increment everything; the saved location will be set to 0
					replc_policy [set] [i] ++; 
				}
                //Check for Dirty Bit Write Back on Evicted Value
                if (wb_dirty_bits [set] [max_location] == 1 )
                {
                    write_backs ++; //happens for both 'R's and 'W's
                    wb_dirty_bits [set] [max_location] = 0; //marked not dirty
                }
                //Mark Dirty Bit for Writes Replacing old value
                if (oper == 'W')
                {
                    wb_dirty_bits [set] [max_location] = 1; //only stays not dirty for 'R's
                }
				the_cache [set] [max_location] = tag; //Put it in the cache
                replc_policy [set] [max_location] = 0;
			}
		}
	}
	fclose(trace_file);
    /*
     *  Print out miss ratio, writes, and reads to memory
    */
	printf("%2.6f \n", ((double)misses / (double)(hits + misses)));
    if (wb == 0)
    {
        printf("%d \n", write_throughs);
    }
    else
    {
        printf("%d \n", write_backs);
    }
    printf("%d \n", reads_to_memory);
	return 0;
}