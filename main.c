#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

struct jobInfo
{

    unsigned char jobType;

    char jobName[100];

    int jobPriority;
    int strideVal;
    int passVal;
};

struct jobQueue
{

    struct jobInfo job;

    char freeSlot;

    char jobBlocked;
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

    // set up local variables
    char *currentStringPos = jobStringInput;
    int whiteSpaceCount = 0;
    char copyStringBuffer[100];

    memset(&copyStringBuffer, 0, sizeof(copyStringBuffer));

    // find where the string ends either with a comman or a newline
    while (currentStringPos[whiteSpaceCount]++ != (',' || '\n'))
    {

        whiteSpaceCount++;
    }
    whiteSpaceCount++;

    // compare the full string to some consts to find what job it is and set struct accordingly
    // job values are listed at top
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
    // set up variables
    char *jobString = NULL;
    int translateJobRetVal = 0;

    errno = 0;

    // read line of file
    if (getline(jobString, NULL, jobFD) == -1)
    {

        if (!errno)
        { // check to see if eof was reached and return with job list complete code

            return 2;
        }
        // if it was errno was set then print error and return 1
        perror("error with getline(jobString,NULL,jobFD): ");

        return 1;
    }

    // translate the string into a job
    if (translateJobRetVal = translateJob(jobString, jobInfoInput))
    {
        // if there is an error print what happend and return 1
        if (translateJobRetVal == 2)
        {

            printf("error in translateJob(): expected job opcode \n");
        }

        printf("error in translateJob() \n");

        return 1;
    }

    // free the string the came from getline() since we now have it in are struct
    free(jobString);

    return 0;
}

int alphabeticalDetermine(char *stringInput1, char *stringInput2)
{

    while (1)
    {

        if (stringInput1 > stringInput2)
        {

            return 1;
        }
        else if (stringInput1 < stringInput2)
        {

            return 2;
        }

        if (stringInput1 == 0 && stringInput2 == 0)
        {

            return 0;
        }
        else if (stringInput1 == 0)
        {

            return 2;
        }
        else if (stringInput2 == 0)
        {

            return 1;
        }

        stringInput1++;
        stringInput2++;
    }
}

int getLowestJob(struct jobQueue *jobQueueInput, int queueSizeInput, struct jobInfo *jobLowest)
{

    struct jobInfo *jobInfoRet;

    jobInfoRet->passVal = 10000;

    int returnVal = 1;

    for (int i = 0; i < queueSizeInput; i++)
    {
        if (jobQueueInput[i].freeSlot && !jobQueueInput[i].jobBlocked)
        {

            returnVal = 0;

            if (jobInfoRet->passVal > jobQueueInput[i].job.passVal)
            {

                jobInfoRet = &jobQueueInput[i].job;
            }
            else if (jobInfoRet->passVal == jobQueueInput[i].job.passVal)
            {

                if (alphabeticalDetermine(jobInfoRet->jobName, jobQueueInput[i].job.jobName) == 1)
                {

                    jobInfoRet = &jobQueueInput[i].job;
                }
            }
        }
    }

    jobLowest = jobInfoRet;

    jobLowest->passVal += jobLowest->strideVal;

    return returnVal;
}

int expandQueueSize(struct jobQueue *jobQueueInput, int queueSizeInput)
{

    // ran to expand the the queueSize
    struct jobQueue *newJobQueueInput = calloc(queueSizeInput + 10, sizeof(struct jobQueue));

    if (!newJobQueueInput)
    {

        perror("error in expandQueueSize() with calloc(): ");

        return 1;
    }

    for (int i = 0; i < queueSizeInput; i++)
    {

        if (jobQueueInput[i].freeSlot)
        {

            newJobQueueInput[i].freeSlot = 1;
            newJobQueueInput[i].job = jobQueueInput[i].job;
            newJobQueueInput[i].jobBlocked = jobQueueInput[i].jobBlocked;
        }
    }

    free(jobQueueInput);

    jobQueueInput = newJobQueueInput;

    return queueSizeInput + 10;
}

void resetQueuePass(struct jobQueue *jobQueueInput, int queueSizeInput)
{

    for (int i = 0; i < queueSizeInput; i++)
    {

        jobQueueInput[i].job.passVal = jobQueueInput[i].job.strideVal;
    }
}

