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
	char *argv[64]; //NOTE hardcoded for now, EDIT changed to 64 can you just leave it like this ?
	int argc;
}tokenizedCommand;


struct tokenizedCommand* tokenizeInput(char *userCommand, int *numCommands){
	
	int count = 1;
	for(char *p = userCommand; *p; p++){
		if(*p == '|') count++;
	}
	
	//able to just use pipelineArray[0], [1]  etc. pointer arithmetic jumps by sizeof(tokCom) bytes
	//also this is dynamically allocated based on amount of pipes found in user input ^^
	struct tokenizedCommand *pipelineArray = malloc(sizeof(struct tokenizedCommand) * count);


	count = 0;
	char *savePipePtr, *saveArgsPtr;
	char *pipeToken = strtok_r(userCommand, "|", &savePipePtr); //first pipe
	while(pipeToken != NULL){
		int argc = 1;
	

		//now get fullfirst command
		char *subCommand = strtok_r(pipeToken, " ", &saveArgsPtr);
		char *command = subCommand;

		pipelineArray[count].command = strdup(command);
		pipelineArray[count].argv[0] = strdup(command);

		char *flags = NULL;
		char *destination = NULL;

		//getting flags + destination etc. loop
		subCommand = strtok_r(NULL, " ", &saveArgsPtr);
		if(subCommand) {
			flags = subCommand;
			pipelineArray[count].argv[1]= strdup(flags);
			argc++;
		}
		//pointer to the first instance of " "
		subCommand = strtok_r(NULL, " ", &saveArgsPtr);

		if(subCommand) {
			destination = subCommand;
			pipelineArray[count].argv[2] = strdup(destination);
			argc++;
		}
		//printf("Command: %s Flags: %s Dest: %s\n",
				//command, flags ? flags : "(No)\n", destination ? destination : "(No)\n");

		pipelineArray[count].argv[3] = NULL;
		pipelineArray[count].argc = argc;

		pipeToken = strtok_r(NULL, "|", &savePipePtr);
		count++;
		(*numCommands) = count; //tells main how many pipes to pass to pipelineProcess
	}

	return pipelineArray;
}


void execute(struct tokenizedCommand *cmd){
	if(strcmp((*cmd).command,"exit") == 0){
		exit(0);
	}

	execvp((*cmd).command, (*cmd).argv);

	perror("Exec failed");
	exit(1);
}


void pipelineProcess(struct tokenizedCommand *pipelineArray, int len){
	int prev_fd = -1;
	pid_t pids[len];
	for (int i = 0; i < len; i++) {
		int fd[2];
		if (i < len - 1) pipe(fd); //pipe if not last command (n-1)
		
		pid_t pid = fork(); 

		if (pid == 0) { //child process
			
			if (prev_fd != -1) { //aka this is not the first command, also needs both read and write open ?
				dup2(prev_fd, STDIN_FILENO); //if there was a previous pipe, duplicate prev_fd to stdin
				close(prev_fd);
			}

			if (i < len - 1) { //if this isnt last pipe
				dup2(fd[WRITE_END], STDOUT_FILENO); //close write end to STDOUT
				close(fd[WRITE_END]);
				close(fd[READ_END]); //close read end
			}
			
			execute(&pipelineArray[i]); //execute current pipe
			exit(1);

		} else{ //parent process
			pids[i] = pid; //adding pid
			if(prev_fd != -1) close(prev_fd); //if there was a previous pipe
			if(i < len - 1){
				prev_fd = fd[READ_END];
				close(fd[WRITE_END]);
			}
		}
	}

	for(int i = 0; i < len; i++){
		waitpid(pids[i], NULL, 0);
	}

}

int main(){
	char command[1024] = "";
	while(strcmp(command, "exit\n") != 0){
			
		fflush(stdout);
		printf("Enter Command -> ");
		fflush(stdout);
		if(!fgets(command, sizeof(command), stdin)){
			perror("Error parsing command, fgets, line 28\n");
			exit(1);
		}
		fflush(stdout);
		command[strcspn(command, "\r\n")] = '\0'; //reads until carriage return
		if(strlen(command) == 0) continue; //prints new line on enter ?
		
		int numCommands = 0;
		struct tokenizedCommand *pipeline = tokenizeInput(command, &numCommands);

		if(strcmp(command, "cd") == 0){
			chdir(pipeline->argv[1]);
		} else{
			pipelineProcess(pipeline, numCommands);
		}
	}
	return 0;
}
