#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
In this assignment, we need to program simple shell for linux. In the following 4 different C function which is responsible for 
specific linux command. This 4 funtion as following 
    {
        - changeDirectory() --> responsible for changing directory when user enters cd. (cd Command)
        - printWorkingDirectory() --> responsible for printing working directory when user enters dir.(pwd Command)
        - history() --> responsible for showing last 10 commands which is entered by user. 
        - bye() --> exiting command which is entered by user.
    }.

Other than this 4 function I have several function which help program for proper execution
    {
        - reader() --> takes input from user
        - readerHandler() --> process input for usage
        - enqueArgumentHistory() --> add command in history FIFO
        - dequeArgumentHistory() --> remove command in history FIFO
        - checkBG() --> checks input contains &
    }

Becide of the general explanation detailedf explanation of each function in the following.
*/

#define ARGUMENT 10 // According to assingment PDF, I declared a arugument number which is 10.
#define ARGUMENTLENGHT 100// According to assingment PDF, I declared a arugument length which is 100.

int front = -1;// Front index of the history FIFO.
int rear = -1;// Rear index of the history FIFO.
int size = 0;// Size of the history FIFO.

int bg = 0;//Background flag.

char directory[512]; //Directory string for usage in printWorkinDirectory(). 

char historyArray[ARGUMENT][ARGUMENTLENGHT] = {0}; // History FIFO creation.

char iCommand[ARGUMENTLENGHT];// User input memomry is created.

char *commandArray[] = {0};// Array of parsed input memory created for usage.


/*This function called when the history FIFO reaches the maximum size. 
This function removes first index of history queue and shifts all element into previous index, for creating empty space for upcoming 
command in the history queue.*/
void dequeArgumentHistory(){
   
    for(int j = 0; j<size; j++){  // A loop for, shifting elements from index to index-1.
        strcpy(historyArray[j], historyArray[j+1]);
    }
    rear--; // Decrement rear position.
    size--; // Decrement size.
}

/*This function called when the command is added in to history queue. It takes command as arg and place it into history 
queue according to the rear position. If there is no space left fuction calls dequeArgumentHistory and called itself again.*/
void enqueArgumentHistory(char* arg){
    //Condition for proper index to insert. 
    if (size != ARGUMENT){//If history queue has proper index.
        rear++; //Increment size position.
        size++; //Increment rear position.
        strcpy(historyArray[rear], arg); //Assign command to the history queue rear index.
        if(front== -1){//Increment front postion, if it is first command inserted in the history queue.
            front++;
        }
    }
    else{//If history queue reaches its maximum capacitiy.
        dequeArgumentHistory();//Dequeue first command.
        enqueArgumentHistory(arg);//Enqueue command. 
    }
}

/*This funtion called when user wants to display his previos 10 entered command.*/
void history(){

    for(int i = front; i<size; i++){//A loop for iteration over the history queue and print each one of the commands.
        printf("[%d]%s \n",(1+i), historyArray[i]);
    }
}

/*This function called when user wants to display his current working directory. Fuction calls getcwd function and print the result.*/
void printWorkingDirectory(){

    getcwd(directory,sizeof(directory));
    printf("%s\n", directory);
}

/*This function called when user wants to change his current working directory. Function uses chdir function. If user input empty directory changed into home.*/
void changeDirectory(char* dir){
    
    char *dirPtr = dir;//Intitiliaze a pointer for directory.

    //Create temporary array with the same size of directory array for relative path. If first chdir does not execute properly 
    //Function wants to concatante given dir argument with directory. In temp array.
    int size = sizeof(directory);
    char temp[size]; // temp directory array creation.

    int i = 0;
    while(directory[i]!='\0'){//Copy directory to temp.
        temp[i] = directory[i];
        i++;
    }
    temp[i] = '/';//Add / char en of the temp index for ex directory-> /home/baran will be temp -> /home/baran/ to concatanation.

    char* tempPtr = &temp[0];//Intitiliaze a pointer for temp array.
     
    
    //check for empty input when cd called by user.
    if (dir != NULL){//If input is not empty.

        int ret = chdir(dir);//chdir function call.

        if (ret==0){//chdir succseed.
            int i = 0;
            while(temp[i]!='\0'){//Update directory to temp. 
                directory[i] = temp[i];
                i++;
            }
        }
        else{
            //Checking for the relative path. In order to do that 
            //I conctanate temp array which is copy of the directory
            //And add relative dir to the temp and perform chdir operation.

            i = 0;//reinitiliaze i as 0
            int j = 0;// initiliaze int j as 0.
            
            //create a loop which iterates until the null character of temp array is encountered. 
            //When the execution of the loop is completed, then the pointer tempPtr points to 
            //the location that exists just after the location of the last character.
            while(temp[i]!='\0'){
                ++tempPtr; i++;
            }
            //loop that iterates till the null character of dir is encountered. 
            //On each iteration, the character stored at the jth location in the dir is appended to
            //the temp by using the statement *str1=*str2.
            while (dir[j]!='\0'){
                *tempPtr = *dirPtr;
                tempPtr++; dirPtr++; j++;
            }

            int retval = chdir(temp);//call chdir function with conctanated temp object.
            if (retval == 0)//if succeed.
            {
                int i = 0;
                while(directory[i]!='\0'){//update directory to temp
                    directory[i] = temp[i];
                    i++;
                }
            }else{perror("myshell> cd");}//else print error massege.

        }
    }
    else {//Update directory to home directory.
        char* home;
        home = getenv("HOME");
        chdir(home);
        int i = 0;
        while(directory[i]!='\0'){//Update directory to home directory.
                directory[i] = home[i];
                i++;
        }
    }

}

