/*
~shell.c is a UNIX shell~
written by Joshua Simmons
written for CISC361 (Operating Systems)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int debug = 0;

//exit string to compare tp
char e[] = "exit";
//path to bin
char bin[] = "/bin/";

#define BUFFER_MAX_LENGTH 50
static char buffer[BUFFER_MAX_LENGTH];
static char bufferFirstRun[BUFFER_MAX_LENGTH];
static char bufferCut[BUFFER_MAX_LENGTH];
static char userInput = '\0';
static char *commandArgv[100];
static int commandArgc = 0;
char *redirectIn;
char *redirectOut;
char *redirect;

/*
A function to concatenate strings(should be in a header somewhere).
*/
char* concat(char *s1,char *s2){
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

/*
The welcome prompt is populated with information on the current session.
This may include other information in the future.
*/
void welcomeScreen(){
    printf("\nSHINY-SPICE-SHELL\n");
    printf("*********************");
}

/*
This function arranges the string to output as shell prompt and prints it.
This may include date, time, PID, or other things in the future.
*/
void sayPrompt()
{
    printf("\nSHELL>");
}

/*
This function checks the first element of the command argument to see if it is
a built in command.
If the command is "exit" the program exits.
-return: This function returns 0 if no built in commands were matched.
*/
int checkBuiltInCommands()
{
    if(debug == 1)
    {
        printf("compare value: %d\n",strcmp("exit",commandArgv[0]));
    }
    if(strcmp(e,commandArgv[0]) == 0)
    {
        return -1;
    }
    return 0;
}

/*
This function checks for the built in commands. If there weren't and built in
commands fork and exec.
*/
void handleUserCommands(char** commandArgv)
{
    if(debug == 1)
    {
        printf("-commandArgV[0] right before handling is: %s\n",commandArgv[0]);
    }
    int check = checkBuiltInCommands();
    if(debug == 1)
    {
    printf("user command was: %d\n",check);
    }
    if(check == 0)
    {
        pid_t pid = fork();
        //int fileExists = access(concat(bin,commandArgv[0]), F_OK );
        if(pid == 0)
        {
            //strncpy(commandArgv[0],concat(bin,commandArgv[0]),strlen(commandArgv[0]));
            if(debug == 1)
            {
            printf("your command is: %s\n",commandArgv[0]);
            }
            execvp(commandArgv[0],commandArgv);
            printf("%s : command not found*\n",commandArgv[0]);
            exit(0);
        }
        else{
            wait(&pid);
        }
    }
    else if(check == -1)
    {
        exit(EXIT_SUCCESS);
    }
}

/*
This function clears out all of the arguments that are in commandArgv
*/
void destroyCommand()
{
    while(commandArgc != 0)
        {
        commandArgv[commandArgc] = NULL;
        commandArgc--;
    }
}

/*
Walks through a copy of the buffer and ads the values to the arguements array.
*/
char** populateCommand()
{
    if(debug == 1)
    {
        printf("-buffer right before populating command : %s\n",buffer);
    }
    char *bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strncpy(buffercpy,buffer,BUFFER_MAX_LENGTH);
    if(debug == 1)
    {
        printf("-copy of buffer right before populating : %s\n",buffercpy);
    }
    bufferPointer = strtok(buffer," \n");
    while(bufferPointer != NULL)
        {
            if(debug == 1)
            {
                printf("buffer tkn : %s\n",bufferPointer);
            }
                commandArgv[commandArgc] = bufferPointer;
                commandArgc++;
                bufferPointer = strtok(NULL," \n");
        }
        if(debug == 1)
        {
            printf("end of populate : %s\n",commandArgv[0]);
        }
        return commandArgv;
}

/*
This function walks through the stdin while the character is not '\n'
and the buffer being copied to is not full (except the last character in the
buffer which is reserved for the null terminator). It places what was in
userInput into the buffer and at the end of its loop it ads a null terminator
to the last element of the buffer.
*/

void getTextLine()
{
    fgets(buffer,BUFFER_MAX_LENGTH,stdin);
    strncpy(buffer,concat(&userInput,buffer),BUFFER_MAX_LENGTH);
    if(debug == 1)
    {
    printf("-buffer: %s",buffer);
    }
}

int checkIORedirect(){
    int numberOfRedirectsIn = 0;
    int numberOfRedirectsOut = 0;
    char *bufferPointer;
    strncpy(bufferFirstRun,buffer,BUFFER_MAX_LENGTH);
    bufferPointer = strtok(bufferFirstRun," \n");
    while(bufferPointer != NULL)
        {
            switch(bufferPointer[0])
            {
                case '<':
                {
                    if(redirect == NULL)
                    {
                        redirect = bufferPointer;
                    }
                    redirectIn = bufferPointer;
                    numberOfRedirectsIn++;
                }
                case '>':
                {
                    if(redirect == NULL)
                    {
                        redirect = bufferPointer;
                    }
                    redirectOut = bufferPointer;
                    numberOfRedirectsOut++;
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

int removeRestOfBuff()
{
    int found = 0;
    for(int i = 0;buffer[i] != '\0' && found != 1;i++)
    {
        if(buffer[i] == '>' || buffer[i] == '<')
        {
            buffer[i] = '\0';
            found = 1;
            return i;
        }
    }
    /*
    char *bufferPointer;
    int bufferSize = 0;
    int *concatSize;
    strncpy(bufferCut,buffer,BUFFER_MAX_LENGTH);
    bufferPointer = strtok(bufferCut," \n");
    while(bufferPointer != NULL)
        {
            switch(bufferPointer[0])
            {
                case '<':
                {
                    if(concatSize == NULL)
                    {
                        int currBuffSize = bufferSize;
                        concatSize = &currBuffSize;
                    }
                }
                case '>':
                {
                    if(concatSize == NULL)
                    {
                    int currBuffSize = bufferSize;
                    concatSize = &currBuffSize;
                    }
                }
                default:
                {
                    printf("sizeOfBuffer:%d\n",bufferSize);
                    bufferSize += sizeof(bufferPointer)/sizeof(int);
                }
            }
            bufferPointer = strtok(NULL," \n");
        }
        if(debug == 1)
        {
        //    printf("concatSize:%d\n",*concatSize);
        }
        //buffer[*concatSize] = '\0';
        */
    if(debug == 1)
    {
        printf("-buffer after cutting: %s\n",buffer);
    }
    return -1;

}

int main(){
    //copy the stdin and stdout in case they are manipulated
    int stdoutCpy = dup(1);
    int stdinCpy = dup(0);
    int in;
    //init();
    welcomeScreen();
    sayPrompt();
    while(1){
        destroyCommand();
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
                getTextLine();
                int redirects = checkIORedirect();
                if(debug == 1)
                {
                    printf("-checked for redirect\n--redirectIn: %s\n--redirectOut: %s\n--redirect: %s\n",redirectIn,redirectOut,redirect);
                }
                if(debug == 1)
                {
                printf("-redirects: %d\n",redirects);
                }
                if(redirects == 1)
                {
                    if(redirectOut != NULL)
                    {
                        char toOut[strlen(redirectOut)+1];
                        for(int i = 1;i<strlen(redirectOut);i++)
                        {
                            toOut[i-1] = redirectOut[i];
                        }
                        toOut[strlen(redirectOut)-1] = '\0';

                        int out = open(toOut,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        if(debug == 1)
                        {
                        }
                        dup2(out,STDOUT_FILENO);
                        close(out);
                    }
                    if(redirectIn != NULL)
                    {
                        char toIn[strlen(redirectIn)+1];
                        for(int i = 1;i<strlen(redirectIn);i++)
                            {
                            toIn[i-1] = redirectIn[i];
                        }
                        toIn[strlen(redirectIn)-1] = '\0';
                        int in = open(toIn,O_RDONLY);
                        dup2(in,0);
                        close(in);
                        int start = removeRestOfBuff();
                        char get[BUFFER_MAX_LENGTH];
                        fgets(get,BUFFER_MAX_LENGTH,stdin);
                        for(int i = 0;get[i] != '\n';i++)
                        {
                            buffer[start+i] = get[i];
                        }
                    }


                }
                else if(redirects == -1)
                {
                    if(debug == 1)
                    {
                    printf("-invalid redirecting\n");
                    }
                }
                else
                {
                if(debug == 1)
                {
                printf("-no redirects found\n");
                }
                }

            }
            break;
        }
        handleUserCommands(populateCommand());
        dup2(stdoutCpy,STDOUT_FILENO);
        dup2(stdinCpy,STDIN_FILENO);
        sayPrompt();
        redirectIn = NULL;
        redirectOut = NULL;
        redirect = NULL;
    }
    close(stdinCpy);
    close(stdoutCpy);
    printf("\n");
    return(0);
}
