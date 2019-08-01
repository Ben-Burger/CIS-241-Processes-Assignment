#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
	
	int numCommands = 0;
	char** commands;

	int flag = 1;

	while (flag) {
		char input[30];

		printf("Please enter a command to run or type \"execute\" to execute command(s).\n");
		scanf("%s", input);

		if (strcmp(input, "execute") == 0) {
			flag = 0;
		} else {
			numCommands++;

			if (numCommands == 1) {
				commands = (char**) malloc(numCommands * sizeof(char*));
			} else if (numCommands > 1) {
				commands = (char**) realloc(commands, numCommands * sizeof(char*));
			} 
			commands[numCommands - 1] = (char*) malloc(strlen(input) * sizeof(char));
			strcpy(commands[numCommands - 1], input);
		}	
	}

	pid_t process_ids[numCommands];
	int exit_codes[numCommands];
	double times[numCommands];

	double total_time = 0.00;

	for (int i = 0; i < numCommands; i++) {
		int status;
		int exit_code;
		clock_t begin;
		clock_t end;
		double time;

		pid_t c_pid = fork();
		

		if (c_pid == 0) {
			
			begin = clock();
			exit_code = execvp(commands[i], "");
			exit(exit_code);
		} else if (c_pid > 0) {
			process_ids[i] = c_pid;

			wait(&status);
		
			if (WIFEXITED(status)) {
				end = clock();
				exit_codes[i] = WEXITSTATUS(status);
				time = (double) (end - begin) / CLOCKS_PER_SEC;
				times[i] = time;

				total_time += time;
			}
		}

	}

	for (int i = 0; i < numCommands; i++) {
		printf("%s \t %d \t %d \t %f\n", commands[i], process_ids[i], exit_codes[i], times[i]);
	}

	exit(0);	
}
