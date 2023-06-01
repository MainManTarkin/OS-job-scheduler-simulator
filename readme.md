# OS-job-scheduler-simulator

## Project Description

This program was created for Cathage's operating systems class. The guidelines for the project were to create
a functional simulated __stride scheduler__. The program should be able to take in a list of commands from a text file that
represents the actions a stride scheduler would undergo.

### commands

To simulate an OS task scheduler, commands are listed in a text file for the program to read.
Commands can have one or several argument values seperated by commas.

the list of commands are:
* newjob - create new job with _arg1_ being the jobname (100 characters max) and _arg2_ being the priority level
* finish - current job is done
* interrupt - current running program has exceeded it runtime
* block - blocked current running program
* unblock - unblock a blocked job with _arg1_ being the jobname to unblock
* runnable - print information about the jobs in the job queue
* running - print information about current running job
* blocked - print information about blocked jobs

## how to run program
compile main.c on posix compliant system.

should work just fine.
> gcc main.c

## how to use program

1. create text file with commands (commands should be seperated by newline)
2. run program with argument __-i__ followed by path to command text file
3. program should print output based on given commands