#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
/*This funtion basically resets a string .*/
void freeString(char* str, int size){
    for(int i =0; i<size; i++){
        str[i] = '\0';
    }
}
/*This function returns the number of procceses that are currentlu executing.*/
int getProcesses() {
    int pidArray[2048];

    int pipefd[2];
    pipe(pipefd) == -1;

    int pid = fork();


    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execlp("ps", "ps", "aux", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        FILE *fp = fdopen(pipefd[0], "r");

        char buffer[1024];
        int count = 0;

        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                count++;
        }

        fclose(fp);
        wait(NULL);

        return count;
    }
}

/*This function reverses the entire given string.*/
void reverseStrings(char *str) {
    char *start = str;
    char *end = str + strlen(str) - 1;

    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}    

/*Type struct to store Aliases.*/
typedef struct alias
{
    char key[256];
    char command[20][256];
}Alias;

/*This funtions frees allocated dynamic memory.*/
void freeMems(char** ops, int count){
    for (int fre = 0; fre<count; fre++){
                free(ops[fre]);
            }
}

void printAliasList(Alias* als, int count){
    for(int i = 0; i<count; ++i){
        printf("%s,", als[i].key);
    }
}
/*This function removes " and ' signs from a string, I used this for alias commands."*/
void removeQuotes(char *str) {
    int i, j;

    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] != '\'' && str[i] != '\"') {
            str[j++] = str[i];
        }
    }

    str[j] = '\0';
}

/*This function searches the PATH and checks if the parmeter command is in it.*/
int isCommandInPath(char *command) {
    char *path = getenv("PATH");
    if (path == NULL) {
        return -1;
    }

    char *pathCopy = strdup(path);
    char *token = strtok(pathCopy, ":"); //The path is seperated with : symbols.

    while (token != NULL) {
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", token, command);

        if (access(fullPath, X_OK) == 0) {
            free(pathCopy);
            return 1;
        }

        token = strtok(NULL, ":");
    }
    free(pathCopy);
    return 0;
}
/*Checks if a command is defined with alias or not.*/
int isAlias(char* command, Alias* alList, int alCount){
    for (int i =0; i<alCount; i++){
        if(strcmp(command, alList[i].key) == 0){
            return i;
        }       
    }
    return -1;
}

/*I used this function for debugging purposes. It prints an array.*/
void printStringArray(char* strings[]) {
    if (strings == NULL || strings[0] == NULL) {
        printf("{}\n");
        return;
    }

    printf("{\"%s\"", strings[0]);

    for (int i = 1; strings[i] != NULL; ++i) {
        printf(", \"%s\"", strings[i]);
    }

    printf("}\n");
}

