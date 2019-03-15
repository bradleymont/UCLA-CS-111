// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include <stdio.h>  //for fprintf(3)
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi
#include <string.h> //for atoi
#include <mraa.h>
#include <poll.h> //for poll
#include <math.h> //for log
#include <sys/time.h> //gettimeofday(2)
#include <time.h> //localtime()
#include <sys/types.h> //open(2)
#include <sys/stat.h> //open(2)
#include <fcntl.h> //open(2)
#include <unistd.h> //write(2)
#include <sys/socket.h> //for socket(2), connect(2)
#include <netdb.h> //for gethostbyname(3)

const int B = 4275;
const int R0 = 100000;

mraa_aio_context temperature;
char scale = 'F';
int period = 1;
int loggingEnabled = 0;
char* logFileName;
int logFileDescriptor;
int generateReports = 1;

char* id;
char* host;
int portNumber;
int socketFD;
struct hostent* server;
struct sockaddr_in serverAddress;

struct timeval currTime;
struct timeval lastRead;
struct tm * localTime;

void Exit(int status)
{
    //close temperature sensor
    mraa_aio_close(temperature);
    exit(status);
}

void Write(int fd, const void *buf, size_t count)
{
    ssize_t bytesWritten = write(fd, buf, count);
    
    if (bytesWritten < 0)
    {
        fprintf(stderr, "Error writing.\n");
        Exit(2);
    }
}

float readTemperature()
{
    float tempReading = mraa_aio_read(temperature);
    float R = 1023.0 / tempReading - 1.0;
    R = R0 * R;
    float currTemp = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
    if (scale == 'F')
    {
        return (currTemp * 9 / 5) + 32;
    }
    else //Celsius
    {
        return currTemp;
    }
}

void printUsage()
{
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab4c_tcp --period=# --scale=(C|F) --log=logFileName --id=# --host=(name|address) portNumber\n");
    exit(1);
}

