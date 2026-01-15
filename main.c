#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define READ_END 0
#define WRITE_END 1

struct tokenizedCommand{
	char *command;
	char *argv[4]; //NOTE hardcoded for now
	int argc;
}tokenizedCommand;


struct tokenizedCommand * tokenizeInput(char *userCommand){
	

	int count = 1;
	for(char *p = userCommand; *p; p++){
		if(*p == '|') count++;
	}
	
	//able to just use pipelineArray[0], [1]  etc. pointer arithmetic jumps by sizeof(tokCom) bytes
	//also this is dynamically allocated based on amount of pipes found in user input ^^
	struct tokenizedCommand *pipelineArray = malloc(sizeof(struct tokenizedCommand) * count);


	count = 0;
	int argc = 1;
	char *savePipePtr, *saveArgsPtr;
	char *pipeToken = strtok_r(userCommand, "|", &savePipePtr); //first pipe
	while(pipeToken != NULL){
		printf("Full subcommand %s\n", pipeToken);
	

		//now get fullfirst command
		char *subCommand = strtok_r(pipeToken, " ", &saveArgsPtr);
		char *command = subCommand;

		pipelineArray[count].command = command;
		pipelineArray[count].argv[0] = command;

		char *flags = NULL;
		char *destination = NULL;

		//getting flags + destination etc. loop
		subCommand = strtok_r(NULL, " ", &saveArgsPtr);
		if(subCommand) {
			flags = subCommand;
			pipelineArray[count].argv[1]= flags;
			argc++;
		}
		//pointer to the first instance of " "
		subCommand = strtok_r(NULL, " ", &saveArgsPtr);

		if(subCommand) {
			destination = subCommand;
			pipelineArray[count].argv[2] = destination;
			argc++;
		}
		printf("Command: %s Flags: %s Dest: %s\n",
				command, flags ? flags : "(No)", destination ? destination : "(No)");

		pipelineArray[count].argv[3] = NULL;
		pipelineArray[count].argc = argc;

		pipeToken = strtok_r(NULL, "|", &savePipePtr);
		count++;
	}

	return pipelineArray;
}


void execute(struct tokenizedCommand *cmd){
	if(strcmp((*cmd).command,"exit") == 0){
		exit(0);
	}

	char binaryCommand[1028] = "";
	snprintf(binaryCommand, sizeof(binaryCommand), "/bin/%s", (*cmd).command);
	
	execvp(binaryCommand, (*cmd).argv);
	perror("Exec failed");
	exit(1);
}


void pipelineProcess(struct tokenizedCommand *pipelineArray, int len){
	int prev_fd = -1;

	for (int i = 0; i < len; i++) {
	    int fd[2];
	    if (i < len - 1) pipe(fd);

	    if (fork() == 0) {
		if (prev_fd != -1) {
		    dup2(prev_fd, STDIN_FILENO);
		    close(prev_fd);
		}

		if (i < len - 1) {
		    dup2(fd[WRITE_END], STDOUT_FILENO);
		    close(fd[READ_END]);
		    close(fd[WRITE_END]);
		}
		
		execute(pipelineArray);
		exit(1);
	    }

	    if (prev_fd != -1) close(prev_fd);
	    if (i < len - 1) {
		close(fd[WRITE_END]);
		prev_fd = fd[READ_END];
	    }
	}

}

int main(int argc, char *argv[]){
	char command[1024] = "";
	while(strcmp(command, "exit") != 0){

		printf("Enter Command -> ");
		if(!fgets(command, sizeof(command), stdin)){
			perror("Error parsing command, fgets, line 28");
			exit(1);
		}

		command[strcspn(command, "\r\n")] = '\0'; //reads until carriage return
		if(strlen(command) == 0) continue; //prints new line on enter ?
		
		struct tokenizedCommand *pipeline = tokenizeInput(command);

		if(strcmp(command, "cd") == 0){
			//chdir(flags);
			printf("CD\n");
		} else{
			pipelineProcess(pipeline, pipeline->argc);
		}
	}
	return 0;
}