int addJobToQueue(struct jobQueue *jobQueueInput, int queueSizeInput, struct jobInfo newJobInput)
{

    struct jobQueue *newJobQueueInput = NULL;

    newJobInput.strideVal = 10000 / newJobInput.jobPriority;
    newJobInput.passVal = newJobInput.strideVal;

    while (1)
    {

        for (int i = 0; i < queueSizeInput; i++)
        {

            if (!jobQueueInput[i].freeSlot)
            {

                jobQueueInput[i].job.jobPriority = newJobInput.jobPriority;
                jobQueueInput[i].job.jobType = newJobInput.jobType;
                jobQueueInput[i].job.strideVal = newJobInput.strideVal;
                strcpy(jobQueueInput[i].job.jobName, newJobInput.jobName);

                jobQueueInput[i].freeSlot = 1;

                resetQueuePass(jobQueueInput, queueSizeInput);

                return queueSizeInput;
            }
        }

        if (queueSizeInput = expandQueueSize(jobQueueInput, queueSizeInput))
        {

            return 1;
        }
    }
}

void unblockJob(struct jobQueue *jobQueueInput, int queueSizeInput, struct jobInfo jobToUnblockInput){

    for (int i = 0; i < queueSizeInput; i++)
    {
        
        if(!strcmp(jobQueueInput[i].job.jobName,jobToUnblockInput.jobName)){

            jobQueueInput[i].jobBlocked = 0;

            break;

        }

    }
    

}

void blockJob(struct jobQueue *jobQueueInput, int queueSizeInput, struct jobInfo *jobToBlockInput){

    for (int i = 0; i < queueSizeInput; i++)
    {

        if (&jobQueueInput[i].job == jobToBlockInput)
        {

            jobQueueInput[i].jobBlocked = 1;
        }
    }

}

void removeFromQueue(struct jobQueue *jobQueueInput, int queueSizeInput, struct jobInfo *removedJobInput)
{

    for (int i = 0; i < queueSizeInput; i++)
    {

        if (&jobQueueInput[i].job == removedJobInput)
        {

            jobQueueInput[i].freeSlot = 0;
        }
    }
}

void listRunnables(struct jobQueue *jobQueueInput, int queueSizeInput){

    struct jobInfo *tempStoredJob = NULL;

    int anyRunnables = 0;

    for(int i = 0; i < queueSizeInput; i++){

        if(jobQueueInput[i].freeSlot && !jobQueueInput[i].jobBlocked){

            anyRunnables = 1;

            tempStoredJob = &jobQueueInput[i];

            for(int x = 0; x < queueSizeInput; x++){

                if(tempStoredJob->passVal > jobQueueInput[x].job.passVal){



                }

            }

        }

    }

}

int mainScheduler(int jobListFD)
{

    struct jobInfo currentJob;
    struct jobInfo *runningJob = NULL;

    int queueSize = 10;
    struct jobQueue *jobsQueue = calloc(10, sizeof(struct jobQueue));

    if (!jobsQueue)
    {

        perror("problem with calloc(): ");

        return 1;
    }

    memset(&currentJob, 0, sizeof(struct jobInfo));

    while (!getJob(jobListFD, &currentJob))
    {

        switch (currentJob.jobType)
        {
        case 0:

            if ((queueSize = addJobToQueue(jobsQueue, queueSize, currentJob)) == 1)
            {

                return 1;
            }

            if (!runningJob)
            {

                if (getLowestJob(jobsQueue, queueSize, runningJob))
                {

                    runningJob = NULL;
                }
            }

            printf("New job: %s added with priority: %d", currentJob.jobName, currentJob.jobPriority);

            break;
        case 1:

            if (!runningJob)
            {

                printf("Error. System is idle. \n");
            }

            removeFromQueue(jobsQueue, queueSize, runningJob);

            printf("Job: %s completed.", runningJob->jobName);

            if (getLowestJob(jobsQueue, queueSize, runningJob))
            {

                runningJob = NULL;
            }

            break;
        case 2:

            if (!runningJob)
            {

                printf("Error. System is idle. \n");
            }

            if (getLowestJob(jobsQueue, queueSize, runningJob))
            {

                runningJob = NULL;

            }else{

                printf("Job: %s scheduled.", runningJob->jobName);

            }

            break;
        case 3:

            if (!runningJob)
            {

                printf("Error. System is idle. \n");
            }

            blockJob(jobsQueue, queueSize, runningJob);

            printf("Job: %s blocked.", runningJob->jobName);

            if (getLowestJob(jobsQueue, queueSize, runningJob))
            {

                runningJob = NULL;

            }

            break;
        case 4:

            break;
        case 5:

            break;
        case 6:

            break;
        case 7:

            break;
        }
    }

    free(jobsQueue);
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