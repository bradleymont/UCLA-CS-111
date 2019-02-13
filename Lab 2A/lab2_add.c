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
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
int mySpinLock = 0;

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

void addValue(int val)
{
    for (int i = 0; i < numIterations; i++)
    {
        switch (synchronization)
        {
            case MUTEX:
                pthread_mutex_lock(&myMutex);
                add(&counter, val);
                pthread_mutex_unlock(&myMutex);
                break;
            case SPIN_LOCK:
                while (__sync_lock_test_and_set(&mySpinLock, 1));
                add(&counter, val);
                __sync_lock_release(&mySpinLock);
                break;
            case COMP_AND_SWAP:
                ;   //to appease compiler
                long long oldval, sum;
                do
                {
                    oldval = counter;
                    sum = oldval + val;
                    if(opt_yield)
                    {
                        sched_yield();
                    }
                } while(__sync_val_compare_and_swap(&counter, oldval, sum) != oldval);
                break;
            case NO_LOCK:
                add(&counter, val);
                break;
        }
    }
}

void addCaller()
{
    addValue(1);
    addValue(-1);
}

void printUsage()
{
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_add --threads=[numThreads] --iterations=[numIterations] --yield --sync=[m|s|c]\n");
    fflush(stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    //note the (high resolution) starting time for the run
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    long long start = (startTime.tv_sec * 1000000000) + startTime.tv_nsec;
    
    static struct option long_options[] =
    {
        {"threads",    required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield",      no_argument,       NULL, 'y'},
        {"sync",       required_argument, NULL, 's'},
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
                numThreads = atoi(optarg);
                break;
            case 'i':
                numIterations = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                synchronization = optarg[0];
                if ( ! (synchronization == 'm' || synchronization == 's' || synchronization == 'c'))
                {
                    printUsage();
                }
                break;
            case '?':
                printUsage();
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
            exit(1);
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
            exit(1);
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
    return 0;
}

