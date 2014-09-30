#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

//exit string to compare tp
char e[] = "exit";
//path to bin
char bin[] = "/bin/";

#define BUFFER_MAX_LENGTH 50
static char buffer[BUFFER_MAX_LENGTH];
static char userInput = '\0';
static int bufferChars = 0;
static char *commandArgv[5];
static int commandArgc = 0;

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
    bufferPointer = strtok(buffer," ");
    while(bufferPointer != NULL){
        commandArgv[commandArgc] = bufferPointer;
        bufferPointer = strtok(NULL," ");
        commandArgc++;
    }
}

void getTextLine(){
    destroyCommand();
    while((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH)){
        buffer[bufferChars++] = userInput;
        userInput = getchar();
    }
    buffer[bufferChars] = 0x00;
    populateCommand();
}


int main(){
    //init();
    //welcomeScreen();
    sayPrompt();
    while(1){
        userInput = getchar();
        switch(userInput){
            case '\n':
                sayPrompt();
                break;
            default:
                getTextLine();
                handleUserCommands();
                sayPrompt();
                break;
        }
    }
    printf("\n");
    return(0);
}
