// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <time.h> //for clock_gettime(2)
#include <pthread.h> //for pthread library
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi, rand(3)
#include <string.h> //for atoi
#include <stdio.h> //for fprintf(3)
#include <signal.h> //for signal(2)
#include "SortedList.h"

//global constants to represent each type of synchronization
#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'

struct Sublist
{
    SortedList_t* list;
    pthread_mutex_t myMutex;
    int mySpinLock;
};

//global variables
char synchronization = NO_LOCK;
struct Sublist* sublists;
SortedListElement_t* listElements; //an array of linked list elements (each with a randomized key)
int numThreads = 1;
int numIterations = 1;
int numLists = 1;
const int keySize = 5;
int numElements;
int opt_yield = 0;
long long* mutexWaitTimes;  //to keep track of the time spent waiting for mutex locks

int hash(const char* key)
{
    int ASCIIsum = 0;
    for (int i = 0; i < keySize; i++)
    {
        ASCIIsum += (int) key[i];
    }
    return ASCIIsum % numLists;
}

long long timeDifference(struct timespec start, struct timespec end)
{
    long long startTime = (start.tv_sec * 1000000000) + start.tv_nsec;
    long long endTime = (end.tv_sec * 1000000000) + end.tv_nsec;
    return endTime - startTime;
}

char* generateRandomKey()
{
    char* key = malloc((keySize + 1) * sizeof(char));   // +1 to have the null byte at the end
    for (int i = 0; i < keySize; i++)
    {
        //add 'a' to a random number between 0 and 25, thus generating a random ASCII character between 'a' and 'z'
        char randomLowercaseLetter = (rand() % 26) + 'a';
        key[i] = randomLowercaseLetter;
    }
    key[keySize] = '\0';    //C string must end with null byte
    return key;
}

SortedList_t* initializeEmptyList()
{
    SortedList_t* emptyList = malloc(sizeof(SortedList_t));
    emptyList->next = emptyList;
    emptyList->prev = emptyList;
    emptyList->key = NULL;
    return emptyList;
}

void initializeListElements()
{
    srand(time(NULL));  //create a seed for rand()
    numElements = numThreads * numIterations;
    
    listElements = malloc(numElements * sizeof(SortedListElement_t));
    
    for (int i = 0; i < numElements; i++)
    {
        //generate a random key (assume that each key is a string of 'keySize' random lowercase letters)
        char * randomKey = generateRandomKey();
        listElements[i].key = randomKey;
    }
}

void printUsage()
{
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_list --threads=[numThreads] --iterations=[numIterations] --lists=[numLists] --yield=[idl] --sync=[m|s]\n");
    fflush(stderr);
    exit(1);
}

void lock(struct timespec* waitStart, struct timespec* waitEnd, int index, struct Sublist* sublist)
{
    switch (synchronization)
    {
        case MUTEX:
            clock_gettime(CLOCK_MONOTONIC, waitStart); //measure time at the start of waiting for the lock
            pthread_mutex_lock(&sublist->myMutex);
            clock_gettime(CLOCK_MONOTONIC, waitEnd); //measure time after getting the lock
            mutexWaitTimes[index] += timeDifference(*waitStart, *waitEnd); //add the wait time to the mutex wait times array
            return;
        case SPIN_LOCK:
            while (__sync_lock_test_and_set(&sublist->mySpinLock, 1));
            return;
            //for NO_LOCK, do nothing
    }
}

void unlock(struct Sublist* sublist)
{
    switch (synchronization)
    {
        case MUTEX:
            pthread_mutex_unlock(&sublist->myMutex);
            return;
        case SPIN_LOCK:
            __sync_lock_release(&sublist->mySpinLock);
            return;
            //for NO_LOCK, do nothing
    }
}

void modifyLinkedList(void* startIndex)
{
    int start = *((int*) startIndex);
    int end = start + numIterations;
    
    //to time waiting for mutex lock
    struct timespec waitStart;
    struct timespec waitEnd;
    int mutexWaitIndex = start / numIterations;
    mutexWaitTimes[mutexWaitIndex] = 0;
    
    //insert all pre-allocated elements into the multi-list (which sublist the key should go into determined by a hash of the key)
    for (int i = start; i < end; i++)
    {
        const char* currKey = listElements[i].key;
        struct Sublist* currSublist = &sublists[hash(currKey)];
        lock(&waitStart, &waitEnd, mutexWaitIndex, currSublist);
        SortedList_insert(currSublist->list, &listElements[i]);
        unlock(currSublist);
    }
    
    //get the list length (enumerating all of the sub-lists)
    int listLength = 0;
    for (int i = 0; i < numLists; i++)
    {
        lock(&waitStart, &waitEnd, mutexWaitIndex, &sublists[i]);
        int sublistLength = SortedList_length(sublists[i].list);
        
        if (sublistLength == -1)
        {
            fprintf(stderr, "Error: SortedList_length function found a sublist to be corrupted.\n");
            fflush(stderr);
            exit(2);
        }
        
        listLength += sublistLength;
        
        unlock(&sublists[i]);
    }
    
    if (listLength < 0)
    {
        fprintf(stderr, "Error: List has a negative length.\n");
        fflush(stderr);
        exit(2);
    }
    
    //look up and delete each of the keys that were previously inserted
    for (int i = start; i < end; i++)
    {
        const char* currKey = listElements[i].key;
        struct Sublist* currSublist = &sublists[hash(currKey)];
        lock(&waitStart, &waitEnd, mutexWaitIndex, currSublist);
        SortedListElement_t* elementToDelete = SortedList_lookup(currSublist->list, currKey);
        if (elementToDelete == NULL)
        {
            fprintf(stderr, "Error: SortedList_lookup function could not find element: list has been corrupted.\n");
            fflush(stderr);
            exit(2);
        }
        unlock(currSublist);
        
        lock(&waitStart, &waitEnd, mutexWaitIndex, currSublist);
        int deleteStatus = SortedList_delete(elementToDelete);
        if (deleteStatus)
        {
            fprintf(stderr, "Error: SortedList_delete function found list to be corrupted.\n");
            fflush(stderr);
            exit(2);
        }
        unlock(currSublist);
    }
}

