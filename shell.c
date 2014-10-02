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

//exit string to compare tp
char e[] = "exit";
//path to bin
char bin[] = "/bin/";

#define BUFFER_MAX_LENGTH 50
static char buffer[BUFFER_MAX_LENGTH];
int typedCount = 0;
static char userInput = '\0';
static int bufferChars = 0;
static char *commandArgv[5];
static int commandArgc = 0;
char *inRedirect;

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
    if(commandArgv[0] != NULL)
    {

    if(strcmp(e,commandArgv[0]) == 0)
        {
            exit(EXIT_SUCCESS);
        }
    }
        return 0;
}

/*
This function checks for the built in commands. If there weren't and built in
commands fork and exec.
*/
void handleUserCommands()
{
    if(checkBuiltInCommands() == 0)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            if( access( concat(bin,commandArgv[0]), F_OK ) != -1 )
            {
                printf("%d",access(concat(bin,commandArgv[0]),F_OK));
                execvp(*commandArgv,commandArgv);
            }
            else
            {
                exit(0);
            }
        }
        else{
            wait(&pid);
        }
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
    bufferChars = 0;
}

/*
Walks through a copy of the buffer and ads the values to the arguements array.
*/
void populateCommand()
{
    char* bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strncpy(buffercpy,buffer,BUFFER_MAX_LENGTH);
    bufferPointer = strtok(buffercpy," ");
    while(bufferPointer != NULL)
        {
        char toOut[strlen(bufferPointer)+1];
        switch(bufferPointer[0])
        {

            case '>':
            {
                for(int i = 1;i<strlen(bufferPointer);i++)
                    {
                    toOut[i-1] = bufferPointer[i];
                }
                toOut[strlen(bufferPointer)-1] = '\0';
                int out = open(toOut,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                dup2(out,1);
                close(out);
            }
            break;
            case '<':
            {
            }
            break;
            case '|':
            {
            }
            break;
            case '&':
            {
            }
            break;
            default:
            {
                commandArgv[commandArgc] = bufferPointer;
                commandArgc++;
            }
            break;
        }
        bufferPointer = strtok(NULL," ");
    }
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
    while((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH))
        {
        buffer[bufferChars] = userInput;
        typedCount++;
        bufferChars++;
    }
    buffer[bufferChars++] = 0x00;
}

/*
This function runs through a copy of the buffer to check if any IO
redirection (stdin only) needs to take place.
-return: 1 if there is a redirection in. 0 if no redirection is required.
*/
int checkForIOIn(){

    char* bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strncpy(buffercpy,buffer,BUFFER_MAX_LENGTH);
    bufferPointer = strtok(buffercpy," ");
    while(bufferPointer != NULL){
        switch(bufferPointer[0])
        {
            case '<':
            {
                inRedirect = bufferPointer;
                return 1;
            }
            break;
            default:
            {
            }
            break;
        }
        bufferPointer = strtok(NULL," ");
    }
    return 0;
}

int length(const char* array[]) {
  return sizeof(array)/sizeof(char*);
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
        typedCount = 0;
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
                //get what the user typed
                getTextLine();
                populateCommand();
                //get through it once and look for IO redirection IN
                if(checkForIOIn() == 1)
                {
                    //open file redirect
                    in = open(inRedirect,O_RDONLY,0);
                    buffer[bufferChars] = ' ';
                    getTextLine();
                    dup2(in,0);
                    buffer[bufferChars++] = ' ';
                    getTextLine();
                    dup2(stdinCpy,0);
                    close(in);
                }
                destroyCommand();
                populateCommand();
                printf("buffer:%s",buffer);
                for(int i = 0;i<length(commandArgv);i++ )
                {
                printf("commands:%s",commandArgv[i]);
                }
                handleUserCommands();
                dup2(stdoutCpy,1);
                dup2(stdinCpy,0);
                sayPrompt();
            }
            break;
        }
    }
    //close(stdinCpy);
    //close(stdoutCpy);
    printf("\n");
    return(0);
}
