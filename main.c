#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

// CIS241 Final Project
// Ben Burger and Reuben Nyenhuis
// 8-2-2019
// This code is designed to imitate a trivial shell. It continuously asks
// the user for command inputs until the command "execute" is submitted. Then,
// the program runs each process and stores key data such as process time, process id,
// exit code, and total runtime.


int find_int_length(int n);

int main(int argc, char** argv) {
	
	int numCommands = 0;
	char** commands;

	int flag = 1;

	// Continuously loops and stores data until the user enters "execute"
	while (flag) {
		char *input;
		size_t len = 0;

		printf("Please enter a command to run or type \"execute\" to execute command(s).\n");
		//scanf("%[^\n]", input);

		getline(&input, &len, stdin);
		
		// getline stores the carriage return, this line removes it
		input[strlen(input) - 1] = '\0';	

		// if the user enters the "execute" command, the flag is turned off so the program continues
		if (strcmp(input, "execute") == 0) {
			flag = 0;
		} else {
			numCommands++;
			
			// dynamically creates and adds to the command array as the user continuously enters data
			if (numCommands == 1) {
				commands = (char**) malloc(numCommands * sizeof(char*));
			} else if (numCommands > 1) {
				//the 'realloc' command is used to copy over existing data while increasing the size
				commands = (char**) realloc(commands, numCommands * sizeof(char*));
			} 
			commands[numCommands - 1] = (char*) malloc(strlen(input) * sizeof(char));

			// puts the user input into the newly created array slot
			strcpy(commands[numCommands - 1], input);
		}	
	}

	pid_t process_ids[numCommands];
	int exit_codes[numCommands];
	double times[numCommands];

	double total_time = 0.00;

	// Works through all of the inputted commands to try and run them
	for (int i = 0; i < numCommands; i++) {
		int status;
		int exit_code;
		clock_t start, stop;
		double time_passed;
		int arguments = 1;
		int isString = 0;

		// Counts the number of arguments in the given command
		for (int count = 0; count < strlen(commands[i]); count++){
			char ch = commands[i][count];
			if (ch == '"') {
				if (isString == 0) {
					isString = 1;
				} else {
					isString = 0;
				}
			}

			if (isblank(ch) && (isString == 0)) {
				arguments++;
			}
		}

		// An argument array is created to store all of the individual arguments
		char argarray[arguments][30];

		int j = 0; 
		int cnt = 0;

		// Goes through the command and parses out the arguments to store separately
    		for (int y = 0; y <= strlen(commands[i]); y++){

			if (commands[i][y] == '"') {
				if (isString == 0) { 
					isString = 1;
				} else { 
					isString = 0;
				}
			}

        		if((commands[i][y] == ' ') && (isString == 0)) {
            			argarray[cnt][j] = '\0';
            			cnt++;
            			j = 0;
        		} else {
            			argarray[cnt][j] = commands[i][y];
            			j++;
        		}
    		}

		// Forks into a separate process to run the command
		pid_t c_pid = fork();

		// saves the time of the clock for future reference
		start = clock();

		// if c_pid is 0, the current process is the child
		if (c_pid == 0) {
			
			// saves the arguments onto the command 'args' array
			char* args[arguments + 1];
			for(int k = 0; k < arguments; k++){
				args[k] = argarray[k];
			}
			
			printf("\n%s:\n", commands[i]);

			// sets the last value of the 'args' array to null so the 'execvp' command knows that all the arguments are entered
			args[arguments] = 0;
		
			// the command and its arguments are send to 'execvp' to be run, and the exit code is stored
			exit_code = execvp(args[0], args);

			// if 'execvp' runs into an error, it is displayed
			perror("Error: ");	

			// the child process dismisses with the exit code given by 'execvp'
			exit(exit_code);

		// if the c_pid isn't 0, the current process is the parent process.
		} else if (c_pid > 0) {
			// the process id of the child is stored
			process_ids[i] = c_pid;

			// the parent waits for the 'status' to be updated by the child
			wait(&status);
		
			// If status has been changed, the time is saved, the 
			// exit code is saved, and the process time is calculated and stored
			if (WIFEXITED(status)) {
				stop = clock();
				exit_codes[i] = WEXITSTATUS(status);
				time_passed = (double) (stop - start) / CLOCKS_PER_SEC;
				times[i] = time_passed;
				total_time += time_passed;
			}
		}
	}

	
	// find max lengths
	int command_length = 7;
	int process_length = 3;
	int exit_code_length = 9;
	for (int i = 0; i < numCommands; i++) {
		if (strlen(commands[i]) > command_length) {
			command_length = strlen(commands[i]);
		}

		int process_nDigits = find_int_length(process_ids[i]);
		if (process_nDigits > process_length) {
			process_length = process_nDigits;
		}

		int exit_code_nDigits = find_int_length(exit_codes[i]);
		if (exit_code_nDigits > exit_code_length) {
			exit_code_length = exit_code_nDigits;
		}
	}

	// Print the results after every command has been run.
	int n1;
	int spacing = 4;
	printf("\n");

	printf("command");
	n1 = command_length - 7;
	for (int i = 0; i < n1 + spacing; i++) {
		printf(" ");
	}

	printf("pid");
	n1 = process_length - 3;
	for (int i = 0; i < n1 + spacing; i++) {
		printf(" ");
	}

	printf("exit code");
	n1 = exit_code_length - 9;
	for (int i = 0; i < n1 + spacing; i++) {
		printf(" ");
	}

	printf("time");
	printf("\n");

	for (int i = 0; i < numCommands; i++) {
		int n2;
		
		printf("%s", commands[i]);
		n2 = command_length - strlen(commands[i]);
		for (int i = 0; i < n2 + spacing; i++) {
			printf(" ");
		}

		printf("%d", process_ids[i]);
		n2 = process_length - find_int_length(process_ids[i]);
		for (int i = 0; i < n2 + spacing; i++) {
			printf(" ");
		}

		printf("%d", exit_codes[i]);
		n2 = exit_code_length - find_int_length(exit_codes[i]);
		for (int i = 0; i < n2 + spacing; i++) {
			printf(" ");
		}

		printf("%f sec\n", times[i]);
	}

	printf("\nTotal Time: %f sec\n\n", total_time);
	exit(0);	
}

int find_int_length(int n) {
	if (n == 0) {
		return 1;
	} else {
		return (floor(log10(abs(n))) + 1);
	}
}
