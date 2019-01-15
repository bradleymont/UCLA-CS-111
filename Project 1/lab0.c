// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <stdio.h> //for fprintf(3), stderror(3)
#include <unistd.h> //for read(2),write(2)
#include <stdlib.h> //for malloc,exit(2)
#include <getopt.h> //for getopt_long(3)
#include <fcntl.h> //for open(2)
#include <signal.h> //for signal(2)
#include <string.h> //for stderror(3)
#include <errno.h> //to get errno for error messages

void setInputFile(char* fileName)
{
    int ifd = open(fileName, O_RDONLY);
    if (ifd >= 0)
    {
        close(0);
        dup(ifd);
        close(ifd);
    }
    else //if we're unable to open the file
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when using --input argument to open the following file name: %s\n", fileName);
        //exit(1);
        //the test script says EXIT_BADIN=2
        exit(2);
    }
}

void setOutputFile(char* fileName)
{
    int ofd = creat(fileName, 0666);
    if (ofd >= 0)
    {
        close(1);
        dup(ofd);
        close(ofd);
    }
    else
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when using --output argument to output to the following file name: %s\n", fileName);
        //exit(2);
        //the test script says EXIT_BADOUT=3
        exit(3);
    }
}

void segFaultHandler(int signum)
{
    if (signum == SIGSEGV)
    {
        fprintf(stderr, "Segmentation fault caught by SIGSEGV handler.\n");
        exit(4);
    }
}

void forceSegFault(int catchFlag)
{
    if (catchFlag)
    {
        signal(SIGSEGV, segFaultHandler);
    }
    
    char* will_cause_segfault = NULL;
    *will_cause_segfault = 'f'; //dereferencing a null pointer will cause a segmentation fault
}

int main(int argc, char **argv)
{
    int catchSegfault = 0;
    
    //long options table: array of struct options to represent the command line options
    //each option has a (const char* name, int has_arg, int* flag, int val)
    static struct option long_options[] =
    {
        {"input",     required_argument, NULL, 'i'},
        {"output",    required_argument, NULL, 'o'},
        {"segfault",  no_argument,       NULL, 's'},
        {"catch",     no_argument,       NULL, 'c'},
        {"dump-core", no_argument,       NULL, 'd'},
        {0,           0,                 0,    0}
    };
    
    int optResult;
    
    //First check all arguments for argument syntax
    while (1)
    {
        //option_index will store the index of the option in long_options that is currently being parsed by getopt_long
        //it will be -1 until getopt_long modifies its value
        int option_index = -1;
        
        //since we only use long option arguments, we set optstring equal to "" (getopt man page)
        optResult = getopt_long(argc, argv, "", long_options, &option_index);
        
        //getopt_long returns -1 if all command-line options have been parsed (getopt man page)
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        if (optResult == '?')   //an error occurred
        {
            fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab0 --input=[inputName] --output=[outputName] --segfault --catch --dump-core\n");
            //exit(3);
            //the test script says to exit with 1
            exit(1);
        }
    }
    
    if (optind < argc)  //non-option arguments passed - throw an error
    {
        fprintf(stderr, "Error: non-option argument.\nUsage: ./lab0 --input=[inputName] --output=[outputName] --segfault --catch --dump-core\n");
        exit(1);
    }
    
    //reset optind to iterate through arguments again
    optind = 1;
    
    while (1)
    {
        int option_index = -1;
        optResult = getopt_long(argc, argv, "", long_options, &option_index);
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        switch (optResult)
        {
            case 'i': //if the input flag is passed, use that file as standard input (input redirection)
                setInputFile(optarg);
                break;
            case 'o': //if the output flag is passed, use that file as standard output (output redirection)
                setOutputFile(optarg);
                break;
            case 's':
                forceSegFault(catchSegfault);
                break;
            case 'c':
                catchSegfault = 1;
                break;
            case 'd':
                catchSegfault = 0;
                break;
        }
    }
    
    //READING AND WRITING
    while(1)
    {
        //create a buffer to read into
        char* currChar = (char *) malloc(sizeof(char));
        
        //read from stdin into the buffer, one byte at a time
        ssize_t bytesRead = read(0, currChar, 1);
        
        //bytesRead now contains the amount of bytes read (negative if error occurred)
        if (bytesRead == 0) break;  //if we didn't read any bytes, we've reached the end of the file (break)
        
        if (bytesRead < 0)
        {
            fprintf(stderr, "Error reading input.");
            exit(2);
        }
        
        //write the current char in the buffer to stdout
        ssize_t bytesWritten = write(1, currChar, 1);
        
        //if bytesWritten is zero or negative, then an error occurred
        if (bytesWritten <= 0)
        {
            fprintf(stderr, "Error writing output.");
            exit(3);
        }
    }
    
    exit(0);
}
