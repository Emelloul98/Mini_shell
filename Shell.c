// final ex2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
void deleteSpaces(char**);
#define MAX_INPUT_LENGTH 511// 510 chars + /0
#define MAX_LINE_LENGTH 512 // 510 chars + /0 + /n
#define MAX_ARGS_LENGTH 10
#define NUMBER_OF_EXIT 3
#define bool int
#define true 1
#define false 0
pid_t last_stopped_pid;
pid_t pid=-1;
bool cont_z=false;
bool bg_process=false;
// a dynamic struct for easily save the variables input
typedef struct Node
{
    char* var; // the var name
    char* command; // the var meaning
    struct Node* next; // a pointer to the next var that was saved.
}Node;
// A function that insert a new Node to the list
void insert(Node** head, char** newVar, char** newCommand)
{
    Node* curr = *head;
    Node* last; // the pointer that will save the last Node on the list
    while (curr!=NULL)
    {
        if (strcmp(curr->var,*newVar) ==0) // if the name of var already exist
        {
            curr->command = strdup(*newCommand); // changing the var meaning instead of creating a new one.
            return;
        }
        last=curr;
        curr=curr->next;
    }
    Node* newNode=(Node*)malloc(sizeof(Node));
    if(newNode==NULL)
    {
        printf("ERR\n");
        exit(EXIT_FAILURE);
    }
    //creating a new node
    newNode->var=strdup(*newVar);
    newNode->command=strdup(*newCommand);
    newNode->next=NULL;
    // if the list was empty:
    if(*head==NULL) *head=newNode;
    else last->next=newNode;
}
// a function that searches if the input var exist
char* search(Node** const head,char* varSearch)
{
    Node* curr=*head;
    while(curr!=NULL)
    {
        if(strcmp(curr->var,varSearch) == 0) // if there is a match between the variables names
            return curr->command; // returns this var meaning
        curr=curr->next;
    }
    return ""; // when there was no var with this name.
}
// only when the NUMBER_OF_EXIT times of enter sign was entered by the user
void deleteList(Node** head)
{
    Node* curr=*head;
    Node* next;
    // the loop is saving the next Node,and free his father until the end of the list
    while(curr!=NULL)
    {
        next=curr->next;
        free(curr->var);
        free(curr->command);
        free(curr);
        curr=next;
    }
}
// a function that gets a word and if there is a dollar sign in it, changing him to his saved meaning
bool dollarCheck(Node** head,char** word)
{
    char* dollarPtr=strchr(*word,'$');
    if(dollarPtr!=NULL)
    {
        int i=0;
        char* before="",* after="",* temp;
        char* varToSend=(char*)calloc(MAX_INPUT_LENGTH,sizeof(char ));
        char* res=(char*)calloc(MAX_INPUT_LENGTH,sizeof(char));
        temp=*word;
        if(*temp!='$')
        {
            before= strtok(temp,"$");
        }
        temp=dollarPtr+1;
        while(*temp!='\0' && *temp!='"')
        {
            varToSend[i]=*temp;
            temp++;
            i++;
        }
        after=temp;
        char* trans=search(head, varToSend); //calling the search function
        strcat(res, before);
        if(strcmp(trans,"")!=0)
        {
            strcat(res, trans);
        }
        strcat(res, after);
        *word=res;
        free(varToSend);
        return true;
    }
    return false;
}

