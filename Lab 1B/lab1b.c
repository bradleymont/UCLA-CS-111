// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <unistd.h> //for close(2), dup2(2), execvp(3), fork(2), getopt_long(3), pipe(2)
#include <getopt.h> //for getopt_long(3)
#include <fcntl.h> //for open(2)
#include <signal.h> //for sigaction(2)
#include <stdio.h> //for fprintf(3), stderror(3)
#include <stdlib.h> //for malloc,exit(2)
#include <string.h> //for stderror(3), strcpy
#include <errno.h> //to get errno for error messages
#include <sys/wait.h> //for wait(2)

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

int openFile(char* fileName, int permission, int** FDs, int position)
{
    int openFileReturnStatus = 0;
    
    //first, open the file, setting it to the lowest unused file descriptor
    //the 3rd parameter, mode, specifies the permissions to use in case a new file is created
    int fileDescriptor = open(fileName, permission, 0666);
    
    if (fileDescriptor < 0) //if an error occurred opening the file
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when trying to open the following file name: %s\n", fileName);
        fflush(stderr);
        openFileReturnStatus = 1;
    }
    
    //if the open was successful, the file descriptor of 'fileName' will be added to the array
    //if the open failed, then -1 will be added to the array
    (*FDs)[position] = fileDescriptor;
    *FDs = realloc(*FDs, (position + 2) * sizeof(int));
    
    return openFileReturnStatus;
}

int openPipe(int **FDs, int position)
{
    int pipeReturnStatus = 0;
    
    int pipefd[2];
    
    //pipe stores the file descriptor of the read end of the pipe in pipefd[0] and the file descriptor of the write end of the pipe in pipefd[1]
    //0 is returned on success, and -1 is returned on error
    //pipe(2) does not modify pipefd on failure
    int pipeSuccess = pipe(pipefd);
    
    if (pipeSuccess < 0) //if an error occurred opening the file
    {
        //errno contains the error number
        //strerror then takes the error number as a parameter, and then returns a string describing the error
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error caused when trying to create a pipe\n");
        fflush(stderr);
        
        pipefd[0] = -1;
        pipefd[1] = -1;
        
        pipeReturnStatus = 1;
    }
    
    //if the pipe was successful, the file descriptor for the read end of the pipe and the write end of the pipe will be added to the FD array
    //if the open failed, then -1 will be added to the array (for both the read and write ends of the pipe)
    
    //add the read end of the file descriptor to the array
    (*FDs)[position] = pipefd[0];
    
    *FDs = realloc(*FDs, (position + 4) * sizeof(int));
    
    (*FDs)[position + 1] = pipefd[1];
    
    return pipeReturnStatus;
}

//int checkFileDescriptors(int in, int out, int err, int fileDescriptorNum)
int checkFileDescriptors(int in, int out, int err, int fileDescriptorNum, int * FDs)
{
    int valid = 1;
    if (in < 0 || in >= fileDescriptorNum)
    {
        fprintf(stderr, "Error: invalid file descriptor passed to --command for stdin: %d\n", in);
        fflush(stderr);
        valid = 0;
    }
    else if (FDs[in] == -1)
    {
        fprintf(stderr, "Error: the file descriptor passed to --command for stdin (%d) contains a file that failed to open or has been closed.\n", in);
        fflush(stderr);
        valid = 0;
    }
    
    if (out < 0 || out >= fileDescriptorNum)
    {
        fprintf(stderr, "Error: invalid file descriptor passed to --command for stdout: %d\n", out);
        fflush(stderr);
        valid = 0;
    }
    else if (FDs[out] == -1)
    {
        fprintf(stderr, "Error: the file descriptor passed to --command for stdout (%d) contains a file that failed to open or has been closed.\n", out);
        fflush(stderr);
        valid = 0;
    }
    
    if (err < 0 || err >= fileDescriptorNum)
    {
        fprintf(stderr, "Error: invalid file descriptor passed to --command for stderr: %d\n", err);
        fflush(stderr);
        valid = 0;
    }
    else if (FDs[err] == -1)
    {
        fprintf(stderr, "Error: the file descriptor passed to --command for stderr (%d) contains a file that failed to open or has been closed.\n", err);
        fflush(stderr);
        valid = 0;
    }
    
    return valid;
}

struct commandFlagArgs
{
    int in, out, err;
    char* cmd;
    char** args;
    int numArgs;
    int validArgs;
};

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

struct commandFlagArgs parseCommandArguments(int argc, char **argv)
{
    struct commandFlagArgs result;
    
