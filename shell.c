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

//function to concatenate strings
char* concat(char *s1,char *s2){
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

//arrange the string to output as shell prompt
void sayPrompt(){
    printf("\nSHELL>");
}

int checkBuiltInCommands(){
    if(strcmp("exit",commandArgv[0]) == 0){
        exit(EXIT_SUCCESS);
    }
    return 0;
}

int fileExists(const char* file) {
    struct stat buf;
    return stat(file, &buf);
}

void handleUserCommands(){
    if(checkBuiltInCommands() == 0){
        pid_t pid = fork();
        if(pid == 0){
            if(fileExists(concat(bin,commandArgv[0])) == 0){
                execvp(*commandArgv,commandArgv);
            }
            else{
                exit(0);
            }
        }
        else{
            wait(&pid);
        }
    }
}

//walks backwards and re-nulls the ArgV
void destroyCommand(){
    while(commandArgc != 0){
        commandArgv[commandArgc] = NULL;
        commandArgc--;
    }
    bufferChars = 0;
}


//walks forward through the input until null terminator

void populateCommand(){
    char* bufferPointer;
    char buffercpy[BUFFER_MAX_LENGTH];
    strcpy(buffercpy,buffer);
    bufferPointer = strtok(buffercpy," ");
    while(bufferPointer != NULL){
        char toOut[strlen(bufferPointer)+1];
        switch(bufferPointer[0])
        {

            case '>':
            {
                for(int i = 1;i<strlen(bufferPointer);i++){
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
                printf("default");
                //commandArgv[commandArgc] = bufferPointer;
                //commandArgc++;
            }
            break;
        }
        bufferPointer = strtok(NULL," ");
    }
}

void getTextLine(){
    while((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH)){
        buffer[bufferChars++] = userInput;
        userInput = getchar();
        typedCount++;
    }
    buffer[bufferChars] = 0x00;
}

int checkForIOIn(){

        char* bufferPointer;
        char buffercpy[BUFFER_MAX_LENGTH];
        strcpy(buffercpy,buffer);
        bufferPointer = strtok(buffercpy," ");
        while(bufferPointer != NULL){
            switch(bufferPointer[0])
            {
                case '<':
                {
                    inRedirect = bufferPointer;

                    printf("%s",inRedirect);
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


int main(){
    //copy the stdin and stdout in case they are manipulated
    int stdoutCpy = dup(1);
    int stdinCpy = dup(0);
    int in;
    //init();
    //welcomeScreen();
    sayPrompt();
    while(1){
        typedCount = 0;
        //get the first char
        userInput = getchar();
        typedCount++;
        switch(userInput){
            //if user just typed enter then start over.
            case '\n':
                sayPrompt();
                break;
            default:
            {
            destroyCommand();
            //get what the user typed
            getTextLine();
            //get through it once and look for IO redirection IN
            if(checkForIOIn() == 1){
                //open file redirect
                in = open(inRedirect,O_RDONLY,0);
                //make stdin this file
                //close the fd
            }
            destroyCommand();
            getTextLine();
            if(&in != NULL){
                dup2(in,0);
                getTextLine();
                dup2(stdinCpy,0);
                close(in);
            }
            populateCommand();
            destroyCommand();
            //handleUserCommands();
            dup2(stdoutCpy,1);
            dup2(stdinCpy,0);
            close(stdoutCpy);
            close(stdinCpy);
            sayPrompt();
            }
                break;
        }
    }
    printf("\n");
    return(0);
}