void segFaultHandler(int signum)
{
    if (signum == SIGSEGV)
    {
        fprintf(stderr, "Error: Segmentation fault caught by SIGSEGV handler: list has been corrupted.\n");
        fflush(stderr);
        exit(2);
    }
}

int main(int argc, char **argv)
{
    signal(SIGSEGV, segFaultHandler);
    
    static struct option long_options[] =
    {
        {"threads",    required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield",      required_argument, NULL, 'y'},
        {"sync",       required_argument, NULL, 's'},
        {"lists",      required_argument, NULL, 'l'},
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
                ; //to appease compiler
                int length = strlen(optarg);
                for (int i = 0; i < length; i++)
                {
                    char currChar = optarg[i];
                    switch (currChar)
                    {
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            printUsage();
                            break;
                    }
                }
                break;
            case 's':
                synchronization = optarg[0];
                if ( ! (synchronization == 'm' || synchronization == 's'))
                {
                    printUsage();
                }
                break;
            case 'l':
                numLists = atoi(optarg);
                break;
            default:
                printUsage();
                break;
        }
    }
    
    //initialize array of sublists
    sublists = malloc(numLists * sizeof(struct Sublist));
    
    //initialize each individual sublist
    for (int i = 0; i < numLists; i++)
    {
        sublists[i].list = initializeEmptyList();
        switch (synchronization)
        {
            case MUTEX:
                ; //to appease compiler
                int initReturn = pthread_mutex_init(&sublists[i].myMutex, NULL);
                if (initReturn)
                {
                    fprintf(stderr, "Error: Could not initialize mutex lock.\n");
                    fflush(stderr);
                    exit(2);
                }
                break;
            case SPIN_LOCK:
                sublists[i].mySpinLock = 0;
                break;
        }
    }
    
    initializeListElements(); //create and initialize (numThreads x numIterations) list elements (listElements)
    
    //note the (high resolution) starting time for the run
    struct timespec startTimeSpec;
    clock_gettime(CLOCK_MONOTONIC, &startTimeSpec);
    
    //array of time spent waiting for each mutex lock
    mutexWaitTimes = malloc(numThreads * sizeof(long long));
    
    //array of threads
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    
    //fill out array of the start index in the listElements array for each thread
    int* startIndices = malloc(numThreads * sizeof(int));
    for (int i = 0; i < numThreads; i++)
    {
        startIndices[i] = i * numIterations;
    }

    //create each thread
    for (int i = 0; i < numThreads; i++)
    {
        //each thread will be given numIterations elements of listElements as its parameter
        int createReturn = pthread_create(&threads[i], NULL, (void *) &modifyLinkedList, (void *) &startIndices[i]);
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
    struct timespec endTimeSpec;
    clock_gettime(CLOCK_MONOTONIC, &endTimeSpec);
    
    //check the length of each sublist to confirm that they are all zero
    for (int i = 0; i < numLists; i++)
    {
        if (SortedList_length(sublists[i].list) != 0)
        {
            fprintf(stderr, "Error: Length of list is not zero.\n");
            fflush(stderr);
            exit(2);
        }
    }
    
    //create test name
    char testName[15];
    strcat(testName, "list-");
    
    //yieldopts
    if (opt_yield & INSERT_YIELD)
    {
        strcat(testName, "i");
    }
    if (opt_yield & DELETE_YIELD)
    {
        strcat(testName, "d");
    }
    if (opt_yield & LOOKUP_YIELD)
    {
        strcat(testName, "l");
    }
    if (opt_yield == 0)
    {
        strcat(testName, "none");
    }
    
    //syncopts
    switch (synchronization)
    {
        case NO_LOCK:
            strcat(testName, "-none");
            break;
        case MUTEX:
            strcat(testName, "-m");
            break;
        case SPIN_LOCK:
            strcat(testName, "-s");
            break;
    }
    
    //print CSV
    int numOperations = numThreads * numIterations * 3;

    long long runTime = timeDifference(startTimeSpec, endTimeSpec);
    
    long long avgTimePerOperation = runTime / numOperations;
    
    //timing mutex waits
    long long mutexWaitTime = 0;
    
    if (synchronization == MUTEX)
    {
        for (int i = 0; i < numThreads; i++)
        {
            mutexWaitTime += mutexWaitTimes[i];
        }
    }
    
    long long numLockOperations = numThreads * (2 * numIterations + 1);  //check this
    
    long long avgMutexWaitTime = mutexWaitTime / numLockOperations;
    
    printf("%s,%d,%d,%d,%d,%lld,%lld,%lld", testName, numThreads, numIterations, numLists, numOperations, runTime, avgTimePerOperation, avgMutexWaitTime);
    printf("\n");
    
    //free dynamically allocated memory
    for (int i = 0; i < numIterations; i++)
    {
        free((void*) listElements[i].key);
    }
    free(listElements);
    free(threads);
    free(startIndices);
    free(mutexWaitTimes);
    for (int i = 0; i < numLists; i++)
    {
        free(sublists[i].list);
    }
    free(sublists);
    
    return 0;
}
