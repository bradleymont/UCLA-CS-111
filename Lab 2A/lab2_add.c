// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <time.h> //for clock_gettime(2)
#include <pthread.h> //for pthread library
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi
#include <string.h> //for atoi

#include <stdio.h> //for fprintf(3), stderror(3)

void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    *pointer = sum;
}

int main(int argc, char **argv)
{
    int exitStatus = 0;
    
    long long counter = 0;
    
    int numThreads = 1;
    int numIterations = 1;
    
    static struct option long_options[] =
    {
        {"threads",    required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {0,            0,                 0,      0}
    };
    
    int optResult;
    
    while (1)
    {
        optResult = getopt_long(argc, argv, "", long_options, NULL);
        
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        switch (optResult)
        {
            case 't':
                if (optarg != NULL)
                {
                    numThreads = atoi(optarg);
                }
                break;
            case 'i':
                if (optarg != NULL)
                {
                    numIterations = atoi(optarg);
                }
                break;
            case '?':
                fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_add --threads=numThreads --iterations=numIterations\n");
                fflush(stderr);
                exitStatus = 1;
                break;
        }
    }
    
    printf("threads: %d\n", numThreads);
    printf("iterations: %d\n", numIterations);
    
    return exitStatus;
}