    result.validArgs = 1;
    
    optind--;
    
    int count = 0;
    int cmdArgCount = 0;
    
    int optindAtFirstCmdArg = -1;
    
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
                result.cmd = currArg;
                break;
            case 4:
                optindAtFirstCmdArg = optind;
                cmdArgCount++;
                break;
            default: //one of cmd's arguments
                cmdArgCount++;
        }
        
        optind++;
        count++;
    }
    
    if (count < 4)  //invalid arguments passed
    {
        result.validArgs = 0;
    }
    
    //if any arguments were passed to the --command flag
    if (cmdArgCount > 0)
    {
        result.args = (char **) malloc(cmdArgCount * sizeof(char *));
        
        optind = optindAtFirstCmdArg;   //reset optind to the first command argument
        
        int pos = 0;
        while (optind < argc)
        {
            char* currArg = argv[optind];
            
            //check to see if the currArg starts with two dashes. If so, then break
            if (startsWithTwoDashes(currArg))
            {
                break;
            }
            
            result.args[pos] = currArg;
            optind++;
            pos++;
        }
    }
    
    result.numArgs = cmdArgCount;
    
    return result;
}

struct childProcess
{
    int PID;
    char* commandPlusArgs;
};

struct childProcess executeCommand(struct commandFlagArgs cmdArgs, int * FDarray, int numDescriptors)
{
    int childPID = fork();
    
    if (childPID < 0)    //the fork failed
    {
        fprintf(stderr, "Error: fork command failed.\n");
        fflush(stderr);
        exit(1);
    }
    else if (childPID == 0)  //child process
    {
        //IO REDIRECTION
        
        //update stdin
        int newStdin = FDarray[cmdArgs.in];
        close(0);
        dup2(newStdin, 0);
        
        //update stdout
        int newStdout = FDarray[cmdArgs.out];
        close(1);
        dup2(newStdout, 1);
        
        //update stderr
        int newStderr = FDarray[cmdArgs.err];
        close(2);
        dup2(newStderr, 2);
        
        //close all file descriptors besides the 3 we are using for the command
        for (int i = 0; i < numDescriptors; i++)
        {
            int fd = FDarray[i];
            if (fd != -1)
            {
                close(fd);
            }
        }
        
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
       
    }
    
    
    
    
    
    
    //////////
    struct childProcess result;
    result.PID = childPID;

    //form string
    int cStringLength = strlen(cmdArgs.cmd) + 1;  //+1 to make room for null byte to terminate the string
    
    
    for (int i = 0; i < cmdArgs.numArgs; i++)
    {
        cStringLength += strlen(cmdArgs.args[i]) + 1; //+1 for the space before it
    }
    
    char* argsString = malloc(cStringLength);
    
    //strcat(argsString, cmdArgs.cmd);
    strcpy(argsString, cmdArgs.cmd);
    
    for (int i = 0; i < cmdArgs.numArgs; i++)
    {
        strcat(argsString, " ");
        strcat(argsString, cmdArgs.args[i]);
    }
    
    result.commandPlusArgs = argsString;
    
    
    return result;
}

void segFaultHandler(int signum)
{
    fprintf(stderr, "Signal %d caught.\n", signum);
    fflush(stderr);
    exit(signum);
}

