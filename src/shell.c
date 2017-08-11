/*
~shell.c is a UNIX shell~
written by Joshua Simmons
written for CISC361 (Operating Systems)
*/

#include <stdio.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


int debug = 0;
#define BUFFER_MAX_LENGTH 50
#define PATH_MAX_LENGTH 1024
static const char EXTSTR[] = "exit";
static char buffer[BUFFER_MAX_LENGTH];
static char bufferFirstRun[BUFFER_MAX_LENGTH];
static char userInput = '\0';
static char *commandArgv[100];
static char *commandArgv2[100];
static int commandArgc = 0;
static int commandArgc2 = 0;
int redirects = 0;
char *newOut;
char *newIn;
char toOut[PATH_MAX_LENGTH];
char toIn[PATH_MAX_LENGTH];
int redirectIn = 0;
int redirectOut = 0;
int bg = 0;
pid_t pid;
int jobBg;
int piped = 0;

/*
A function to concatenate strings.
*/
char* concat(char *s1,char *s2){
    char *result = (char *)malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

void handleSIGINT(int signum)
{
    const char msg[] = "\nexiting\n";
    write(STDERR_FILENO, msg, sizeof(msg)-1);
    exit(0);
}

/*
The welcome prompt is populated with information on the current session.
This may include other information in the future.
*/
void welcomeScreen(){
    printf("\nSHINY-SPICE-SHELL\n");
    printf("*********************\n");
}

/*
This function arranges the string to output as shell prompt and prints it.
This may include date, time, PID, or other things in the future.
*/
void sayPrompt()
{
    char *PS1 = getenv("PS1");
    if(PS1 == NULL) {
        printf("%s", "$");
    }
    else {
        printf("%s", PS1);
    }
    //printf("[ ID:%d:SHELL ]$ ",getpid());
}

char* getTextLine(char* buffer)
{
    fgets(buffer,BUFFER_MAX_LENGTH,stdin);
    return concat(&userInput,buffer);
}

int checkIORedirect(char* buffer){
    int numberOfRedirectsIn = 0;
    int numberOfRedirectsOut = 0;

    char *bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strncpy(buffercpy,buffer,BUFFER_MAX_LENGTH);

    bufferPointer = strtok(buffercpy," \n");
    while(bufferPointer != NULL)
        {
            switch(bufferPointer[0])
            {
                case '<':
                {
                    redirectIn = 1;
                    strncpy(&toIn[0],bufferPointer,PATH_MAX_LENGTH);
                    numberOfRedirectsIn++;
                    fflush(stdout);
                    break;

                }
                case '>':
                {
                    redirectOut = 1;
                    strncpy(&toOut[0],bufferPointer,PATH_MAX_LENGTH);
                    numberOfRedirectsOut++;
                    fflush(stdout);
                    break;
                }
            }
        bufferPointer = strtok(NULL," \n");
        }
    if(numberOfRedirectsIn > 1 || numberOfRedirectsOut > 1)
    {
        return -1;
    }
    else if(numberOfRedirectsIn == 1 || numberOfRedirectsOut == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



/*
Walks through a copy of the buffer and ads the values to the arguements array.
*/
char** populateCommand(char** commandArgv,char* buffer)
{
    char *bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strncpy(buffercpy,buffer,BUFFER_MAX_LENGTH);
    bufferPointer = strtok(buffer," \n");
    while(bufferPointer != NULL)
        {
            switch(bufferPointer[0])
            {
                case '>':
                {
                    char toOutcpy[PATH_MAX_LENGTH];
                    strncpy(toOutcpy,toOut,PATH_MAX_LENGTH);
                    for(int i = 1;(i<PATH_MAX_LENGTH + 1)&&(toOutcpy[i] != '\0');i++)
                    {
                        toOut[i-1] = toOutcpy[i];
                    }
                    toOut[strlen(toOutcpy)-1] = '\0';
                    newOut = toOut;
                    fflush(stdout);
                    break;
                }
                case '<':
                {
                    char toIncpy[PATH_MAX_LENGTH];
                    strncpy(toIncpy,toIn,PATH_MAX_LENGTH);
                    for(int i = 1;(i<PATH_MAX_LENGTH+1)&&(toIncpy[i] !='\0');i++)
                    {
                        toIn[i-1] = toIncpy[i];
                    }
                    toIn[strlen(toIncpy)-1] = '\0';
                    newIn = toIn;
                    fflush(stdout);
                    break;
                }
                case '&':
                {
                    bg = 1;
                    break;
                }
                default:
                {
                    commandArgv[commandArgc] = bufferPointer;
                    commandArgc++;
                    break;
                }
            }

            bufferPointer = strtok(NULL," \n");
        }

        return commandArgv;
}

/*
This function checks the first element of the command argument to see if it is
a built in command.
If the command is "exit" the program exits.
-return: This function returns 0 if no built in commands were matched.
*/
int checkBuiltInCommands(char** commandArgv)
{
    if(strcmp(EXTSTR,commandArgv[0]) == 0)
    {
        return -1;
    }
    if(strcmp("resume",commandArgv[0]) == 0)
    {
        return 1;
    }
    return 0;
}

/*
This function checks for the built in commands. If there weren't and built in
commands fork and exec.
*/
void handleUserCommands(int redirects,char** commandArgv)
{
    int check = checkBuiltInCommands(commandArgv);
    if(check == 0)
    {
        if(bg == 1)
        {
            int status;
            pid = fork();
            jobBg = 1;
            if(pid == 0)
            {
                if(redirects == 1)
                {
                    if(redirectOut == 1)
                    {
                        int out = open(newOut,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        dup2(out,STDOUT_FILENO);
                        close(out);
                    }
                    if(redirectIn == 1)
                    {
                        int in = open(toIn,O_RDONLY);
                        dup2(in,STDIN_FILENO);
                        close(in);
                    }
                    setpgid(0,0);
                    status = execvp(commandArgv[0],commandArgv);
                    printf("%s : command not found*\n",commandArgv[0]);
                    exit(0);
                }
                else if(redirects == -1)
                {
                    printf("shell says : invalid redirects\n");
                    exit(0);
                }
                else
                {
                    setpgid(0,0);
                    status = execvp(commandArgv[0],commandArgv);
                    printf("%s : command not found*\n",commandArgv[0]);
                    exit(0);
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            int status;
            pid = fork();
            jobBg = 1;
            if(pid == 0)
            {
                if(redirects == 1)
                {
                    if(redirectOut == 1)
                    {
                        int out = open(newOut,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        dup2(out,STDOUT_FILENO);
                        close(out);
                    }
                    if(redirectIn == 1)
                    {
                        int in = open(toIn,O_RDONLY);
                        dup2(in,STDIN_FILENO);
                        close(in);
                    }
                    status = execvp(commandArgv[0],commandArgv);
                    printf("%s : command not found*\n",commandArgv[0]);
                    exit(0);
                }
                else if(redirects == -1)
                {
                    printf("shell says : invalid redirects\n");
                    exit(0);
                }
                else
                {
                    status = execvp(commandArgv[0],commandArgv);
                    printf("%s : command not found*\n",commandArgv[0]);
                    exit(0);
                }
            }
            else
            {
                waitpid(pid,&status,WUNTRACED);
                if (WIFEXITED(status))
                {
                    if(WEXITSTATUS(status) != 0)
                    {
                    printf("[ last program returned exit code: %d ] ", WEXITSTATUS(status));
                    }
                }
            }
        }
    }
    else if(check == 1)
    {
        kill(pid,SIGCONT);
        printf("resumed\n:pid:%d:sig:%d\n",pid,SIGCONT);
        fflush(stdout);


    }
    else if(check == -1)
    {
        exit(EXIT_SUCCESS);
    }
}

/*
This function clears out all of the arguments that are in commandArgv
*/
void destroyCommand(char** commandArgv)
{
    while(commandArgc != 0)
        {
        commandArgv[commandArgc] = NULL;
        commandArgc--;
    }
}

int main(){
    signal(SIGINT,handleSIGINT);
    tcsetpgrp(STDIN_FILENO, getpid());
    char *commandArgv[100];
    welcomeScreen();
    sayPrompt();
    while(1){
        destroyCommand(commandArgv);
        //get the first char
        userInput = getchar();
        switch(userInput)
        {
            //if user just typed enter then start over.
            case '\n':
            sayPrompt();
            break;
            default:
            {
                char buffer[BUFFER_MAX_LENGTH];
                strncpy(buffer,getTextLine(buffer),BUFFER_MAX_LENGTH);
                int redirects = checkIORedirect(buffer);
                populateCommand(commandArgv,buffer);
                handleUserCommands(redirects,commandArgv);
                redirectIn = 0;
                redirectOut = 0;
                bg = 0;
                jobBg = 0;
                piped = 0;
                sayPrompt();
                break;
            }
        }
    }
    printf("\n");
    return(0);
}