//this function is called when the button is pressed
void shutdownDevice()
{
    //output (and log) a final sample with the time and the string SHUTDOWN (instead of a temperature)
    localTime = localtime(&currTime.tv_sec);
    
    char report[30]; 
    sprintf(report, "%02d:%02d:%02d SHUTDOWN\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
    
    printf("%s", report);
    fflush(stdout);

    if (loggingEnabled)
    {
        Write(logFileDescriptor, report, strlen(report));
    }
    //send report over the connection
    if (socketFD >= 0)
    {
        Write(socketFD, report, strlen(report));
    }
    Exit(0);
}

void printSample()
{
    //sample temperature
    float temp = readTemperature();

    //get time of the sample in the local timezone
    localTime = localtime(&currTime.tv_sec);
    
    //create an outgoing buffer for the report
    char report[20];
    sprintf(report, "%02d:%02d:%02d %.1f\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, temp);

    //push the buffer to stdout
    printf("%s", report);
    fflush(stdout);

    //if logging has been enabled, append the report to log file
    if (loggingEnabled)
    {
        Write(logFileDescriptor, report, strlen(report));
    }
    //send report over the connection
    if (socketFD >= 0)
    {
        Write(socketFD, report, strlen(report));
    }
}

void processCommand(char * cmd)
{
    if (loggingEnabled)
    {
        Write(logFileDescriptor, cmd, strlen(cmd));
        Write(logFileDescriptor, "\n", 1);
    }
    
    if (strcmp(cmd, "SCALE=F") == 0)
    {
        scale = 'F';
    }
    else if (strcmp(cmd, "SCALE=C") == 0)
    {
        scale = 'C';
    }
    else if (strncmp(cmd, "PERIOD=", strlen("PERIOD=")) == 0)
    {
        period = atoi(cmd + 7);
    }
    else if (strcmp(cmd, "STOP") == 0)
    {
        generateReports = 0;
    }
    else if (strcmp(cmd, "START") == 0)
    {
        generateReports = 1;
    }
    else if (strcmp(cmd, "OFF") == 0)
    {
        shutdownDevice();
    }
}

int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
        {"period", required_argument, NULL, 'p'},
        {"scale",  required_argument, NULL, 's'},
        {"log",    required_argument, NULL, 'l'},
        {"id",     required_argument, NULL, 'i'},
        {"host",   required_argument, NULL, 'h'},
        {0,        0,                 0,      0}
    };
    
    int optResult;
    int idFlag = 0;
    int hostFlag = 0;
    
    while (1)
    {
        optResult = getopt_long(argc, argv, "", long_options, NULL);
        
        if (optResult == -1) break; //break out of the loop after parsing all arguments
        
        switch (optResult)
        {
            case 'p':
                period = atoi(optarg);
                break;
            case 's':
                scale = optarg[0];
                if (scale != 'F' && scale != 'C')
                {
                    printUsage();
                }
                break;
            case 'l':
                logFileName = optarg;
                loggingEnabled = 1;
                break;
            case 'i':
                id = optarg;
                idFlag = 1;
                break;
            case 'h':
                host = optarg;
                hostFlag = 1;
                break;
            case '?':
                printUsage();
                break;
        }
    }
    
    //check for mandatory parameters
    if ( (optind == argc) | !idFlag | !hostFlag | !loggingEnabled)
    {
        fprintf(stderr, "Error: Missing mandatory parameter(s).\n");
        printUsage();
    }
    
    //parse port number input
    if (optind < argc)
    {
        portNumber = atoi(argv[optind]);
        if (portNumber <= 1024)
        {
            fprintf(stderr, "Error: Invalid port number.\n");
            exit(1);
        }
    }

    if (loggingEnabled)
    {
        logFileDescriptor = open(logFileName, O_WRONLY|O_CREAT|O_TRUNC, 0666);

        if (logFileDescriptor < 0)
        {
            fprintf(stderr, "Error creating log file.\n");
            exit(1);
        }
    }
    
    //1. open a TCP connection to the server at the specified address and port
    
    //use socket to create an endpoint for communication
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketFD < 0)
    {
        fprintf(stderr, "Error creating socket.\n");
        exit(2);
    }
    
    //identify server
    server = gethostbyname(host);
    
    if (server == NULL)
    {
        fprintf(stderr, "Error finding host.\n");
        exit(2);
    }
    
    //connect to remote host
    bzero((char *) &serverAddress, sizeof(serverAddress));
    bcopy((char *) (server->h_addr), (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    
    int connectReturn = connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    
    if (connectReturn < 0)
    {
        fprintf(stderr, "Error connecting to host.\n");
        exit(2);
    }
    
    //2. immediately send (and log) an ID terminated with a newline: ID=ID-number
    char IDlog[15] = "ID=";
    strcat(IDlog, id);
    strcat(IDlog, "\n");
    
    printf("%s", IDlog);
    Write(logFileDescriptor, IDlog, strlen(IDlog)); //log
    Write(socketFD, IDlog, strlen(IDlog)); //send to server
    
    //initialize temperature sensor and button
    temperature = mraa_aio_init(1); //1 is the I/O pin which refers to the analog A0/A1 connector
    if (temperature == NULL)
    {
        fprintf(stderr, "Error initializing temperature sensor.\n");
        mraa_deinit();
        exit(2);
    }
    
    struct pollfd pollServerInput;
    pollServerInput.fd = socketFD;
    pollServerInput.events = POLLIN;
    
    gettimeofday(&lastRead, NULL);
    
    char cmdBuffer[128];
    char cmdBufferCopy[128];
    memset(cmdBuffer, 0, 128);
    memset(cmdBufferCopy, 0, 128);
    int index = 0;
    
    while (1)
    {
        gettimeofday(&currTime, NULL);
        //if enough time has passed since the last read, 
        if ( (currTime.tv_sec >= lastRead.tv_sec + period) && generateReports)
        {
            printSample();
            gettimeofday(&lastRead, NULL); //update lastRead to be the time of this read
        }

        //2nd parameter is # of file descriptors to poll, 3rd parameter is timeout (0 in this case)
        int pollRet = poll(&pollServerInput, 1, 0);
        
        if (pollRet < 0)
        {
            fprintf(stderr, "Error polling for data to read.\n");
            Exit(1);
        }
        else if (pollServerInput.revents & POLLIN) //there is data to read from the server
        {
            ssize_t bytesRead = read(socketFD, cmdBuffer, 128);
            
            if (bytesRead < 0)
            {
                fprintf(stderr, "Error reading in standard input.\n");
                Exit(1);
            }
            
            int i;
            for (i = 0; i < bytesRead && index < 128; i++)
            {
                if (cmdBuffer[i] == '\n')
                {
                    processCommand( (char *) &cmdBufferCopy);
                    memset(cmdBufferCopy, 0, 128);
                    index = 0;
                }
                else
                {
                    cmdBufferCopy[index] = cmdBuffer[i];
                    index++;
                }
            }
        }
    }
    
    Exit(0);
}