int main(int argc, char **argv)
{
    int EXITSTATUS = 0;
    
    //array of file flags
    static int fileFlags[11] = {0};

    //each index represents the following:
    //0: --append
    //1: --cloexec
    //2: --creat
    //3: --directory
    //4: --dsync
    //5: --excl
    //6: --nofollow
    //7: --nonblock
    //8: --rsync
    //9: --sync
    //10: --trunc
    
    static struct option long_options[] =
    {
        {"rdonly",    required_argument, NULL,           'r'},
        {"wronly",    required_argument, NULL,           'w'},
        {"rdwr",      required_argument, NULL,           'b'},  //b for both reading and writing
        {"pipe",      no_argument,       NULL,           'p'},
        {"command",   required_argument, NULL,           'c'},
        {"wait",       no_argument,      NULL,           't'},
        {"verbose",   no_argument,       NULL,           'v'},
        {"abort",     no_argument,       NULL,           'a'},
        {"catch",     required_argument, NULL,           'h'},
        {"ignore",    required_argument, NULL,           'i'},
        {"default",   required_argument, NULL,           'd'},
        {"pause",     no_argument,       NULL,           'u'},
        {"close",     required_argument, NULL,           'x'},
        {"append",    no_argument,       &fileFlags[0],  O_APPEND},
        {"cloexec",   no_argument,       &fileFlags[1],  O_CLOEXEC},
        {"creat",     no_argument,       &fileFlags[2],  O_CREAT},
        {"directory", no_argument,       &fileFlags[3],  O_DIRECTORY},
        {"dsync",     no_argument,       &fileFlags[4],  O_DSYNC},
        {"excl",      no_argument,       &fileFlags[5],  O_EXCL},
        {"nofollow",  no_argument,       &fileFlags[6],  O_NOFOLLOW},
        {"nonblock",  no_argument,       &fileFlags[7],  O_NONBLOCK},
        {"rsync",     no_argument,       &fileFlags[8],  O_RSYNC},
        {"sync",      no_argument,       &fileFlags[9],  O_SYNC},
        {"trunc",     no_argument,       &fileFlags[10], O_TRUNC},
        {0,           0,                 0,              0}
    };

    //when we open a file, we put its file descriptor in the lowest available spot in the fileDescriptors array
    //if the file fails to open, we put a -1 in the array
    int * fileDescriptors = (int *) malloc(sizeof(int));
 
    int fileDescriptorNum = 0;
    
    int optResult;
    
    int verbose = 0;
    
    int filePermission;
    
    struct childProcess * children = (struct childProcess *) malloc(sizeof(struct childProcess));
    
    int numChildren = 0;
    
    while (1)
    {
        optResult = getopt_long(argc, argv, "", long_options, NULL);
        
        //getopt_long returns -1 if all command-line options have been parsed (getopt man page)
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        //if the user passed the --verbose flag, output each option
        if (verbose)
        {
            switch (optResult)
            {
                case   0:
                case 'p':
                case 'u':
                case 'a':
                case 't':
                    write(1, argv[optind - 1], strlen(argv[optind - 1]));
                    write(1, "\n", 1);
                    break;
                case 'r':
                case 'w':
                case 'b':
                case 'h':
                case 'i':
                case 'd':
                case 'x':
                    write(1, argv[optind - 2], strlen(argv[optind - 2]));
                    write(1, " ", 1);
                    write(1, optarg, strlen(optarg));
                    write(1, "\n", 1);
                    break;
            }
        }
        
        switch (optResult)
        {
            case 'r':
                filePermission = O_RDONLY;
                
                for (int i = 0; i < 11; i++)    //apply any file flags currently activated
                {
                    filePermission |= fileFlags[i];
                }
                
                int rdOnlyReturnStatus = openFile(optarg, filePermission, &fileDescriptors, fileDescriptorNum);
                
                if (rdOnlyReturnStatus)
                {
                    EXITSTATUS = max(EXITSTATUS, 1);
                }
                fileDescriptorNum++;
                
                //reset file flags
                for (int i = 0; i < 11; i++)
                {
                    fileFlags[i] = 0;
                }
                
                break;
            case 'w':
                filePermission = O_WRONLY;
                
                for (int i = 0; i < 11; i++) //apply any file flags currently activated
                {
                    filePermission |= fileFlags[i];
                }
                
                int wrOnlyReturnStatus = openFile(optarg, filePermission, &fileDescriptors, fileDescriptorNum);
                
                if (wrOnlyReturnStatus)
                {
                    EXITSTATUS = max(EXITSTATUS, 1);
                }
                fileDescriptorNum++;
                
                //reset file flags
                for (int i = 0; i < 11; i++)
                {
                    fileFlags[i] = 0;
                }
                
                break;
            case 'b':
                
                filePermission = O_RDWR;
                
                for (int i = 0; i < 11; i++) //apply any file flags currently activated
                {
                    filePermission |= fileFlags[i];
                }
                
                int rdwrReturnStatus = openFile(optarg, filePermission, &fileDescriptors, fileDescriptorNum);
                
                if (rdwrReturnStatus)
                {
                    EXITSTATUS = max(EXITSTATUS, 1);
                }
                fileDescriptorNum++;
                
                //reset file flags
                for (int i = 0; i < 11; i++)
                {
                    fileFlags[i] = 0;
                }
                
                break;
            case 'p':
                ;   //empty statement because you can't have declaration after a label in C
                
                int pipeReturnStatus = openPipe(&fileDescriptors, fileDescriptorNum);
                
                if (pipeReturnStatus)
                {
                    EXITSTATUS = max(EXITSTATUS, 1);
                }
                
                fileDescriptorNum += 2;
                break;
            case 'c':
                ;   //empty statement because you can't have declaration after a label in C
                //create an instance of the struct
                struct commandFlagArgs cmdArgs = parseCommandArguments(argc, argv);
                
                if (verbose)
                {
                    printf("--command %d %d %d %s", cmdArgs.in, cmdArgs.out, cmdArgs.err, cmdArgs.cmd);
                    fflush(stdout);
                    
                    for (int i = 0; i < cmdArgs.numArgs; i++)
                    {
                        printf(" %s", cmdArgs.args[i]);
                        fflush(stdout);
                        
                    }
                    printf("\n");
                    fflush(stdout);
                }
                
                if (cmdArgs.validArgs == 0) //if not enough arguments were to passed to --command
                {
                    fprintf(stderr, "Error: missing arguments for --command flag.\nUsage: --command [stdin] [stdout] [stderr] [command] [args]\n");
                    fflush(stderr);
                    EXITSTATUS = max(EXITSTATUS, 1);
                    break;
                }

                //check that valid ints were passed for the file descriptors
                //the fd should be >= 0 and < fileDescriptorNum
                //int validFdsPassed = checkFileDescriptors(cmdArgs.in, cmdArgs.out, cmdArgs.err, fileDescriptorNum);
                int validFdsPassed = checkFileDescriptors(cmdArgs.in, cmdArgs.out, cmdArgs.err, fileDescriptorNum, fileDescriptors);
                
                if (validFdsPassed)
                {
                    //executeCommand(cmdArgs, fileDescriptors, fileDescriptorNum);
                    struct childProcess child = executeCommand(cmdArgs, fileDescriptors, fileDescriptorNum);
                    
                    children[numChildren] = child;
                    children = realloc(children, (numChildren + 2) * sizeof(struct childProcess));
                    numChildren++;
                }
                else  //an invalid file descriptor number passed
                {
                    EXITSTATUS = max(EXITSTATUS, 1);
                }
                
                //free memory
                if (cmdArgs.numArgs > 0)
                {
                    free(cmdArgs.args);
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case 'a':
                ;   //empty statement because you can't have declaration after a label in C
                char* will_cause_segfault = NULL;
                *will_cause_segfault = 'f'; //dereferencing a null pointer will cause a segmentation fault
                break;
            case 'h': //--catch
                signal(atoi(optarg), segFaultHandler);
                break;
            case 'i':
                signal(atoi(optarg), SIG_IGN);
                break;
            case 'd':
                signal(atoi(optarg), SIG_DFL);
                break;
            case 'u':   //pause
                pause();
                break;
            case 'x':
                ;   //empty statement because you can't have declaration after a label in C
                int fdToClose = fileDescriptors[atoi(optarg)];
                close(fdToClose);
                fileDescriptors[atoi(optarg)] = -1;
                break;
            case 't': //wait
                while (1)
                {
                    int status;
                    int childPID = wait(&status);
                    
                    if (childPID == -1)   //did not successfully wait for a child process
                    {
                        break;
                    }
                    
                    
                    //linear search for which process in my array matches the childPID
                    int childIndex = -1;
                    
                    for (int i = 0; i < numChildren; i++)
                    {
                        if (childPID == children[i].PID)
                        {
                            childIndex = i;
                            break;
                        }
                    }
                    
                    
                    
                    if (WIFEXITED(status))  //if the child terminated normally
                    {
                        printf("exit %d %s\n", WEXITSTATUS(status), children[childIndex].commandPlusArgs);
                        fflush(stdout);
                        
                        
                        EXITSTATUS = max(EXITSTATUS, WEXITSTATUS(status));
                        
                    }
                    
                    if (WIFSIGNALED(status))  //if the child exited with a signal
                    {
                        printf("signal %d %s\n", WTERMSIG(status), children[childIndex].commandPlusArgs);
                        fflush(stdout);
                        
                        EXITSTATUS = max(EXITSTATUS, WTERMSIG(status));
                    }
                    
         
                }
                
                //reset children array
                for (int i = 0; i < numChildren; i++)
                {
                    free(children[i].commandPlusArgs);
                }
                free(children);
                
                children = (struct childProcess *) malloc(sizeof(struct childProcess));
                numChildren = 0;
                
                
                break;
            case '?':
                fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab1a --rdonly [fileName] --wronly [fileName] --command [stdin] [stdout] [stderr] [executable] [args] --verbose\n");
                fflush(stderr);
                EXITSTATUS = max(EXITSTATUS, 1);
                break;
        }
    }
    
    //free dynamically allocated memory
    free(fileDescriptors);
    free(children);
    
    return EXITSTATUS;
}
