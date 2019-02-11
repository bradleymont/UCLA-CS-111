// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <time.h> //for clock_gettime(2)
#include <pthread.h> //for pthread library
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi, rand(3)
#include <string.h> //for atoi
#include <stdio.h> //for fprintf(3)
#include "SortedList.h"

//global variables
int numThreads = 1;
int numIterations = 1;
SortedList_t* list;
SortedListElement_t* listElements; //an array of linked list elements (each with a randomized key)
const int keySize = 5;
int numElements; //think about if we want this global
int opt_yield = 0;

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
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab2_list --threads=[numThreads] --iterations=[numIterations] --yield=[idl]\n");
    fflush(stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
        {"threads",    required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield",      required_argument, NULL, 'y'},
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
            case 'y':
                for (int i = 0; i < strlen(optarg); i++)
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
}
