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

//global variables
char synchronization = NO_LOCK;
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
int mySpinLock = 0;

int numThreads = 1;
int numIterations = 1;
SortedList_t* list;
SortedListElement_t* listElements; //an array of linked list elements (each with a randomized key)
const int keySize = 5;
int numElements;
int opt_yield = 0;

void lock()
{
    switch (synchronization)
    {
        case MUTEX:
            pthread_mutex_lock(&myMutex);
            return;
        case SPIN_LOCK:
            while (__sync_lock_test_and_set(&mySpinLock, 1));
            return;
        //for NO_LOCK, do nothing
    }
}

void unlock()
{
    switch (synchronization)
    {
        case MUTEX:
            pthread_mutex_unlock(&myMutex);
            return;
        case SPIN_LOCK:
            __sync_lock_release(&mySpinLock);
            return;
        //for NO_LOCK, do nothing
    }
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

void initializeEmptyList()
{
    list = malloc(sizeof(SortedList_t));
    list->next = list;
    list->prev = list;
    list->key = NULL;
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
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_list --threads=[numThreads] --iterations=[numIterations] --yield=[idl] --sync=[m|s]\n");
    fflush(stderr);
    exit(1);
}

void modifyLinkedList(void* startIndex)
{
    int start = *((int*) startIndex);
    int end = start + numIterations;
    
    //insert all pre-allocated elements into a (single shared-by-all-threads) list
    for (int i = start; i < end; i++)
    {
        lock();
        SortedList_insert(list, &listElements[i]);
        unlock();
    }
    
    //get the list length
    lock();
    int listLength = SortedList_length(list);
    unlock();
    
    if (listLength == -1)
    {
        fprintf(stderr, "Error: SortedList_length function found list to be corrupted.\n");
        fflush(stderr);
        exit(2);
    }
    
    //look up and delete each of the keys that were previously inserted
    //listElements
    for (int i = start; i < end; i++)
    {
        const char* currKey = listElements[i].key;
        
        lock();
        SortedListElement_t* elementToDelete = SortedList_lookup(list, currKey);
        unlock();
        
        if (elementToDelete == NULL)
        {
            fprintf(stderr, "Error: SortedList_lookup function could not find element: list has been corrupted.\n");
            fflush(stderr);
            exit(2);
        }
        lock();
        int deleteStatus = SortedList_delete(elementToDelete);
        unlock();
        if (deleteStatus)
        {
            fprintf(stderr, "Error: SortedList_delete function found list to be corrupted.\n");
            fflush(stderr);
            exit(2);
        }
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
            default:
                printUsage();
                break;
        }
    }
    
    initializeEmptyList(); //initialize an empty list (list)
    
    initializeListElements(); //create and initialize (numThreads x numIterations) list elements (listElements)
    
    //note the (high resolution) starting time for the run
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    long long start = (startTime.tv_sec * 1000000000) + startTime.tv_nsec;
    
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
    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    long long end = (endTime.tv_sec * 1000000000) + endTime.tv_nsec;
    
    //check the length of the list to confirm that it is zero
    if (SortedList_length(list) != 0)
    {
        fprintf(stderr, "Error: Length of list is not zero.\n");
        fflush(stderr);
        exit(2);
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
    long long runTime = end - start;
    long long avgTimePerOperation = runTime / numOperations;

    printf("%s,%d,%d,%d,%d,%lld,%lld", testName, numThreads, numIterations, 1, numOperations, runTime, avgTimePerOperation);
    printf("\n");
    
    //free dynamically allocated memory
    for (int i = 0; i < numIterations; i++)
    {
        free((void*) listElements[i].key);
    }
    free(list);
    free(listElements);
    free(threads);
    free(startIndices);
    
    return 0;
}
