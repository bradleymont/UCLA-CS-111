// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <time.h> //for clock_gettime(2)
#include <pthread.h> //for pthread library
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi
#include <string.h> //for atoi
#include <stdio.h> //for fprintf(3)

//global constants to represent each type of synchronization
#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'
#define COMP_AND_SWAP 'c'

//global variables
long long counter = 0;
int numThreads = 1;
int numIterations = 1;
int opt_yield = 0;

char synchronization = NO_LOCK;

void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if (opt_yield)
    {
        sched_yield();
    }
    *pointer = sum;
}

void addCaller()
{
    for (int i = 0; i < numIterations; i++)
    {
        add(&counter, 1);
    }
    for (int i = 0; i < numIterations; i++)
    {
        add(&counter, -1);
    }
}

void printUsage(int* exitStatus)
{
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_add --threads=[numThreads] --iterations=[numIterations] --sync=[m|s|c]\n");
    fflush(stderr);
    *exitStatus = 1;
}

int main(int argc, char **argv)
{
    //note the (high resolution) starting time for the run
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    long long start = (startTime.tv_sec * 1000000000) + startTime.tv_nsec;
    
    int exitStatus = 0;
    
    static struct option long_options[] =
    {
        {"threads",    required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield",      no_argument,       NULL, 'y'},
        {"sync",       required_argument, NULL, 's'},
        {0,            0,                 0,      0}
    };
    
    int optResult;
    
    //think about optional_argument vs required argument - checking optarg=null
    
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
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                synchronization = optarg[0];
                //THINK ABOUT ERROR CHECKING LATER
                break;
            case '?':
                printUsage(&exitStatus);
                break;
        }
    }
    
    //array of threads
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    
    //create each thread
    for (int i = 0; i < numThreads; i++)
    {
        //thread, attributes, routine to be executed, argument passed to routine
        int createReturn = pthread_create(&threads[i], NULL, (void *) &addCaller, NULL);
        if (createReturn)   //if an error occurred creating the thread
        {
            fprintf(stderr, "Error creating thread #%d\n", i + 1);
            fflush(stderr);
            exitStatus = 1;
            //check if exit status equals 1 or 2
        }
    }
    
    //wait for all threads to complete
    for (int i = 0; i < numThreads; i++)
    {
        int joinReturn = pthread_join(threads[i], NULL);
        if (joinReturn)   //if an error occurred creating the thread
        {
            fprintf(stderr, "Error joining thread #%d\n", i + 1);
            fflush(stderr);
            exitStatus = 1;
            //check if exit status equals 1 or 2
        }
    }
    
    //note the (high resolution) end time
    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    long long end = (endTime.tv_sec * 1000000000) + endTime.tv_nsec;
    
    //print CSV record
    
    
    
    
    
    //name of the test, numThreads, numIterations, operationsPerformed, run time, avg time per operation, total at the end
    char testName[15];
    if (opt_yield)  //yield argument passed
    {
        switch (synchronization)
        {
            case NO_LOCK:
                strcpy(testName, "add-yield-none");
                break;
            case MUTEX:
                strcpy(testName, "add-yield-m");
                break;
            case SPIN_LOCK:
                strcpy(testName, "add-yield-s");
                break;
            case COMP_AND_SWAP:
                strcpy(testName, "add-yield-c");
                break;
        }
    }
    else //no yield
    {
        switch (synchronization)
        {
            case NO_LOCK:
                strcpy(testName, "add-none");
                break;
            case MUTEX:
                strcpy(testName, "add-m");
                break;
            case SPIN_LOCK:
                strcpy(testName, "add-s");
                break;
            case COMP_AND_SWAP:
                strcpy(testName, "add-c");
                break;
        }
    }
    
    int numOperations = numThreads * numIterations * 2;
    long long runTime = end - start;
    long long avgTimePerOperation = runTime / numOperations;
    
    printf("%s,%d,%d,%d,%lld,%lld,%lld", testName, numThreads, numIterations, numOperations, runTime, avgTimePerOperation, counter);
    printf("\n");
    fflush(stdout);
    
    //free dynamically allocated memory
    free(threads);
    
    return exitStatus;
}