/*This function handles the redirect operations. It uses pipes to transfer output
from execvp function to file. The opName parameter determines what 
redirection is being used. */
void redirectOps(char** command, char* file, int opName ,int count, Alias* alList, int alCount, char** lastExec){
    if(opName == 0){
        FILE* fp = fopen(file, "w");
        int isAl = isAlias(command[0], alList, alCount);
        int procCount = 0;
        if(isCommandInPath(command[0]) != 1  && isAl == -1 && strcmp(command[0], "bello") != 0){
            printf("Command not found in PATH: %s\n", command[0]);
            freeMems(command, count);
            fclose(fp);
            return;
        }
        if(strcmp(command[0], "bello") == 0){
            procCount = getProcesses();
        }


        int pipe_fd[2];
        char buffer[4096];
        freeString(buffer, 4096);
        pipe(pipe_fd);
        
        

        pid_t id = fork();
        if(id == 0){
            
            if(strcmp(command[0], "bello") == 0 && isAl == -1){
                close(pipe_fd[0]);
                if(command[1] != NULL){
                    printf("%s\n","Invalid argument(s)");
                    exit(EXIT_SUCCESS);
                }
                char* tty = ttyname(STDIN_FILENO);
                char *username = getenv("USER");
                char hostname[1024];
                hostname[1023] = '\0';
                gethostname(hostname, 1024);
                char* shell = getenv("SHELL");
                char* home = getenv("HOME");
                time_t t = time(NULL);
                char* time = ctime(&t);
                char* nlR = strchr(time, '\n');
                *nlR = '\0';
                char lastExed[256];
            char proccessCount[20];
            sprintf(proccessCount, "%d", procCount);
            if(*lastExec[0] == '\0'){
                strcpy(lastExed,"NULL");
            }
            else{
                strcpy(lastExed,lastExec[0]);
            }
            char* strings[] = {username, hostname,lastExed, tty, shell, home, time, proccessCount};
            free(lastExec[0]);
            lastExec[0] = malloc(256);
            strcpy(lastExec[0], "bello");
                for (int i = 0; strings[i] != NULL; ++i) {
                    size_t len = strlen(strings[i]);
                    write(pipe_fd[1], strings[i], len);

                    // Write a newline after each string
                    write(pipe_fd[1], "\n", 1);
                }
                close(pipe_fd[1]);
            
            }                        
            else{
                close(pipe_fd[0]);
                dup2(pipe_fd[1], 1);
                dup2(pipe_fd[1], 2);
                close(pipe_fd[1]);
                if(isAl != -1){
                    char* tempCmd[256];
                   free(command[0]);
                   for (int i= 1; i<count; i++){
                        tempCmd[i] = malloc(256);
                        strcpy(tempCmd[i], command[i]);
                        free(command[i]);
                   }

                   int alX = 0;
                   char* alCmd = malloc(256);
                   strcpy(alCmd,alList[isAl].command[alX]);
                   while (alCmd[0] != '\0')
                   {
//                    printf("%d\n", alX);
                     command[alX] = malloc(256);                     
                     strcpy(command[alX], alCmd);
                     alX++;
                     strcpy(alCmd,alList[isAl].command[alX]);
                     
                   }
                   alX--;
                   free(alCmd);
                   for (int k=1; k<count; k++){
                     command[alX+k] = malloc(256);
                     strcpy(command[alX+k], tempCmd[k]);
                     free(tempCmd[k]);
                   }
                   count = count + alX -1;
                }
                execvp(*command, command);
                if(*lastExec[1] ==  '\0'){
                    free(lastExec[0]);
                    lastExec[0] = malloc(256);
                }
                else{
                    char* tSwitch = malloc(256);
                    strcpy(tSwitch,lastExec[1]);
                    free(lastExec[0]);
                    free(lastExec[1]);
                    lastExec[0] = malloc(256);
                    lastExec[1] = malloc(256);
                    strcpy(lastExec[0],tSwitch);
                    free(tSwitch);
                }
                exit(EXIT_SUCCESS);
            }
        }
        else{
            int status;
            close(pipe_fd[1]);
            ssize_t bytes_read;
            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0){
                fwrite(buffer, 1, bytes_read, fp) ;
            }
            fclose(fp);
        
            close(pipe_fd[0]);
            waitpid(id, &status, 0);
            for (int fre = 0; fre<count; fre++){
                free(command[fre]);
            }

            
        }

    }
    else if(opName == 1){
        FILE* fp = fopen(file, "a");
        int isAl = isAlias(command[0], alList, alCount);
        int procCount = 0;
        if(isCommandInPath(command[0]) != 1 && isAl == -1 && strcmp(command[0], "bello") != 0){
            printf("Command not found in PATH: %s\n", command[0]);
            freeMems(command, count);
            fclose(fp);
            return;
        }
        if(strcmp(command[0], "bello") == 0){
            procCount = getProcesses();
        }

        int pipe_fd[2];
        char buffer[4096];
        freeString(buffer, 4096);
        pipe(pipe_fd);
        


        pid_t id = fork();
        if(id == 0){
            
            if(strcmp(command[0], "bello") == 0 && isAl == -1){
            close(pipe_fd[0]);
            if(command[1] != NULL){
                printf("Invalid argument(s)\n");
                freeMems(command, count);
                return;
            }
            char* tty = ttyname(STDIN_FILENO);
            char *username = getenv("USER");
            char hostname[1024];
            hostname[1023] = '\0';
            gethostname(hostname, 1024);
            char* shell = getenv("SHELL");
            char* home = getenv("HOME");
            time_t t = time(NULL);
            char* time = ctime(&t);
            char* nlR = strchr(time, '\n');
                *nlR = '\0';
            char lastExed[256];
            char proccessCount[20];
            sprintf(proccessCount, "%d", procCount);
            if(*lastExec[0] == '\0'){
                strcpy(lastExed,"NULL");
            }
            else{
                strcpy(lastExed,lastExec[0]);
            }
            char* strings[] = {username, hostname,lastExed, tty, shell, home, time, proccessCount};
            free(lastExec[0]);
            lastExec[0] = malloc(256);
            strcpy(lastExec[0], "bello");
            for (int i = 0; strings[i] != NULL; ++i) {
                size_t len = strlen(strings[i]);
                write(pipe_fd[1], strings[i], len);

                // Write a newline after each string
                write(pipe_fd[1], "\n", 1);
            }
            close(pipe_fd[1]);
            
        }                        
            else{
                close(pipe_fd[0]);
                dup2(pipe_fd[1], 1);
                dup2(pipe_fd[1], 2);
                close(pipe_fd[1]);
                if(isAl != -1){
                    char* tempCmd[256];
                   free(command[0]);
                   for (int i= 1; i<count; i++){
                        tempCmd[i] = malloc(256);
                        strcpy(tempCmd[i], command[i]);
                        free(command[i]);
                   }

                   int alX = 0;
                   char* alCmd = malloc(256);
                   strcpy(alCmd,alList[isAl].command[alX]);
                   while (alCmd[0] != '\0')
                   {
//                    printf("%d\n", alX);
                     command[alX] = malloc(256);                     
                     strcpy(command[alX], alCmd);
                     alX++;
                     strcpy(alCmd,alList[isAl].command[alX]);
                     
                   }
                   alX--;
                   free(alCmd);
                   for (int k=1; k<count; k++){
                     command[alX+k] = malloc(256);
                     strcpy(command[alX+k], tempCmd[k]);
                     free(tempCmd[k]);
                   }
                   count = count + alX -1;
                }
                execvp(*command, command);
                if(*lastExec[1] ==  '\0'){
                    free(lastExec[0]);
                    lastExec[0] = malloc(256);
                }
                else{
                    char* tSwitch = malloc(256);
                    strcpy(tSwitch,lastExec[1]);
                    free(lastExec[0]);
                    free(lastExec[1]);
                    lastExec[0] = malloc(256);
                    lastExec[1] = malloc(256);
                    strcpy(lastExec[0],tSwitch);
                    free(tSwitch);
                }
                exit(EXIT_SUCCESS);
            }
        }
        else{
            int status;
            close(pipe_fd[1]);
            char* tempLast = malloc(256);
                char* insure = malloc(256);
                int noLast = *lastExec[0] == '\0';
                if(!noLast){
                    strcpy(insure, lastExec[0]);
                }
                strcpy(tempLast, command[0]);
                for (int last=1; last<count; last++){
                    strcat(tempLast, " ");
                    strcat(tempLast, command[last]);
                }
//                printf("%s\n", tempLast);
                free(lastExec[0]);
                lastExec[0] = malloc(256);
                strcpy(lastExec[0], tempLast);
//                printf("%s\n", lastExec[0]);
                if(!noLast){
                    strcpy(lastExec[1], insure);
                }
                free(tempLast);
                free(insure);
            ssize_t bytes_read;
            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0){
                fwrite(buffer, 1, bytes_read, fp) ;
            }
            fclose(fp);
        
            close(pipe_fd[0]);
            waitpid(id, &status, 0);
            for (int fre = 0; fre<count; fre++){
                free(command[fre]);
            }

            
        }

    }
    else{
        FILE* fp = fopen(file, "a");
        int isAl = isAlias(command[0], alList, alCount);
        int procCount = 0;
        if(isCommandInPath(command[0]) != 1 && isAl == -1 && strcmp(command[0], "bello") != 0){
            printf("Command not found in PATH: %s\n", command[0]);
            freeMems(command, count);
            fclose(fp);
            return;
        }
        if(strcmp(command[0], "bello") == 0){
            procCount = getProcesses();
        }

        int pipe_fd[2];
        char buffer[4096];
        freeString(buffer, 4096);
        pipe(pipe_fd);


        pid_t id = fork();
        if(id == 0){
            int isAl = isAlias(command[0], alList, alCount);
            if(strcmp(command[0], "bello") == 0 && isAl == -1){
            close(pipe_fd[0]);
            if(command[1] != NULL){
                printf("Invalid argument(s)\n");
                return;
            }
            char* tty = ttyname(STDIN_FILENO);
            char *username = getenv("USER");
            char hostname[1024];
            hostname[1023] = '\0';
            gethostname(hostname, 1024);
            char* shell = getenv("SHELL");
            char* home = getenv("HOME");
            time_t t = time(NULL);
            char* time = ctime(&t);
            char* nlR = strchr(time, '\n');
                *nlR = '\0';
            char lastExed[256];
            char proccessCount[20];
            sprintf(proccessCount, "%d", procCount);
            if(*lastExec[0] == '\0'){
                strcpy(lastExed,"NULL");
            }
            else{
                strcpy(lastExed,lastExec[0]);
            }
            char* strings[] = {username, hostname,lastExed, tty, shell, home, time, proccessCount};
            free(lastExec[0]);
            lastExec[0] = malloc(256);
            strcpy(lastExec[0], "bello");
            for (int i = 0; strings[i] != NULL; ++i) {
                size_t len = strlen(strings[i]);
                write(pipe_fd[1], strings[i], len);

                // Write a newline after each string
                write(pipe_fd[1], "\n", 1);
            }
            close(pipe_fd[1]);
            
        }                        
            else{
                close(pipe_fd[0]);
                dup2(pipe_fd[1], 1);
                dup2(pipe_fd[1], 2);
                close(pipe_fd[1]);
                if(isAl != -1){
                    char* tempCmd[256];
                   free(command[0]);
                   for (int i= 1; i<count; i++){
                        tempCmd[i] = malloc(256);
                        strcpy(tempCmd[i], command[i]);
                        free(command[i]);
                   }

                   int alX = 0;
                   char* alCmd = malloc(256);
                   strcpy(alCmd,alList[isAl].command[alX]);
                   while (alCmd[0] != '\0')
                   {
//                    printf("%d\n", alX);
                     command[alX] = malloc(256);                     
                     strcpy(command[alX], alCmd);
                     alX++;
                     strcpy(alCmd,alList[isAl].command[alX]);
                     
                   }
                   alX--;
                   free(alCmd);
                   for (int k=1; k<count; k++){
                     command[alX+k] = malloc(256);
                     strcpy(command[alX+k], tempCmd[k]);
                     free(tempCmd[k]);
                   }
                   count = count + alX -1;
                }
                execvp(*command, command);
                printf("Invalid argument(s)\n");
                if(*lastExec[1] ==  '\0'){
                    free(lastExec[0]);
                    lastExec[0] = malloc(256);
                }
                else{
                    char* tSwitch = malloc(256);
                    strcpy(tSwitch,lastExec[1]);
                    free(lastExec[0]);
                    free(lastExec[1]);
                    lastExec[0] = malloc(256);
                    lastExec[1] = malloc(256);
                    strcpy(lastExec[0],tSwitch);
                    free(tSwitch);
                }
                exit(EXIT_SUCCESS);
            }
        }
        else{
            int status;
            close(pipe_fd[1]);
            char* tempLast = malloc(256);
                char* insure = malloc(256);
                int noLast = *lastExec[0] == '\0';
                if(!noLast){
                    strcpy(insure, lastExec[0]);
                }
                strcpy(tempLast, command[0]);
                for (int last=1; last<count; last++){
                    strcat(tempLast, " ");
                    strcat(tempLast, command[last]);
                }
//                printf("%s\n", tempLast);
                free(lastExec[0]);
                lastExec[0] = malloc(256);
                strcpy(lastExec[0], tempLast);
//                printf("%s\n", lastExec[0]);
                if(!noLast){
                    strcpy(lastExec[1], insure);
                }
                free(tempLast);
                free(insure);
            ssize_t bytes_read;
            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0){
                reverseStrings(buffer);
                fwrite(buffer, 1, bytes_read, fp) ;
            }
            fclose(fp);
        
            close(pipe_fd[0]);
            waitpid(id, &status, 0);
            for (int fre = 0; fre<count; fre++){
                free(command[fre]);
            }
            
        }

    }
    return;
}



