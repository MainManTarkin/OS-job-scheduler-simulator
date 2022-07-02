#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

struct argStruct{

    char *inputFIleDest;

};

struct jobInfo
{

    unsigned char jobType;

    char jobName[100];

    int jobPriority;
};

struct jobState
{

    char jobName[100];

    int jobPriority;

    unsigned int passLevel;
    unsigned int stride;

    char blocked;
};

struct jobQueue
{

    struct jobState *theJob;

    struct jobQueue *nextLink;
    struct jobQueue *backLink;
};

// handle incoming argument vector
int handleArgs(int argcInput, char **argvInput, struct argStruct *inputFilePath)
{

    int getOptReturn = 0;

    while ((getOptReturn = getopt(argcInput, argvInput, "i:")) != -1)
    {

        switch (getOptReturn)
        {
        case 'i':

            // get input path for file
            inputFilePath->inputFIleDest = optarg;
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
    int baseCount = 0;
    int whiteSpaceCount = 0;
    char copyStringBuffer[100];

    memset(&copyStringBuffer, 0, sizeof(copyStringBuffer));

    // find where the string ends either with a comman or a newline
    while(1){

        if(jobStringInput[whiteSpaceCount] != ','  && jobStringInput[whiteSpaceCount] != '\n' && jobStringInput[whiteSpaceCount] != 0){

            whiteSpaceCount++;

        }else{

            break;

        }

    }
    

    // compare the full string to some consts to find what job it is and set struct accordingly
    // job values are listed at top
    if (!strncmp(jobStringInput, "newjob", whiteSpaceCount))
    {

        jobInfoInput->jobType = 0;
        baseCount = ++whiteSpaceCount;
        while (1)
        {

            if(jobStringInput[whiteSpaceCount] != ','){

                whiteSpaceCount++;
                
            }else{

                break;

            }
            
        }

        strncpy(jobInfoInput->jobName, jobStringInput + baseCount, whiteSpaceCount - baseCount);

        baseCount = ++whiteSpaceCount;

        while (1)
        {

            if(jobStringInput[whiteSpaceCount] != 0 && jobStringInput[whiteSpaceCount] != '\n'){

                whiteSpaceCount++;
                
            }else{

                break;

            }
            
            
        }

        strncpy(copyStringBuffer, jobStringInput + baseCount, whiteSpaceCount - baseCount);

        if (!(jobInfoInput->jobPriority = atoi(copyStringBuffer)))
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

        baseCount = ++whiteSpaceCount;
        while (1)
        {

            if(jobStringInput[whiteSpaceCount] != 0 && jobStringInput[whiteSpaceCount] != '\n'){

                whiteSpaceCount++;
                
            }else{

                break;

            }
            
            
        }

        strncpy(jobInfoInput->jobName, jobStringInput+baseCount, whiteSpaceCount - baseCount);
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

int getJob(FILE *fileFDInput, struct jobInfo *jobInfoInput)
{
    // set up variables
    size_t jobStringSize = 255;
    char *jobString = calloc(jobStringSize,sizeof(char));
    int translateJobRetVal = 0;
    ssize_t bytesRead = 0;

    errno = 0;

    // read line of file
    if ((bytesRead = getline(&jobString, &jobStringSize, fileFDInput)) == -1)
    {

        if (!errno)
        { // check to see if eof was reached and return with job list complete code

            free(jobString);
            return 2;
        }
        // if it was errno was set then print error and return 1
        perror("error with getline(jobString,NULL,jobFD): ");
        free(jobString);
        return 1;
    }

    // translate the string into a job
    if ((translateJobRetVal = translateJob(jobString, jobInfoInput)))
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

int alphabeticalOrder(struct jobState *job1, struct jobState *job2)
{

    for (int i = 0; i < 100; i++)
    {

        if (job1->jobName[i] == 0 || job2->jobName[i] == 0)
        {

            if (job1->jobName[i] == 0)
            {

                return 0;
            }
            else if (job2->jobName[i] == 0)
            {

                return 1;
            }
        }
        else
        {

            if (job1->jobName[i] < job2->jobName[i])
            {

                return 0;
            }
            else if (job1->jobName[i] > job2->jobName[i])
            {

                return 1;
            }
        }
    }

    return 0;
}

void resetQueuePassVals(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput->nextLink;

    while ((currentNode))
    {

        if (currentNode->theJob)
        {

            currentNode->theJob->passLevel = currentNode->theJob->stride;
        }
        else
        {

            break;
        }

        currentNode = currentNode->nextLink;
    }
}

int addJobToQueue(struct jobQueue *queueInput, struct jobInfo newJobInput)
{

    struct jobQueue *currentNode = queueInput;
    struct jobQueue *prevNode = queueInput;
    struct jobQueue *newNode = calloc(1, sizeof(struct jobQueue));

    if (!newNode)
    {

        perror("error with calloc() in addJobToQueue(): ");

        return 1;
    }

    newNode->theJob = calloc(1, sizeof(struct jobState));

    if (!newNode->theJob)
    {

        perror("error with calloc() in addJobToQueue(): ");

        return 1;
    }

    newNode->theJob->jobPriority = newJobInput.jobPriority;
    newNode->theJob->stride = UINT_MAX / newJobInput.jobPriority;
    newNode->theJob->passLevel = newNode->theJob->stride;
    strcpy(newNode->theJob->jobName, newJobInput.jobName);

    resetQueuePassVals(queueInput);

    while ((currentNode = currentNode->nextLink))
    {

        if (currentNode->theJob->passLevel > newNode->theJob->passLevel)
        {

            currentNode->backLink->nextLink = newNode;
            currentNode->backLink = newNode;

            newNode->backLink = currentNode->backLink;
            newNode->nextLink = currentNode;

            return 0;
        }
        else if (currentNode->theJob->passLevel == newNode->theJob->passLevel)
        {

            if (alphabeticalOrder(currentNode->theJob, newNode->theJob))
            {

                currentNode->backLink->nextLink = newNode;
                currentNode->backLink = newNode;

                newNode->backLink = currentNode->backLink;
                newNode->nextLink = currentNode;

                return 0;
            }
        }

        prevNode = currentNode;
    }

    prevNode->nextLink = newNode;

    newNode->backLink = prevNode;

    newNode->nextLink = NULL;

    return 0;
}

void reAddToQueue(struct jobQueue *queueInput, struct jobQueue *jobToReAdd)
{

    struct jobQueue *currentNode = queueInput;
    struct jobQueue *prevNode = queueInput;

    if (!currentNode->nextLink)
    {

        currentNode->nextLink = jobToReAdd;

        return;
    }

    while ((currentNode = currentNode->nextLink))
    {

        if (currentNode->theJob->passLevel > jobToReAdd->theJob->passLevel)
        {

            currentNode->backLink->nextLink = jobToReAdd;
            currentNode->backLink = jobToReAdd;

            jobToReAdd->backLink = currentNode->backLink;
            jobToReAdd->nextLink = currentNode;

            return;
        }
        else if (currentNode->theJob->passLevel == jobToReAdd->theJob->passLevel)
        {

            if (alphabeticalOrder(currentNode->theJob, jobToReAdd->theJob))
            {

                currentNode->backLink->nextLink = jobToReAdd;
                currentNode->backLink = jobToReAdd;

                jobToReAdd->backLink = currentNode->backLink;
                jobToReAdd->nextLink = currentNode;

                return;
            }
        }

        prevNode = currentNode;
    }

    prevNode->nextLink = jobToReAdd;

    jobToReAdd->backLink = prevNode;

    jobToReAdd->nextLink = NULL;
}

struct jobQueue *getMinJob(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput->nextLink;

    struct jobQueue *returnJob = NULL;

    while (currentNode)
    {

        if (!currentNode->theJob->blocked)
        {

            returnJob = currentNode;

            break;
        }

        currentNode = currentNode->nextLink;
    }

    if (returnJob->backLink)
    {

        returnJob->backLink->nextLink = returnJob->nextLink;
    }

    if (returnJob->nextLink)
    {

        returnJob->nextLink->backLink = returnJob->backLink;
    }

    returnJob->theJob->passLevel += returnJob->theJob->stride;

    return returnJob;
}

struct jobQueue *unblockJob(struct jobQueue *queueInput, struct jobInfo nameOfJobInput, int *runStatus)
{

    struct jobQueue *currentNode = queueInput;

    while ((currentNode = currentNode->nextLink))
    {

        if (!strcmp(currentNode->theJob->jobName, nameOfJobInput.jobName))
        {

            if (currentNode->theJob->blocked)
            {

                currentNode->theJob->blocked = 0;

                if (currentNode->backLink)
                {

                    currentNode->backLink->nextLink = currentNode->nextLink;
                }

                if (currentNode->nextLink)
                {

                    currentNode->nextLink->backLink = currentNode->backLink;
                }

                return currentNode;
            }
            else
            {

                *runStatus = 1;

                return NULL;
            }
        }
    }

    *runStatus = 2;

    return NULL;
}

int listBlocked(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput;

    if (!currentNode->nextLink)
    {

        return 1;
    }

    printf("Blocked: \n");
    printf("NAME    STRIDE  PASS  PRI \n");

    while ((currentNode = currentNode->nextLink))
    {

        if (currentNode->theJob->blocked)
        {

            printf("%s       %d     %d  %d \n", currentNode->theJob->jobName, currentNode->theJob->stride, currentNode->theJob->passLevel, currentNode->theJob->jobPriority);
        }
    }

    return 0;
}

int listRunnables(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput;

    if (!currentNode->nextLink)
    {

        return 1;
    }

    printf("Runnable: \n");
    printf("NAME    STRIDE  PASS  PRI \n");

    while ((currentNode = currentNode->nextLink))
    {

        if (!currentNode->theJob->blocked)
        {

            printf("%s       %d     %d  %d \n", currentNode->theJob->jobName, currentNode->theJob->stride, currentNode->theJob->passLevel, currentNode->theJob->jobPriority);
        }
    }

    return 0;
}

void freeTheQueue(struct jobQueue *queueInput){

    struct jobQueue *currentNode = queueInput->nextLink;
    struct jobQueue *prevNode = queueInput->nextLink;

    if(!prevNode){

        free(queueInput);

        return;

    }

    if(!currentNode->nextLink){

        free(currentNode->theJob);
        free(currentNode);
        free(queueInput);

        return;

    }

    while ((currentNode = currentNode->nextLink))
    {
        
        free(prevNode->theJob);
        free(prevNode);

        prevNode = currentNode;

    }

    free(prevNode->theJob);
    free(prevNode);
    free(queueInput);

}

int mainScheduler(FILE *fileFDInput)
{

    struct jobInfo currentJob;

    struct jobQueue *headNode = calloc(1, sizeof(struct jobQueue));

    struct jobQueue *currentRunningJob = NULL;

    struct jobQueue *NextRunningJob = NULL;

    int functionReturnCode = 0;

    if (!headNode)
    {

        perror("error with calloc(1,sizeof(struct jobQueue*)) in mainScheduler(FILE *fileFDInput): ");

        return 1;
    }

    memset(&currentJob, 0, sizeof(struct jobInfo));

    while (!getJob(fileFDInput, &currentJob))
    {

        switch (currentJob.jobType)
        {
        case 0:

            if (addJobToQueue(headNode, currentJob))
            {

                printf("failure in addJobToQueue() withen mainScheduler() \n");

                return 1;
            }

            if (currentRunningJob)
            {

                currentRunningJob->theJob->passLevel = currentRunningJob->theJob->stride;
            }
            else
            {

                if (!(currentRunningJob = getMinJob(headNode)))
                {

                    printf("No jobs in Queue \n");

                    break;
                }
            }

            printf("New job: %s added with priority: %d \n", currentJob.jobName, currentJob.jobPriority);

            break;
        case 1:

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }

            printf("Job: %s completed. \n", currentRunningJob->theJob->jobName);

            free(currentRunningJob->theJob);
            free(currentRunningJob);

            break;
        case 2:

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }

            reAddToQueue(headNode, currentRunningJob);

            if (!(currentRunningJob = getMinJob(headNode)))
            {

                printf("No jobs in Queue \n");

                break;
            }

            printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);

            break;
        case 3:

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }

            printf("Job: %s blocked. \n", currentRunningJob->theJob->jobName);

            currentRunningJob->theJob->blocked = 1;

            if (!(currentRunningJob = getMinJob(headNode)))
            {

                printf("No jobs in Queue \n");

                break;
            }

            break;
        case 4:

            if (!(NextRunningJob = unblockJob(headNode, currentJob, &functionReturnCode)))
            {

                if (functionReturnCode == 2)
                {

                    printf("No such job to unblock \n");

                    break;
                }
                else
                {

                    printf("Error. Job: %s not blocked. \n", currentJob.jobName);

                    break;
                }
            }

            NextRunningJob->theJob->passLevel += NextRunningJob->theJob->stride;

            printf("Job: %s has unblocked. Pass set to: %d \n", NextRunningJob->theJob->jobName, NextRunningJob->theJob->passLevel);

            reAddToQueue(headNode, currentRunningJob);

            currentRunningJob = NextRunningJob;

            NextRunningJob = NULL;

            break;
        case 5:

            if (listRunnables(headNode))
            {

                printf("None \n");
            }

            break;
        case 6:

            if (!currentRunningJob)
            {
                printf("Running: \n");
                printf("None \n");

                break;
            }
            else
            {

                printf("Running: \n");
                printf("NAME    STRIDE  PASS  PRI \n");

                printf("%s       %d     %d  %d \n", currentRunningJob->theJob->jobName, currentRunningJob->theJob->stride, currentRunningJob->theJob->passLevel, currentRunningJob->theJob->jobPriority);
            }

            break;
        case 7:

            if (listBlocked(headNode))
            {

                printf("None \n");
            }

            break;
        default:
            break;
        }
    }

    freeTheQueue(headNode);
    if(currentRunningJob){

        free(currentRunningJob->theJob);
        free(currentRunningJob);

    }
    

    if(NextRunningJob){

        free(NextRunningJob->theJob);
        free(NextRunningJob);

    }

    return 0;
}

int main(int argc, char *argv[])
{

   struct argStruct argHolder;

   memset(&argHolder,0,sizeof(struct argStruct));

    FILE *fileFD = NULL;

    // setup program by getting input file argument and opening the file
    if (handleArgs(argc, argv, &argHolder))
    {

        printf("error in handle args: invaild argument given \n");

        return 1;
    }

    if (!(fileFD = fopen(argHolder.inputFIleDest, "r")))
    {

        perror("error in fopen(schecdularJobFileInputPath,\"r\"): ");

        return 1;
    }
    // finish setup start performing scheduling

    if(mainScheduler(fileFD)){

        printf("error in mainScheduler() \n");

        return 1;

    }

    //close the file stream
    if (fclose(fileFD))
    {

        perror("error in fclose(fileFD): ");

        return 1;
    }

    return 0;
}