/*Helper for the readerHandler to determine background flag.*/
int  checkBG(char* token){
    if (strcmp(token, "&") == 0 )//String comparasion for background flag.
        {
            return 1;
        }
    return 0;
}

/*Parses user input to tokens according to the white spaces. It checks one of the tokens represents background symbol & for non builtin command and update flag. Parsed inputs stored in command aray.*/
void readerHandler(){
    
    char* token = strtok(iCommand," ");//First token of the input command.
    int i = 0;
    commandArray[i] = token;   // Assignment of the index 0 to first token.
    while( token != NULL ) {//A loop for tokenizing.
        bg = checkBG(token); //Check background flag
        i++;//Incriment index.
        token = strtok(NULL, " ");//Next token
        commandArray[i] = token;//Assignment of the index i to token.  
    }
    
}

/*This funtion takes user input with the help of fgets.*/
void reader(){
    printf("myshell>");
    fgets(iCommand,ARGUMENTLENGHT,stdin);
    
    if (strlen(iCommand) != 1){//If input is not empty.
        iCommand[strlen(iCommand)-1] = 0;//Avoiding null char in the stdin. 
        enqueArgumentHistory(iCommand); //Add history FIFO structure.
        readerHandler();//Call handler for parsing.
    }else{//If input is empty. Function calls it self.
        reader();
    }
   
}

/*This function called exit(0) command when user type bye.*/
void bye(){
    exit(0);
}

/*The main function handles all operation in the shell like program. Firstly it calls reader for input
after some operation to the handled input. I take first argument in the command array to determine 
which command will be executed with case sensitive strcasecmp funtion. If funtion is not build in 
process fork it self perform nonbuiltin operation.*/
int main(int argc, char const *argv[])
{
    while (1){

        reader();       
        char* cmd =commandArray[0];// entered command type e.g. dir, cd, ls or gedit.
    
        if(strcasecmp(cmd,"cd") == 0){//if command is cd call builtin change directory function.
            changeDirectory(commandArray[1]);

        }else if(strcasecmp(cmd,"history") == 0){//uf command is history call builtin history function.
            history();
            
        }else if(strcasecmp(cmd,"dir") == 0){// if command is dir call builtin printWorkingDirectory function.
            printWorkingDirectory();

        }else if(strcasecmp(cmd,"bye") == 0){//if command is bye call bye function
            bye();
        }
        else{//if command is not build in.
            /*
                This part of the code takes arguments and fork it self. If argument list contains
                & it handles for bakcground operation.
            */

            char* notBuiltInCommand = cmd;// assign command for execvp
            char* argList[]={0};//create arg list for execvp
            int status;//create status for waitpid.

            int i =0;
            for(i; i<sizeof(commandArray); i++){//create loop for takin commandarray to argList.
                argList[i] = commandArray[i];               
               
            }
            argList[10]=NULL;//Assign last element to the NULL for execvp.    
        
            pid_t proccess_id = fork();//call fork .
            if(proccess_id == 0){//if child process
               
                if(execvp(notBuiltInCommand,argList) == -1){//execute command with execvp if not succeed. Print  error.
                    perror("Nonbuilt in error");
                }
              
            }
            else if(proccess_id >0 ) {//If parent process
                
                //check command contains background symbol
                if (bg == 0){//if contains
                    bg = 0; //assing background flag to 0
                    waitpid(proccess_id, &status, 0);//does not wait child for background process.
                   
                }
            }
            else {perror("Fork goes wrong!");}//Print error if fork goes wrong.
        }
            
    }
    
    return 0;
}

