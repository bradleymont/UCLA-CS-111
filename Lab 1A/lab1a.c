// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <unistd.h> //for close(2), dup2(2), execvp(3), fork(2), getopt_long(3), pipe(2), execvp(3)
#include <getopt.h> //for getopt_long(3)
#include <fcntl.h> //for open(2)
#include <signal.h> //for sigaction(2)
#include <stdio.h> //for fprintf(3), stderror(3)
#include <stdlib.h> //for malloc,exit(2)
#include <string.h> //for stderror(3), strcpy
#include <errno.h> //to get errno for error messages

void executeCommand(int in, int out, int err, char* cmd, char** args, int argCount)
{
    printf("in: %d\n", in);
    printf("out: %d\n", out);
    printf("err: %d\n", err);
    printf("cmd: %s\n", cmd);

    for (int i = 0; i < argCount; i++)
    {
        printf("argument #%d: %s\n", i + 1, args[i]);
    }
    
    char* arguments[argCount + 2];
    arguments[0] = cmd;
    for (int i = 1; i <= argCount; i++)
    {
        arguments[i] = args[i - 1];
    }
    arguments[1 + argCount] = NULL;

    execvp(arguments[0], arguments);
}

void parseCommandArguments(int argc, char **argv)
{
    optind--;
    
    int count = 0;
    
    int in;
    int out;
    int err;
    char* cmd;
    char** args = (char **) malloc(sizeof(char *));
    int cmdArgCount = 0;
    
    while (optind < argc)
    {
        //check to see if the currArg starts with two dashes. If so, then break
        char* currArg = argv[optind];
        int strLength = strlen(currArg);

        if (strLength >= 2)
        {
            char firstTwoCharacters[3];
            strncpy(firstTwoCharacters, currArg, 2);
            firstTwoCharacters[2] = 0;
            
            char* twoDashes = "--";
            
            if (strcmp(firstTwoCharacters, twoDashes) == 0)
            {
                break;
            }
        }
        
        switch (count)
        {
            case 0:
                in = atoi(currArg);
                break;
            case 1:
                out = atoi(currArg);
                break;
            case 2:
                err = atoi(currArg);
                break;
            case 3:
                count++;
                count--;
                //change this to something smarter!!!
                int argLength = strlen(currArg);
                cmd = (char*) malloc(argLength * sizeof(char));
                strcpy(cmd, currArg);
                break;
            default:    //one of cmd's arguments
                args[cmdArgCount] = currArg;
                char **largerArgArray = realloc(args, (cmdArgCount + 2) * sizeof(char*));
                
                if (largerArgArray == NULL)
                {
                    free(largerArgArray);
                    fprintf(stderr, "Error allocating more memory.");
                    exit(1);
                }
                
                args = largerArgArray;
                cmdArgCount++;
        }
        
        optind++;
        count++;
    } //end of while loop
    executeCommand(in, out, err, cmd, args, cmdArgCount);
}

void openFile(char* fileName, int permission, char** arrayOfFiles, int position)
{
    //first, open the file, setting it to the lowest unused file descriptor
    int fileDescriptor = open(fileName, permission);
    
    printf("putting file %s in file descriptor #%d\n", fileName, fileDescriptor);
    
    if (fileDescriptor < 0)
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when trying to open the following file name: %s\n", fileName);
    }
    
    //then, add the file to our array of files
    arrayOfFiles[position] = fileName;
    
    char** largerFileArray = realloc(arrayOfFiles, (position + 2) * sizeof(char*));
    
    if (largerFileArray == NULL)
    {
        free(arrayOfFiles);
        fprintf(stderr, "Error allocating more memory.");
        exit(1);
    }
    
    arrayOfFiles = largerFileArray;
}

int main(int argc, char **argv)
{
//    char* args[3];
//    args[0] = "ls";
//    args[1] = "CS33";
//    args[2] = NULL;
//
//    execvp(args[0], args);
//    return 0;
    
    static struct option long_options[] =
    {
        {"rdonly",  required_argument, NULL, 'r'},
        {"wronly",  required_argument, NULL, 'w'},
        {"command", required_argument, NULL, 'c'},
        {"verbose", no_argument,       NULL, 'v'},
        {0,         0,                 0,      0}
    };
    
    //array of file names
    char ** files = (char **) malloc(sizeof(char *));
    int fileDescriptorNum = 0;
    
    int optResult;
    
    while (1)
    {
        optResult = getopt_long(argc, argv, "", long_options, NULL);
        
        //getopt_long returns -1 if all command-line options have been parsed (getopt man page)
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        switch (optResult)
        {
            case 'r':
                openFile(optarg, O_RDONLY, files, fileDescriptorNum);
                fileDescriptorNum++;
                break;
            case 'w':
                openFile(optarg, O_WRONLY, files, fileDescriptorNum);
                fileDescriptorNum++;
                break;
            case 'c':
                parseCommandArguments(argc, argv);
                break;
            case 'v':
                //will implement later!!!
                break;
            case '?':
                fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab1a --rdonly [fileName] --wronly [fileName] --command [stdin] [stdout] [stderr] [executable] [args] --verbose\n");
                break;
        }
    }
}
