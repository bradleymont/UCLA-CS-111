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
#include <sys/wait.h> //for wait(2)

//FREE YOUR MEMORY
//THINK ABOUT WHEN THE COMMAND CMD HAS ZERO ARGS

/* for printing exit codes
 
 
 //    if (verbose)
 //    {
 //        printf("--command %d %d %d %s", in, out, err, cmd);
 //        for (int i = 0; i < cmdArgCount; i++)
 //        {
 //            printf(" %s", args[i]);
 //        }
 //        printf("\n");
 //    }
 
 
 //print out status after finishing
 //    printf("exit %d %s", exitCode, cmd);
 //
 //    for (int i = 0; i < cmdArgCount; i++)
 //    {
 //        printf(" %s", args[i]);
 //    }
 //    printf("\n");
 
 
*/

int startsWithTwoDashes(char* arg)
{
    int strLength = strlen(arg);
    
    if (strLength >= 2)
    {
        char firstTwoCharacters[3];
        strncpy(firstTwoCharacters, arg, 2);
        firstTwoCharacters[2] = 0;
        
        char* twoDashes = "--";
        
        if (strcmp(firstTwoCharacters, twoDashes) == 0)
        {
            return 1;
        }
    }
    return 0;
}


struct commandFlagArgs
{
    int in, out, err;
    char* cmd;
    char** args;
    int numArgs;
};




int executeCommand(struct commandFlagArgs cmdArgs, int * fileDescriptors)
{
    int exitStatus = 0;
    
    int PID = fork();
    
    if (PID < 0)    //the fork failed
    {
        fprintf(stderr, "Error: fork command failed.\n");
        exit(1);
    }
    else if (PID == 0)  //child process
    {
        //IO REDIRECTION
        
        //update stdin
        int newStdin = fileDescriptors[cmdArgs.in];
        close(0);
        dup2(newStdin, 0);
        close(newStdin);
        
        //update stdout
        int newStdout = fileDescriptors[cmdArgs.out];
        close(1);
        dup2(newStdout, 1);
        close(newStdout);
        
        //update stderr
        int newStderr = fileDescriptors[cmdArgs.err];
        close(2);
        dup2(newStderr, 2);
        close(newStderr);
        
        char* arguments[cmdArgs.numArgs + 2];
        arguments[0] = cmdArgs.cmd;
        
        for (int i = 1; i <= cmdArgs.numArgs; i++)
        {
            arguments[i] = cmdArgs.args[i - 1];
        }
        
        arguments[cmdArgs.numArgs + 1] = NULL;
      
        execvp(arguments[0], arguments);
    }
    else    //parent process
    {
        //idk what to do
        //int childPID = wait(NULL);
        //wait(NULL);
    }
    
    return exitStatus;
}

struct commandFlagArgs parseCommandArguments(int argc, char **argv)
{
    struct commandFlagArgs result;
    result.args = (char **) malloc(sizeof(char *));
    
    optind--;
    
    int count = 0;
    int cmdArgCount = 0;
    
    while (optind < argc)
    {
        char* currArg = argv[optind];
        
        //check to see if the currArg starts with two dashes. If so, then break
        if (startsWithTwoDashes(currArg))
        {
            break;
        }
        
        switch (count)
        {
            case 0:
                result.in = atoi(currArg);
                break;
            case 1:
                result.out = atoi(currArg);
                break;
            case 2:
                result.err = atoi(currArg);
                break;
            case 3:
                count++;
                count--;
                //change this to something smarter!!!
                int argLength = strlen(currArg);
                result.cmd = (char*) malloc(argLength * sizeof(char));
                strcpy(result.cmd, currArg);
                break;
            default: //one of cmd's arguments
                result.args[cmdArgCount] = currArg;
                char **largerArgArray = realloc(result.args, (cmdArgCount + 2) * sizeof(char*));
            
                if (largerArgArray == NULL)
                {
                    free(largerArgArray);
                    fprintf(stderr, "Error allocating more memory.");
                    exit(1);
                }
                result.args = largerArgArray;
                cmdArgCount++;
        }   //end of switch
        
        optind++;
        count++;
    }  //end of while
    
    result.numArgs = cmdArgCount;
    
    return result;
}

int openFile(char* fileName, int permission, int* fileDescriptors, int position)
{
    int openFileReturnStatus = 0;
    
    //first, open the file, setting it to the lowest unused file descriptor
    int fileDescriptor = open(fileName, permission);
    
    if (fileDescriptor < 0) //if an error occurred opening the file
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when trying to open the following file name: %s\n", fileName);
        openFileReturnStatus = 1;
    }
    
    //then, add the file descriptor to our array of file descriptors
    //if the open was successful, the file descriptor of 'fileName' will be added to the array
    //if the open failed, then -1 will be added to the array
    fileDescriptors[position] = fileDescriptor;
    
    int * largerFileDescriptorArray = realloc(fileDescriptors, (position + 2) * sizeof(int));
    
    if (largerFileDescriptorArray == NULL)
    {
        free(fileDescriptors);
        fprintf(stderr, "Error allocating more memory.");
        exit(1);
    }
    
    fileDescriptors = largerFileDescriptorArray;
    return openFileReturnStatus;
}

int main(int argc, char **argv)
{
    
    //FIGURE OUT HOW TO HANDLE EXITING
    int EXITSTATUS = 0;
    
    static struct option long_options[] =
    {
        {"rdonly",  required_argument, NULL, 'r'},
        {"wronly",  required_argument, NULL, 'w'},
        {"command", required_argument, NULL, 'c'},
        {"verbose", no_argument,       NULL, 'v'},
        {0,         0,                 0,      0}
    };

    //when we open a file, we put its file descriptor in the lowest available spot in the fileDescriptors array
    //if the file fails to open, we put a -1 in the array
    int * fileDescriptors = (int *) malloc(sizeof(int));
    int fileDescriptorNum = 0;
    
    int optResult;
    
    int verbose = 0;
    
    while (1)
    {
        optResult = getopt_long(argc, argv, "", long_options, NULL);
        
        //getopt_long returns -1 if all command-line options have been parsed (getopt man page)
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        switch (optResult)
        {
            case 'r':
                if (verbose)
                {
                    printf("--rdonly %s\n", optarg);
                }
                int rdOnlyReturnStatus = openFile(optarg, O_RDONLY, fileDescriptors, fileDescriptorNum);
                if (rdOnlyReturnStatus)
                {
                    EXITSTATUS = 1;
                }
                fileDescriptorNum++;
                break;
            case 'w':
                if (verbose)
                {
                    printf("--wronly %s\n", optarg);
                }
                int wrOnlyReturnStatus = openFile(optarg, O_WRONLY, fileDescriptors, fileDescriptorNum);
                if (wrOnlyReturnStatus)
                {
                    EXITSTATUS = 1;
                }
                fileDescriptorNum++;
                break;
            case 'c':
                //just to appease the compiler for now
                verbose--; verbose++;
                //create an instance of the struct
                struct commandFlagArgs cmdArgs = parseCommandArguments(argc, argv);
                int exitCode = executeCommand(cmdArgs, fileDescriptors);
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab1a --rdonly [fileName] --wronly [fileName] --command [stdin] [stdout] [stderr] [executable] [args] --verbose\n");
                EXITSTATUS = 1;
                break;
        }
    }
    
    return EXITSTATUS;
}
