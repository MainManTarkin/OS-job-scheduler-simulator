#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

struct argStruct
{

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
    int baseCount = 0;
    int whiteSpaceCount = 0;
    char copyStringBuffer[100];

    memset(&copyStringBuffer, 0, sizeof(copyStringBuffer));

    // find where the string ends either with a comman or a newline
    while (1)
    {

        if (jobStringInput[whiteSpaceCount] != ',' && jobStringInput[whiteSpaceCount] != '\n' && jobStringInput[whiteSpaceCount] != 0)
        {

            whiteSpaceCount++;
        }
        else
        {

            break;
        }
    }

    // compare the full string to some consts to find what job it is and set struct accordingly
    // job values are listed at top
    if (!strncmp(jobStringInput, "newjob", whiteSpaceCount))
    {

        //loop to get name of job
        jobInfoInput->jobType = 0;
        baseCount = ++whiteSpaceCount;
        while (1)
        {

            if (jobStringInput[whiteSpaceCount] != ',')
            {

                whiteSpaceCount++;
            }
            else
            {

                break;
            }
        }

        strncpy(jobInfoInput->jobName, jobStringInput + baseCount, whiteSpaceCount - baseCount);

        baseCount = ++whiteSpaceCount;

        //loop to get job priorty
        while (1)
        {

            if (jobStringInput[whiteSpaceCount] != 0 && jobStringInput[whiteSpaceCount] != '\n')
            {

                whiteSpaceCount++;
            }
            else
            {

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
        //loop to get job name to unblock
        baseCount = ++whiteSpaceCount;
        while (1)
        {

            if (jobStringInput[whiteSpaceCount] != 0 && jobStringInput[whiteSpaceCount] != '\n')
            {

                whiteSpaceCount++;
            }
            else
            {

                break;
            }
        }

        strncpy(jobInfoInput->jobName, jobStringInput + baseCount, whiteSpaceCount - baseCount);
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
    char *jobString = calloc(jobStringSize, sizeof(char));
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

    if (jobString[0] != '\n' && jobString[0] != 0 && jobString[0] != ',')
    {
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
    }
    else
    {
        //if string has bad values return a junk jobtype value
        jobInfoInput->jobType = 8;
    }

    // free the string the came from getline() since we now have it in are struct
    free(jobString);

    return 0;
}

int alphabeticalOrder(struct jobState *job1, struct jobState *job2)
{//if a tie with pass values is found based on job names go through untill you find a greater value

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
{// used to create a more fair pass value set if a newjob is added latter by reseting all pass values in the list
//currently not being used

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
{//create a whole new node from the newjob command and add it to the list

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
    newNode->theJob->stride = 100000 / newJobInput.jobPriority;
    newNode->theJob->passLevel = newNode->theJob->stride;
    strcpy(newNode->theJob->jobName, newJobInput.jobName);

    // used for fair new entry : other purpose
    // resetQueuePassVals(queueInput);

    //all comments in void reAddToQueue()'s while loop and down apply here
    while ((currentNode = currentNode->nextLink))
    {

        if (currentNode->theJob->passLevel > newNode->theJob->passLevel)
        {

            newNode->backLink = currentNode->backLink;
            newNode->nextLink = currentNode;

            currentNode->backLink->nextLink = newNode;
            currentNode->backLink = newNode;

            return 0;
        }
        else if (currentNode->theJob->passLevel == newNode->theJob->passLevel)
        {

            if (alphabeticalOrder(currentNode->theJob, newNode->theJob))
            {

                newNode->backLink = currentNode->backLink;
                newNode->nextLink = currentNode;

                currentNode->backLink->nextLink = newNode;
                currentNode->backLink = newNode;

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
{//readd a the former current running job back into the list

    struct jobQueue *currentNode = queueInput;
    struct jobQueue *prevNode = queueInput;

    if (!currentNode->nextLink)//if the list is only just the head
    {

        jobToReAdd->backLink = currentNode;
        jobToReAdd->nextLink = NULL;
        currentNode->nextLink = jobToReAdd;
        

        return;
    }

    while ((currentNode = currentNode->nextLink))
    {
        //iterate through the list comparing pass values
        //if the pass value of the current node is greater then the one we are adding
        //insert it before that node
        if (currentNode->theJob->passLevel > jobToReAdd->theJob->passLevel)
        {

            jobToReAdd->backLink = currentNode->backLink;
            jobToReAdd->nextLink = currentNode;

            currentNode->backLink->nextLink = jobToReAdd;
            currentNode->backLink = jobToReAdd;

            

            return;
        }//check to see if the pass values where equal
        else if (currentNode->theJob->passLevel == jobToReAdd->theJob->passLevel)
        {
            //if they were determine the order by alphabetical order
            if (alphabeticalOrder(currentNode->theJob, jobToReAdd->theJob))
            {
                //if are readded node is a winner add it before the current node
                jobToReAdd->nextLink = currentNode;
                jobToReAdd->backLink = currentNode->backLink;

                currentNode->backLink->nextLink = jobToReAdd;

                currentNode->backLink = jobToReAdd;

                return;
            }
            //if not then keep searching
        }

        prevNode = currentNode;
    }

    // done only if we reached the end of the list thus placeing the node at the end
    prevNode->nextLink = jobToReAdd;

    jobToReAdd->backLink = prevNode;

    jobToReAdd->nextLink = NULL;
}

struct jobQueue *getMinJob(struct jobQueue *queueInput)
{//get the job from the list with the lowest pass value

    struct jobQueue *currentNode = queueInput->nextLink;

    struct jobQueue *returnJob = NULL;

    while (currentNode)
    {
        //the list is already sorted but iterate through it to find the first non-blocked job

        if (!currentNode->theJob->blocked)
        {

            returnJob = currentNode;

            break;
        }

        currentNode = currentNode->nextLink;
    }

    if (returnJob)//if a job was found remove it from the list
    {

        if (returnJob->backLink)
        {

            returnJob->backLink->nextLink = returnJob->nextLink;
        }

        if (returnJob->nextLink)
        {

            returnJob->nextLink->backLink = returnJob->backLink;
        }

        returnJob->theJob->passLevel += returnJob->theJob->stride;
    }

    return returnJob;
}

struct jobQueue *unblockJob(struct jobQueue *queueInput, struct jobInfo nameOfJobInput, int *runStatus)
{

    struct jobQueue *currentNode = queueInput;

    while ((currentNode = currentNode->nextLink))
    {
        //check to see if the name of the job to the current node it the same as the one we want to unblock
        if (!strcmp(currentNode->theJob->jobName, nameOfJobInput.jobName))
        {

            if (currentNode->theJob->blocked)
            {
                //found job, unblocked it, and return a pointer to it
                currentNode->theJob->blocked = 0;

                /*uncomment incase of blocked sorting issue (it should not be a problem but incase this will solve it)
                if(currentNode->nextLink){

                    currentNode->nextLink->backLink = currentNode->backLink;

                }

                if(currentNode->backLink){

                    currentNode->backLink->nextLink = currentNode->nextLink;

                }

                currentNode->backLink = NULL;
                currentNode->nextLink = NULL;

                reAddToQueue(queueInput,currentNode);
                */

                return currentNode;
            }
            else
            {
                //job found but not blocked
                *runStatus = 1;

                return NULL;
            }
        }
    }
    //did not find job
    *runStatus = 2;

    return NULL;
}

int listBlocked(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput;

    printf("Blocked: \n");

    if (!currentNode->nextLink)//if the list is only the head, end the function
    {

        return 1;
    }

    
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

    printf("Runnable: \n");

    if (!currentNode->nextLink)//if the list is only the head, end the function
    {

        return 1;
    }

    
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

void freeTheQueue(struct jobQueue *queueInput)
{

    struct jobQueue *currentNode = queueInput->nextLink;
    struct jobQueue *prevNode = queueInput->nextLink;

    if (!prevNode)//if the list is only just the head
    {

        free(queueInput);

        return;
    }

    if (!currentNode->nextLink)//if the list is only the head plus one node
    {

        free(currentNode->theJob);
        free(currentNode);
        free(queueInput);

        return;
    }

    //if the list contains more the 2
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
    //set up variable and structs
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

    //main loop

    while (!getJob(fileFDInput, &currentJob))//get a job from file
    {

        switch (currentJob.jobType)
        {
        case 0://add a new job
            
            if (addJobToQueue(headNode, currentJob))
            {

                printf("failure in addJobToQueue() withen mainScheduler() \n");

                return 1;
            }

            printf("New job: %s added with priority: %d \n", currentJob.jobName, currentJob.jobPriority);

            //check to see if there is a current job 
            //if not then get one other wise break from case
            if (currentRunningJob)
            {
                //something from earlier version
                // currentRunningJob->theJob->passLevel = currentRunningJob->theJob->stride;
            }
            else
            {

                if (!(currentRunningJob = getMinJob(headNode)))
                {

                    printf("System is idle. \n");

                    break;
                }

                printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);
            }

            break;
        case 1://the current job is done running

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }

            printf("Job: %s completed. \n", currentRunningJob->theJob->jobName);

            //free the node and its componets
            free(currentRunningJob->theJob);
            free(currentRunningJob);
            currentRunningJob = NULL;//always null the current job when a switch occurs just in case

            if (!(currentRunningJob = getMinJob(headNode)))
            {

                printf("System is idle. \n");
            }
            else
            {

                printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);
            }

            break;
        case 2://the job has completed its quantum 

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }


            //add the job back on to the list in proper order
            reAddToQueue(headNode, currentRunningJob);

            currentRunningJob = NULL;//null the job as always

            if (!(currentRunningJob = getMinJob(headNode)))
            {

                printf("System is idle. \n");

                break;
            }

            printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);

            break;
        case 3://block current job

            if (!currentRunningJob)
            {

                printf("Error. System is idle. \n");

                break;
            }

            printf("Job: %s blocked. \n", currentRunningJob->theJob->jobName);

            //set job status to block before addeding it back on to list
            currentRunningJob->theJob->blocked = 1;

            reAddToQueue(headNode, currentRunningJob);

            currentRunningJob = NULL;

            if (!(currentRunningJob = getMinJob(headNode)))
            {

                printf("System is idle. \n");

                break;
            }
            else
            {

                printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);
            }

            break;
        case 4://unblock specified job (as if the bottom function was not enough of a hint)

            if (!(NextRunningJob = unblockJob(headNode, currentJob, &functionReturnCode)))
            {

                if (functionReturnCode == 2)
                {//could not find job in list

                    printf("Error. Job: %s not blocked. \n", currentJob.jobName);

                    break;
                }
                else
                {//job found but is not blocked

                    printf("Error. Job: %s not blocked. \n", currentJob.jobName);

                    break;
                }
            }
            //since unblocking the job (if it worked) get a new job 
            reAddToQueue(headNode, currentRunningJob);

            currentRunningJob = NULL;

            if (!(currentRunningJob = getMinJob(headNode)))
            {
                printf("Job: %s has unblocked. Pass set to: %d \n", NextRunningJob->theJob->jobName, NextRunningJob->theJob->passLevel);
                printf("System is idle. \n");
            }
            else
            {

                printf("Job: %s has unblocked. Pass set to: %d \n", NextRunningJob->theJob->jobName, NextRunningJob->theJob->passLevel);
                printf("Job: %s scheduled. \n", currentRunningJob->theJob->jobName);
            }

            NextRunningJob = NULL;

            break;
        case 5://list possible job to run in order

            if (listRunnables(headNode))
            {

                printf("None \n");
            }

            break;
        case 6://list what is currently running

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
        case 7://list what is blocked

            if (listBlocked(headNode))
            {

                printf("None \n");
            }

            break;
        default://garbage jobs case
            break;
        }
    }

    freeTheQueue(headNode);
    if (currentRunningJob)
    {

        free(currentRunningJob->theJob);
        free(currentRunningJob);
    }

    return 0;
}

int main(int argc, char *argv[])
{

    struct argStruct argHolder;

    memset(&argHolder, 0, sizeof(struct argStruct));

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

    if (mainScheduler(fileFD))
    {

        printf("error in mainScheduler() \n");

        return 1;
    }

    // close the file stream
    if (fclose(fileFD))
    {

        perror("error in fclose(fileFD): ");

        return 1;
    }

    return 0;
}