// a function that checks if there is an equal sign in the command and calls the insert function
bool equalCheck(Node** head,char* command) {
    if(strncmp(command,"echo",4)==0 || strchr(command,'=')==NULL) // echo or no equals sign
    {
        return false;
    }
    char* cmdCopy=(char*)malloc(strlen(command)+1);
    if(cmdCopy==NULL)
    {
        printf("ERR\n");
        deleteList(head);
        exit(EXIT_FAILURE);
    }
    strcpy(cmdCopy,command);
    deleteSpaces(&cmdCopy); // closing the spaces to only: var=command
    char *equalInd = strchr(cmdCopy, '=');
    char *left;
    char *right;
    // splits the command to two separate words: before the equals sign, and after it
    left = strtok(cmdCopy, "=");
    right = equalInd+1;
    insert(head, &left, &right); // creating a new var with the insert function
    free(cmdCopy);
    return true; // returns to the main that there is noting else to do-continue to the next command
}
// a function that deletes the spaces before and after a command that includes the sign equals
void deleteSpaces(char **cmd) {
    // the "before" spaces: while the char is equal to space,the pointer will continue one char forward
    while (isspace(**cmd))
    {
        (*cmd)++;
    }
    if (**cmd == '\0') return;
    char *end = *cmd + strlen(*cmd) - 1;
    // the "after" spaces: while the char is equal to space,the pointer will continue one char backward
    while (end > *cmd && isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';
}
// a function that changes a word(by reference) to the same word but with no apostrophes at all
bool deleteApostrophes(char** word,bool* oneWord) {
    if(*word==NULL) return false;
    char* apostrophes = strchr(*word, '"');
    if (apostrophes == NULL) {
        return false; // when there are no apostrophes in the word
    }
    int count = 0;
    char* readPtr = *word;
    char* writePtr = *word;
    while (*readPtr != '\0') {
        if (*readPtr != '"') { // if current character is not a double quote
            *writePtr++ = *readPtr; // copy character to write pointer and changing *word also
        } else {
            count++; // increment count if character is a double quote
        }
        readPtr++;
    }
    *writePtr = '\0'; // add null terminator to the new string
    if (count%2==0)
    {
        *oneWord = true;
    }
    return true; // when there was 1 or more apostrophes in the word
}
// a normal str_tok function that I added her the ability to count how much spaces there were inside an apostrophes
// the function is comparing the chars in the command using their ascii value
char* strTokImplement(char *s, char* delim, int* sumOfSpaces)
{
    char* delimAsciiNum;
    int currAsciiNum,sc,numOfSpaces=1;
    char *tok;
    static char* last; // a pointer that will save the last char that the str_tok was working on
    if (s == NULL && (s = last) == NULL) // when there were no command at all
        return NULL;
    cont: // a label that the code can go back to him by using " goto "
    currAsciiNum = (unsigned char)*s++; //the ascii value of the next char
    for (delimAsciiNum = (char *)delim; (sc = (unsigned char)*delimAsciiNum++) != 0;) {
        if (currAsciiNum == sc)
        {
            numOfSpaces++; //counting how much spaces was deleted
            goto cont;

        }
    }
    if(sumOfSpaces!=NULL)
    {
        *sumOfSpaces=numOfSpaces; // puts the spaces value inside the int that passed to the function by reference
    }
    if (currAsciiNum == 0)   /* no non-delimiter characters */
    {
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    for (;;) {
        currAsciiNum = (unsigned char)*s++;
        delimAsciiNum = (char *)delim;
        do {
            if ((sc = (unsigned char)*delimAsciiNum++) == currAsciiNum)
            {
                // if the current chars ascii number equals the limit ascii number
                if (currAsciiNum == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return tok;
            }
        } while (sc != 0);
    }
}
char* str_tok_r_implement(char *str, const char *myDelim, char **last,bool* ampersand,bool* pipe,bool* err) {
    char* tok;
    int inside_apostrophe = 0; // Flag to keep track of whether we're inside an apostrophe
    if (str == NULL) {
        str = *last;
    }
    // Skip leading delimiters
    str += strspn(str, myDelim);

    if(*err==true)
    {
        if(strcmp(str,"")!=0) {
            while (*str != *myDelim && *str!='\0') {
                str++;
            }
            str++;
        }
        else *str='\0';
    }
    *err=false;
    // If we have reached the end of the string, return null
    if (*str == '\0') {
        return NULL;
    }
    // Find the end of the token
    tok = str;
    while (*str != '\0') {
        if (*str == '"' && *(str-1) != '\\') { // Check if we've entered or exited an apostrophe
            inside_apostrophe = !inside_apostrophe;
        } else if (!inside_apostrophe && strchr(myDelim, *str)) {
            // If we're not inside an apostrophes, and we've found a delimiter, break out of the loop
            break;
        }
        else if(!inside_apostrophe && *str=='|')
        {
            *pipe=true;
            break; //cuts the word to the pipe son instead of to the delimiter.
        }
        str++;
    }

    if (*str == '\0') { // If we've reached the end of the string, update last to point to the end of the string
        *last = str;
    } else {
        *str = '\0'; // Replace the delimiter with a null character to terminate the current token
        *last = str + 1; // Update last to point to the start of the next token
    }
    deleteSpaces(&tok);
    if(tok[strlen(tok)-1]=='&')
    {
        tok[strlen(tok)-1]='\0';
        *ampersand=true;
    }
    return tok;
}
void childExit(int res)
{
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0);
    // frees only the sons that finished (terminated..)
}
bool writeToFile(char** fileName, char** cmd)
{
    char* ptr= strchr(*cmd,'>'); //when there is a < sign
    if(ptr==NULL) return false;
    *fileName=strtok(ptr+1,";"); // reads name file from ptr to fileName by reference
    *ptr='\0';
    deleteSpaces(fileName);// deletes all head and backspaces
    return true;
}
void ctrl_z_click(int x)
{
    if(pid== -1 || bg_process) return;
    cont_z=true;
    last_stopped_pid=pid;// saving the last pid of the son that was stopped.
    kill(last_stopped_pid,SIGSTOP); // sends a stop signal to the son
}
int main() {
    signal(SIGCHLD,childExit);
    signal(SIGTSTP,ctrl_z_click);
    bool isInApostrophes=false,boolStat=false,isOneWord=false,freeWord=false,isAmpersand=false,isPipe=false,is_bg=false;
    bool err=false;
    Node *head = NULL;
    char input[MAX_LINE_LENGTH], buffer[MAX_INPUT_LENGTH],sp=' ';
    char* args[MAX_ARGS_LENGTH+1], *word, *cmd, *saved,*fileName;
    int numOfCmd=0,argsIndex=0,status1,numOfArgs=0,exitCounter=0, currentArgs=0,numOfSpaces=0,numOfPipes=0;
    int pipeFd[2];
    char* dir = getcwd(buffer, MAX_INPUT_LENGTH);
    if(dir==NULL)
    {
        perror("ERR");
        exit(EXIT_FAILURE);
    }
    int origin_in = dup(STDIN_FILENO);
    while (1) {
        printf("#cmd:%d|#args:%d@%s>", numOfCmd, numOfArgs, dir); // prints the prompt line
        fgets(input, MAX_LINE_LENGTH, stdin);//gets a command from the user
        bg_process=false;
        if (strcmp(input, "\n") == 0) // if there was an enter press 3 times-exit.
        {
            exitCounter++;
            if (exitCounter == NUMBER_OF_EXIT)
            {
                deleteList(&head);
                exit(0);
            }
            continue;
        }
        exitCounter = 0;
        // if the input in bigger than 510: print an error
        if(input[strlen(input)-1]!='\n')
        {
            while(getchar()!='\n'){} // delete all the extra chars that are waiting until the enter sign.
            // outside the loop:
            printf("ERR\n"); // too many arguments
            continue;
        }
        input[strlen(input)-1] = '\0';
        cmd = str_tok_r_implement(input, ";", &saved,&isAmpersand,&isPipe,&err); // separates the input to commands
        while (cmd != NULL) {
            if(isPipe) numOfPipes++;
            bool toFile= writeToFile(&fileName, &cmd);
            bool res = equalCheck(&head, cmd);
            if (res == false) // if this is not an equals command:
            {
                word = strTokImplement(cmd, &sp,&numOfSpaces); // separates the command to words
                while (word != NULL) {
                    freeWord=dollarCheck(&head,&word);
                    isInApostrophes=deleteApostrophes(&word,&isOneWord); // deletes the apostrophes
//                    dollarCheck(&head,&word); // double check for this case: echo $var" $var " .
                    // checks if there is a dollar sign in the word:
                    if(word==NULL) // when there is no such variable saved
                    {
                        if(argsIndex==0) numOfCmd--;
                        break;
                    }
                    if(strlen(word)>505)
                    {
                        //when there are too many chars in the var:
                        printf("ERR\n");
                        goto reset;
                    }
                    if(strncmp(word,"bg",2)==0)
                    {
                        is_bg=true;
                        numOfArgs++;
                    }
                    if ((strncmp(word, "cd",2)!= 0 || argsIndex!=0) && is_bg==false)
                    {
                        if(numOfSpaces>1 && boolStat==true) // if the word is in an apostrophes
                        {
                            int size=(int)(strlen(word))+1+numOfSpaces; // /0 +spaces + the original word
                            char* result = malloc(sizeof(char)*size);
                            if(result==NULL)
                            {
                                printf("ERR");
                                deleteList(&head);
                                exit(EXIT_FAILURE);
                            }
                            int index=0;
                            // wright the spaces that was deleted by str_tok:
                            for(int i=0;i<numOfSpaces;i++)
                            {
                                result[i]=' ';
                            }
                            // and cooping the rest of the string:
                            for(int j=numOfSpaces;j<size;j++)
                            {
                                result[j]=word[index];
                                index++;
                            }
                            word=strdup(result); //return the result to word
                            freeWord=true;
                            free(result);
                        }
                        // if there is just one word in the apostrophes or not in an apostrophes at all:
                        if((boolStat==false && isInApostrophes==false) || isOneWord==true)
                        {
                            args[argsIndex]=strdup(word);
                            currentArgs++;
                            argsIndex++;
                            isOneWord=false;
                        }
                            // if this is the first word how have an apostrophes:
                        else if(boolStat==false) // && isInApostrophes==true
                        {
                            args[argsIndex] = strdup(word);
                            boolStat = true; // changing the status1 to in an apostrophes until now
                        }
                        else // if the word is in an apostrophes
                        {
                            char *temp = (char *) malloc(sizeof(char) * MAX_INPUT_LENGTH);
                            if (temp == NULL) {
                                printf("ERR");
                                deleteList(&head);
                                exit(EXIT_FAILURE);
                            }
                            // linking the word to the rest of the words in the apostrophes
                            sprintf(temp, "%s %s", args[argsIndex], word);
                            free(args[argsIndex]);
                            args[argsIndex] = strdup(temp); // to the same sell in the args array
                            // if it's a closing apostrophes:
                            if (isInApostrophes == true) // && boolStat==true
                            {
                                currentArgs++;
                                argsIndex++;
                                boolStat = false; // closing the status1 back to not in an apostrophes
                            }
                            free(temp);
                        }
                    } else if(strncmp(word, "cd",2)== 0) // if the word was cd:
                    {
                        numOfCmd--;
                        printf("cd not supported\n");
                    }
                    if(freeWord)
                    {
                        free(word);
                        freeWord=false;
                    }
                    numOfSpaces=0; // resets the counter
                    word = strTokImplement(NULL, &sp, &numOfSpaces);
                    //separates the next word from the command
                }
                // if there is too many arguments:
                if(argsIndex>MAX_ARGS_LENGTH ||(argsIndex>MAX_ARGS_LENGTH-2 && toFile)||(argsIndex>MAX_ARGS_LENGTH-1 && isAmpersand))
                {
                    err=true;
                    printf("ERR\n");
                    goto reset;
                }
                args[argsIndex] = NULL; // cleaning all the cells that we didn't use in args
                if (pipe(pipeFd) == -1)
                {
                    perror("ERR");
                    exit(EXIT_FAILURE);
                }
                pid = fork(); // creating a son.
                if (pid < 0) // if there was an error to create a new son:
                {
                    perror("ERR");
                    deleteList(&head);
                    exit(EXIT_FAILURE);

                } else if (pid == 0) // the son will run the new code with the users commands
                {
                    if(toFile)// == true
                    {
                        int fd = open(fileName, O_WRONLY| O_CREAT| O_TRUNC, 0666); // opens a file
                        if(fd<0)
                        {
                            perror("ERR");
                            deleteList(&head);
                            exit(EXIT_FAILURE);
                        }
                        if(dup2(fd,STDOUT_FILENO)==-1) // redirect the output to the screen to the file.
                        {
                            perror("ERR");
                            deleteList(&head);
                            exit(EXIT_FAILURE);
                        }
                        close(fd);
                    }
                    else if(isPipe)
                    {
                        close(pipeFd[0]);
                        close(STDOUT_FILENO);
                        if(dup2(pipeFd[1],STDOUT_FILENO)==-1)// the out of the son going to the pipe input.
                        {
                            perror("ERR");
                            deleteList(&head);
                            exit(EXIT_FAILURE);
                        }
                    }
                    if(execvp(args[0], args)==-1) {
                        perror("ERR");
                        deleteList(&head);
                        exit(EXIT_FAILURE);
                    }
                } else // the father
                {
                    numOfCmd++;
                    numOfArgs += (currentArgs - res);
                    if(toFile) numOfArgs+=2;
                    if (isPipe)
                    {
                        close(pipeFd[1]);
                        dup2(pipeFd[0],STDIN_FILENO); //redirect the pipe out to the next cmd in
                        close(pipeFd[0]);
                    }
                    if(isAmpersand == false) // if there is no & sign:
                    {
                        waitpid(pid,&status1,WUNTRACED);
                        if (WEXITSTATUS(status1)!=0 && !cont_z)
                        {
                            err=true;
                        }
                    }
                    else
                    {
                        bg_process=true;
                        numOfArgs++;
                    }
                    if(is_bg) // sends a continue signal to the last son that was stopped
                    {
                        kill(last_stopped_pid,SIGCONT);
                    }
                    if(numOfPipes>0 && !isPipe) // in the last pipe case:
                    {
                        close(pipeFd[0]);
                        close(pipeFd[1]);
                        // closing the pipe
                        if(dup2(origin_in,STDIN_FILENO)==-1) // returns the input to his origin
                        {
                            perror("ERR");
                            deleteList(&head);
                            exit(EXIT_FAILURE);
                        }
                        numOfPipes = 0;
                    }
                }
            }
            reset: // resets all the counters to zero and false
            isAmpersand=false;
            boolStat=false;
            toFile=false;
            is_bg=false;
            cont_z=false;
            currentArgs=0;
            numOfSpaces=0;
            // frees the args array:
            for (int i = 0; i<argsIndex ; i++)
            {
                free(args[i]);
            }
            argsIndex=0;
            isPipe=false;
            cmd = str_tok_r_implement(NULL, ";", &saved,&isAmpersand,&isPipe,&err);
            // taking the next command that was entered by the user
        }

    }
}