/*This function handles simple executions by forking a child
and making the child execute the command.*/
void handleExec(char** ops, int count, Alias* alList, int alCount, int background, char** lastExec){
    int isAl = isAlias(ops[0], alList, alCount);
    if(isCommandInPath(ops[0]) != 1 && isAl == -1 && strcmp(ops[0], "bello") != 0){
            printf("Command not found in PATH: %s\n", ops[0]);
            freeMems(ops, count);
            return;
        }
    int proccessCount = 0;
    if(strcmp(ops[0], "bello") == 0){
        proccessCount = getProcesses();
    }
    pid_t id = fork();
        if(id == 0){

//            printf("%d\n", isAl);
            if(strcmp(ops[0], "bello") == 0 && isAl == -1){
                if(ops[1] != NULL){
                    printf("Invalid argument(s)\n");
                    return;
                }
                char* tty = ttyname(STDIN_FILENO);
                char *username = getenv("USER");
                char hostname[1024];
                hostname[1023] = '\0';
                gethostname(hostname, 1024);
                char* shell = getenv("SHELL");
                char* home = getenv("HOME");
                time_t t = time(NULL);
                char* time = ctime(&t);
                printf("%s\n", username);
                printf("%s\n", hostname);
                if(*(lastExec[0]) == '\0'){
                    printf("NULL\n");
                }
                else{
                    printf("%s\n",lastExec[0]);
                }
                printf("%s\n", tty);
                printf("%s\n", shell);
                printf("%s\n", home);
                printf("%s", time);
                printf("%d\n", proccessCount);
                free(lastExec[0]);
                lastExec[256] = malloc(256);
                strcpy(lastExec[0],"bello");
                exit(EXIT_SUCCESS);
            }
            else if(isCommandInPath(ops[0]) != 1 && isAl == -1){
                printf("Typed command is not in the PATH.\n");
                exit(EXIT_SUCCESS);
            }
            else{
                
                if(isAl != -1){
                    
                    char* tempCmd[256];
                   free(ops[0]);
                   for (int i= 1; i<count; i++){
                        tempCmd[i] = malloc(256);
                        strcpy(tempCmd[i], ops[i]);
                        free(ops[i]);
                   }

                   int alX = 0;
                   char* alCmd = malloc(256);
                   strcpy(alCmd,alList[isAl].command[alX]);
                   while (alCmd[0] != '\0')
                   {
//                    printf("%d\n", alX);
                     ops[alX] = malloc(256);                     
                     strcpy(ops[alX], alCmd);
                     alX++;
                     strcpy(alCmd,alList[isAl].command[alX]);
                     
                   }
                   alX--;
                   free(alCmd);
                   for (int k=1; k<count; k++){
                     ops[alX+k] = malloc(256);
                     strcpy(ops[alX+k], tempCmd[k]);
                     free(tempCmd[k]);
                   }
                   count = count + alX -1;
                }
//                printStringArray(ops);
                execvp(*ops, ops);
                printf("Invalid argument(s)\n");
                if(*lastExec[1] ==  '\0'){
                    free(lastExec[0]);
                    lastExec[0] = malloc(256);
                }
                else{
                    char* tSwitch = malloc(256);
                    strcpy(tSwitch,lastExec[1]);
                    free(lastExec[0]);
                    free(lastExec[1]);
                    lastExec[0] = malloc(256);
                    lastExec[1] = malloc(256);
                    strcpy(lastExec[0],tSwitch);
                    free(tSwitch);
                }
                exit(EXIT_SUCCESS);


            }
        }
        else{
            int stat;
            char* tempLast = malloc(256);
                char* insure = malloc(256);
                int noLast = *lastExec[0] == '\0';
                if(!noLast){
                    strcpy(insure, lastExec[0]);
                }
                strcpy(tempLast, ops[0]);
                for (int last=1; last<count; last++){
                    strcat(tempLast, " ");
                    strcat(tempLast, ops[last]);
                }
//                printf("%s\n", tempLast);
                free(lastExec[0]);
                lastExec[0] = malloc(256);
                strcpy(lastExec[0], tempLast);
//                printf("%s\n", lastExec[0]);
                if(!noLast){
                    strcpy(lastExec[1], insure);
                }
                free(tempLast);
                free(insure);

            if(!background){
                waitpid(id, &stat, 0); // Wait for child to complete if its not a background procces.
            }
            for (int fre = 0; fre<count; fre++){
                free(ops[fre]);
            }

            
        }

}
/*In main function inputs are taken with a while loop and fgets function.
In each iteration username, hostname and current dir is searched and stored.*/
int main(){
    char line[256];
    char* nullArg[] = {NULL};
    Alias alArray[128];
    char redFile[256];
    char* ops[256];
    int count = 0;
    char* lastExec[2];
    lastExec[0] = malloc(256);
    lastExec[1] = malloc(256);

    while(1){
        FILE* alFile = fopen("aliases", "a+");
        char alLine[256];
        freeString(alLine, 256);
        int lineCount = 0;
        freeString(redFile, 256);

        int redOp= -1;

        while(fgets(alLine,  256, alFile) != NULL){
            char* cmd = malloc(256);
            char* com = malloc(256);
            cmd = strtok(alLine, ",");
            if(cmd == NULL){
                free(com);
                break;
            }
            char* nl = strchr(cmd, '\n');
            if(nl != NULL){
                *nl = '\0';
            }
            strcpy(alArray[lineCount].key,cmd);
            cmd = strtok(NULL, ",");
            com = strtok(cmd, " ");
            int commandX = 0;
            while(com != NULL){
                nl = strchr(com, '\n');
                if(nl != NULL){
                    *nl = '\0';
                }
                strcpy(alArray[lineCount].command[commandX],com);
                com = strtok(NULL, " ");
                commandX++;
            }            
            free(com);

            lineCount++;
        }

        char *username = getenv("USER");
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1024);
        char dir[1024];
        dir[1023] = '\0';
        getcwd(dir, 1024);
        printf("%s@%s %s---", username, hostname, dir);
        freeString(line, 256);
        if(fgets(line, 256, stdin) == NULL){
            break;
        }
        char* space = " ";
        for (int i=0; i<256; i++){
            ops[i] = NULL;
        }
         int background =0;
         char* bck = strchr(line, '&');
        if(bck != NULL){
            *bck = '\0';
            background =1;
        }
        
        count = 0;
        char* red = strchr(line, '>');
        if(red != NULL){
            char* red_two = red+1;
            char* red_three = red+2;
            if(red_two == NULL){
                printf("%s\n", "Incorrect syntax, please enter your command like: cmd >> a.txt");
                freeMems(ops, count);
                fclose(alFile);
                continue;
            }
            if(*red_two == '>'){
                if(red_three != NULL && *red_three == '>'){
                    redOp = 2;
                    *red = '\0';
                    red++;
                    *red = '\0';
                    red++;
                    *red = '\0';
                    red++;
                    char* fn;
                    fn = strtok(red, " ");
                    char* newL = strchr(fn, '\n');
                    if(newL != NULL){
                        *newL = '\0';
                    }
                    strcpy(redFile, fn);
                    char* op = strtok(line, space);
                    while(op != NULL){
                    ops[count] = malloc(strlen(op) + 1);
                    char *newline = strchr(op, '\n');
                    if(newline != NULL){
                        *newline = '\0';
                    }
                    strcpy(ops[count], op);
                    count++;
                    op = strtok(NULL, " ");
                }
                }
                else{
                    redOp = 1;
                    *red = '\0';
                    red++;
                    *red = '\0';
                    red++;
                    char* fn;
                    fn = strtok(red, " ");
                    char* newL = strchr(fn, '\n');
                    if(newL != NULL){
                        *newL = '\0';
                    }
                    strcpy(redFile, fn);
                    char* op = strtok(line, space);
                    while(op != NULL){
                    ops[count] = malloc(strlen(op) + 1);
                    char *newline = strchr(op, '\n');
                    if(newline != NULL){
                        *newline = '\0';
                    }
                    strcpy(ops[count], op);
                    count++;
                    op = strtok(NULL, " ");
                }

                }
            }
            else{
                redOp = 0;
                *red = '\0';
                red++;
                char* fn;
                fn = strtok(red, " ");
                char* newL = strchr(fn, '\n');
                if(newL != NULL){
                    *newL = '\0';
                }
                strcpy(redFile, fn);
                char* op = strtok(line, space);
                while(op != NULL){
                ops[count] = malloc(strlen(op) + 1);
                char *newline = strchr(op, '\n');
                if(newline != NULL){
                    *newline = '\0';
                }
                strcpy(ops[count], op);
                count++;
                op = strtok(NULL, " ");
            }               

            }

        }
        else{
            char* op = strtok(line, space);

            while(op != NULL){
    //            printf("%s\n", op);
                ops[count] = malloc(strlen(op) + 1);
                char *newline = strchr(op, '\n');
                if(newline != NULL){
                    *newline = '\0';
                }
                strcpy(ops[count], op);
                count++;
                op = strtok(NULL, " ");
            }
        }
        if(strcmp(ops[0], "exit") == 0){
            break; //Exit the loop if the first word is exit.
        }
       
        else if(strcmp(ops[0], "alias") == 0){
            if(ops[1] == NULL){
                printf("%s\n", "Incorrect syntax, please enter your command like: alias x=cmd\n");
                for(int fre =0; fre<count; fre++){
                    free(ops[fre]);
                }
                fclose(alFile);
                continue;
            }
            char* equals = strchr(ops[1], '=');
            if(equals == NULL){
                printf("%s\n", "Incorrect syntax, please enter your command like: alias x=cmd\n");
                for(int fre =0; fre<count; fre++){
                    free(ops[fre]);
                }
                fclose(alFile);
                continue;
            }
            if(redOp != -1){
                switch (redOp)
                {
                case 0:
                    FILE *tempF_1 = fopen(redFile, "a");
                    fclose(tempF_1);

                    break;
                case 1:
                    FILE *tempF_2 = fopen(redFile, "a");
                    fclose(tempF_2);

                    break;
                case 2:
                    FILE *tempF_3 = fopen(redFile, "a");
                    fclose(tempF_3);

                    break;
                default:
                    break;
                }
            }
            char* keys = strtok(ops[1], "=");
            char key[256];
            char cmnd[256];
            freeString(key, 256);
            freeString(cmnd, 256);
            strcpy(key, keys);
            strcat(key, ",");
            keys = strtok(NULL, "=");

            strcpy(cmnd, keys);
            for(int x = 2; x<count; x++){
                strcat(cmnd, " ");
                strcat(cmnd, ops[x]);
            }
            removeQuotes(cmnd);
            strcat(key, cmnd);
            fprintf(alFile,"%s\n", key);
            for(int fre =0; fre<count; fre++){
                    free(ops[fre]);
            }
            fclose(alFile);
            continue;
        }
//        printStringArray(ops);
        fclose(alFile);
        
        if(redOp != -1){
            redirectOps(ops, redFile, redOp, count, alArray, lineCount, lastExec);
        }
        else{
            handleExec(ops, count, alArray, lineCount, background, lastExec);
        }
//        redirectOps(ops, "James.txt", 2, count, alArray, lineCount);
    }
    freeMems(ops, count);
    free(lastExec[0]);
    free(lastExec[1]);

    return 0;
}

