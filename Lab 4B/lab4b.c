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

const int B = 4275;
const int R0 = 100000;

mraa_aio_context temperature;
mraa_gpio_context button;
char scale = 'F';
int period = 1;
int loggingEnabled = 0;
char* logFileName;
int logFileDescriptor;

struct timeval currTime;
struct timeval lastRead;
struct tm * localTime;

float readTemperature()
{
    float tempReading = mraa_aio_read_float(temperature);
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
    fprintf(stderr, "Error: incorrect argument.\nUsage: ./lab4b --period=# --scale=(C|F) --log=logFileName\n");
    exit(1);
}

//this function is called when the button is pressed
void buttonShutdown()
{
    //output (and log) a final sample with the time and the string SHUTDOWN (instead of a temperature)
    localTime = localtime(&currTime.tv_sec);
    
    char report[15]; 
    sprintf(report, "%02d:%02d:%02d SHUTDOWN\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

    printf("%s", report);
    fflush(stdout);

    if (loggingEnabled)
    {
        ssize_t bytesWritten = write(logFileDescriptor, report, strlen(report));

        if (bytesWritten < 0)
        {
            fprintf(stderr, "Error appending report to log file.\n");
            exit(1);
        }
    }

    //SHOULD THIS GO HERE?
    //close temperature sensor and button
    //mraa_aio_close(temperature);
    //mraa_gpio_close(button);

    exit(0);
}

void printSample()
{
    //sample temperature
    float temp = readTemperature();

    //get time of the sample in the local timezone
    localTime = localtime(&currTime.tv_sec);
    
    //create an outgoing buffer for the report
    char report[15]; 
    sprintf(report, "%02d:%02d:%02d %.1f\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, temp);

    //push the buffer to stdout
    printf("%s", report);
    fflush(stdout);

    //if logging has been enabled, append the report to log file
    if (loggingEnabled)
    {
        ssize_t bytesWritten = write(logFileDescriptor, report, strlen(report));

        if (bytesWritten < 0)
        {
            fprintf(stderr, "Error appending report to log file.\n");
            exit(1);
        }
    }
}

void processCommand(char * cmd)
{
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
        //do something
    }
    else if (strcmp(cmd, "START") == 0)
    {
        //do something
    }
    else if (strcmp(cmd, "OFF") == 0)
    {
        //do something
    }
    //LOG ?
}

int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
        {"period", required_argument, NULL, 'p'},
        {"scale",  required_argument, NULL, 's'},
        {"log",    required_argument, NULL, 'l'},
        {0,        0,                 0,      0}
    };
    
    int optResult;
    
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
            case '?':
                printUsage();
                break;
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
    
    //initialize temperature sensor and button
    temperature = mraa_aio_init(1); //1 is the I/O pin which refers to the analog A0/A1 connector
    if (temperature == NULL)
    {
        printf("fail temp\n");
    }
    
    button = mraa_gpio_init(60);    //60 is the I/O pin which refers to the GPIO_50 connector
    if (button == NULL)
    {
        printf("fail button\n");
    }
    
    //configure button GPIO interface to be an input pin
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    
    //when the button is pressed, call buttonShutdown
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &buttonShutdown, NULL);
    
    
    struct pollfd fds[1];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    
    int pret;
    int timeout;
    
    gettimeofday(&lastRead, NULL);

    while (1)
    {
        gettimeofday(&currTime, NULL);
        //if enough time has passed since the last read, 
        if (currTime.tv_sec >= lastRead.tv_sec + period)
        {
            printSample();
            gettimeofday(&lastRead, NULL); //update lastRead to be the time of this read
        }



        /*
        pret = poll(fds, 1, 0);
        
        if (pret == 0)
        {
            //print temp and time
            
        }
        else
        {
            //read
            //call a processCommand function
            
        }
        */
    }
    
    
    
    
    //close temperature sensor and button
    mraa_aio_close(temperature);
    mraa_gpio_close(button);
    
    exit(0);
}
