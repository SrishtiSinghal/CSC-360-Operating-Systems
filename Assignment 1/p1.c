//Srishti Singhal
//Programming Assignment 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_LINE_LENGTH 50

typedef struct bg_pro {
	pid_t pid;
	char* command[MAX_LINE_LENGTH];
	struct bg_pro* next;
} bg_process;

bg_process* root;

int printPrompt() {
	char cwd[PATH_MAX];	
  	char* username  = (char*)malloc(20);
	char hostname[30];

	username = getlogin();//System call to get the user name
	gethostname(hostname, sizeof(hostname)); //System call to get the host name

	if (getcwd(cwd, sizeof(cwd)) != NULL) { //If the current working directory is not null it prints the prompt.
       	printf("SSI: %s@%s: %s > ", username, hostname, cwd);
	} else {
		perror("getcwd() error"); 
		    return 1;
	}
  
  	return 0;
}


int changeDirectories(char* command[], int size) {	
	if(size > 1){
		char *dir = command[1];
		int ret;

		if(strcmp(command[1],"~") == 0){
			chdir(getenv("HOME"));
			return 0; 
		}
		ret = chdir(dir);
		return 0;
	}
	else{
		chdir(getenv("HOME"));
		return 0;
	}

	return 0;
}

bg_process* createNode(){
    bg_process* temp = malloc(sizeof(bg_process)); // allocate memory using malloc()
    temp->next = NULL;// make next point to NULL
    return temp;//return the new node
}

void copyStringArray(char* destination[], char* source[]) {
	int i = 0; 
  	while (source[i] != NULL) {
    	destination[i] = (char*)malloc(sizeof(source[i]));
    	strcpy(destination[i], source[i]);
    	i ++;
  	}
  	destination[i] = NULL;
}

void storeProcessDetails(pid_t pid, char* command[]) {
 	if (root == NULL) {
    	root = createNode();
    	root->pid = pid;
    	copyStringArray(root->command, command);
  	} else {
    	bg_process* newProcess = createNode();
    	newProcess->pid = pid;
    	copyStringArray(newProcess->command, command);
    	newProcess->next = root;  // prepend new node to root
    	root = newProcess; // update root
  	}
}


int processBackgroundCommand(char *command[]) {
 	if (command[0] == NULL) {
    	return 0;
 	}
  
 	pid_t pid = fork();

	if (pid < 0) {
		perror("Fork Failed");
		return 1;
	}
  
	if (pid == 0) {
		execvp(command[0], command);	
	} 
 	else {
	 	storeProcessDetails(pid, command);
 	}
  
	return 0;
}

int processGeneralCommand(char* command[]) {
  	pid_t pid;
	
  	pid = fork();

	if (pid < 0) {
		perror("Fork Failed");
		return 1;
	}
  
	if (pid == 0) {
		execvp(command[0], command);	
	} else {
	  waitpid(-1, NULL, 0);
	}
  
  	return 0;
}

int processBGListCommand() {
	int num_process = 0;
	bg_process* node = root;
 	while (node != NULL) {
	    printf("%d:", node->pid);
	    
	    char** command = node->command;
	    while (*command != NULL) {
	      printf(" %s", *command);
	      command ++;
	    }
	    printf("\n");
	    num_process++;
	    node = node->next;
  	}
  	printf("Total Background jobs: %d\n", num_process);
 	return 0; 
}

int checkBackgroundTerminations() {
	bg_process* temp;
	if(root == NULL);
	else if(root!= NULL){
		pid_t terminate = waitpid(0, NULL, WNOHANG);
		while(terminate > 0){
			if(root->pid == terminate){
				char** command  = root->command;
				printf("%d: ", root->pid);
				while(*command != NULL){
					printf("%s ", *command);
					command++;
				}
				printf("has terminated\n");
				root = root->next;
			}
			else{
				while(temp->next->pid != terminate){
					char** command = temp->next->command;
					printf("%d: ", temp->next->pid);
					while(*command != NULL){
						printf("%s ", *command);
						command++;
					}
					printf("has terminated\n");
					temp->next = temp->next->next;
				}
			}
		terminate = waitpid(0, NULL, WNOHANG);
		}
	}
 	return 0;
}

int processCommand() {
	char user_input[MAX_LINE_LENGTH];
	char *pointer;
	char *command[MAX_LINE_LENGTH];
	int i = 0; 

	fgets(user_input, MAX_LINE_LENGTH, stdin);
  

  	int err = checkBackgroundTerminations();
	if (err != 0) {
	    return err;
	}


	if (strcmp(user_input, "exit\n") == 0) {
    	return 2;
 	}

	pointer = strtok(user_input, " \n");

	while(pointer !=NULL) {
		command[i++] = pointer;
		pointer = strtok(NULL, " \n");
	} 	
	
	command[i] = NULL;
 
	if (command[0] == NULL) {
		return 0;
  	}

	if (strcmp(command[0], "cd") == 0) {
		return changeDirectories(command, i);
	} 
  
	if (strcmp(command[0], "bg") == 0) {
    	return processBackgroundCommand(command+1);
  	}
  
  	if (strcmp(command[0], "bglist") == 0) {
    	return processBGListCommand();
  	}

  	return processGeneralCommand(command);
}

int main(int argc, char **argv) {
	while(1){
	  	int err = printPrompt();
	    if (err != 0) {
	      return err;
	    }
		    
			err = processCommand();
	    if (err != 0) {
	      return err;
   		}	
	}
	return 0;
}