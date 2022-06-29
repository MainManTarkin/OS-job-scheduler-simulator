#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct jobInfo
{

    unsigned char jobType;
    char jobName[100];
    int jobPriority;
};

// handle incoming argument vector
int handleArgs(int argcInput, char **argvInput, char *inputFilePath)
{

    char getOptReturn = 0;

    while ((getOptReturn = getopt(argcInput, argvInput, "i:")) != -1)
    {

        switch (getOptReturn)
        {
        case 'i':

            // get input path for file
            inputFilePath = optarg;
            break;

        default:

            return 1;
            break;
        }
    }

    return 0;
}

int translateJob(char *jobStringInput, struct jobInfo *jobInfoInput)
{
    /*
        ===Job Values===
        0 = newjob
        1 = finish
        2 = interrupt
        3 = block
        4 = unblock
        5 = runnable
        6 = running
        7 = blocked


    */

    //set up local variables
    char *currentStringPos = jobStringInput;
    int whiteSpaceCount = 0;
    char copyStringBuffer[100];

    memset(&copyStringBuffer, 0, sizeof(copyStringBuffer));


    //find where the string ends either with a comman or a newline 
    while (currentStringPos[whiteSpaceCount]++ != (',' || '\n') )
    {

        whiteSpaceCount++;
    }
    whiteSpaceCount++;

    //compare the full string to some consts to find what job it is and set struct accordingly
    //job values are listed at top
    if (!strncmp(jobStringInput, "newjob", whiteSpaceCount))
    {

        jobInfoInput->jobType = 0;

        while (currentStringPos[whiteSpaceCount]++ != ',')
        {

            whiteSpaceCount++;
        }

        strncpy(jobInfoInput->jobName, currentStringPos, whiteSpaceCount);

        while (currentStringPos[whiteSpaceCount]++ != '\n')
        {

            whiteSpaceCount++;
        }

        strncpy(&copyStringBuffer, currentStringPos, whiteSpaceCount);

        if (jobInfoInput->jobPriority = atoi(&copyStringBuffer))
        {

            perror("error with atoi(&copyStringBuffer): ");

            return 1;
        }

        return 0;
    }
    else if (!strncmp(jobStringInput, "finish", whiteSpaceCount))
    {

        jobInfoInput->jobType = 1;
        return 0;
    }
    else if (!strncmp(jobStringInput, "interrupt", whiteSpaceCount))
    {

        jobInfoInput->jobType = 2;
        return 0;
    }
    else if (!strncmp(jobStringInput, "block", whiteSpaceCount))
    {

        jobInfoInput->jobType = 3;
        return 0;
    }
    else if (!strncmp(jobStringInput, "unblock", whiteSpaceCount))
    {

        jobInfoInput->jobType = 4;

        while (currentStringPos[whiteSpaceCount]++ != '\n')
        {

            whiteSpaceCount++;
        }

        strncpy(jobInfoInput->jobName, currentStringPos, whiteSpaceCount);
        return 0;
    }
    else if (!strncmp(jobStringInput, "runnable", whiteSpaceCount))
    {

        jobInfoInput->jobType = 5;
        return 0;
    }
    else if (!strncmp(jobStringInput, "running", whiteSpaceCount))
    {

        jobInfoInput->jobType = 6;
        return 0;
    }
    else if (!strncmp(jobStringInput, "blocked", whiteSpaceCount))
    {

        jobInfoInput->jobType = 7;
        return 0;
    }

    return 2;
}

int getJob(int jobFD, struct jobInfo *jobInfoInput)
{
    //set up variables
    char *jobString = NULL;
    int translateJobRetVal = 0;

    // read line of file
    if (getline(jobString, NULL, jobFD) == -1)
    {

        perror("error with getline(jobString,NULL,jobFD): ");

        return 1;
    }

    //translate the string into a job
    if(translateJobRetVal = translateJob(jobString,jobInfoInput)){
        //if there is an error print what happend and return 1
        if(translateJobRetVal == 2){

            printf("error in translateJob(): expected job opcode \n");

        }

        printf("error in translateJob() \n");

        return 1;

    }

    // free the string the came from getline() since we now have it in are struct
    free(jobString);


    return 0;
}

int main(int argc, char *argv[])
{

    char schecdularJobFileInputPath = 0;

    int schecdularJobFileDesciptor = 0;

    // setup program by getting input file argument and opening the file
    if (handleArgs(argc, argv, &schecdularJobFileInputPath))
    {

        printf("error in handle args: invaild argument given \n");

        return 1;
    }

    if ((schecdularJobFileDesciptor = open(schecdularJobFileInputPath, O_RDONLY)) == -1)
    {

        perror("error in open(schecdularJobFileInputPath,O_RDONLY): ");

        return 1;
    }
    // finish setup start performing scheduling

    return 0;
}