#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define MAX_CMDLINE_LEN 256

/* function prototypes go here... */
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);
int tokenize_cmd_line(char tokens[MAX_CMDLINE_LEN][MAX_CMDLINE_LEN], char *cmdline);
void child_command();

/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	while (1) 
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		
		process_cmd(cmdline);
	}
	return 0;
}

void process_cmd(char *cmdline)
{
	// printf("%s\n", cmdline);
	char tokens[MAX_CMDLINE_LEN][MAX_CMDLINE_LEN];
	int token_num = tokenize_cmd_line(tokens, cmdline);
	if (token_num == 1 && strncmp(tokens[0], "exit", 4) == 0) {
		printf("myshell is terminated with pid %i\n", getpid());
		exit(0);
	}
	if (token_num == 2 && strncmp(tokens[0], "child", 5) == 0) {
		if (str_to_int(tokens[1]) == 1) {
			child_command(atoi(tokens[1]));
			return;
		}
	}
	printf("Invalid command\n");
}

void show_prompt() 
{
	char cwd[1024];
        char dir[256];
	int i;
	getcwd(cwd, sizeof(cwd));
	int len = strlen(cwd);
	for (i = len-1; i >= 0; --i) {
		if (cwd[i] == '/') {
			break;
		}
	}
	strcpy(dir, cwd + i + 1);
	printf("[%s] myshell> ", dir);
}

int get_cmd_line(char *cmdline) 
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LEN, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ')
    {
        ++i;
    }
    if (i == n)
    {
        // Empty command
        return -1;
    }
    return 0;
}

int tokenize_cmd_line(char tokens[MAX_CMDLINE_LEN][MAX_CMDLINE_LEN], char *cmdline) {
	int count = 0;
	int i, l;
	char token[MAX_CMDLINE_LEN];
	for (i = 0; 1 == sscanf(cmdline + i, "%s%n", token, &l); i = i + l) {
		strcpy(tokens[count], token);
		count += 1;
	}
 	return count;
}

void child_command(int time) {
	int child_status;
	pid_t pid = fork();
	if (pid == 0) { // Child
		printf("child pid %d is started\n", getpid());
		sleep(time);
		exit(0);
	}
	// Parent
	pid_t child_pid = wait(&child_status);
	printf("child pid %d is terminated with status %i\n", child_pid, child_status);
}

int str_to_int(char *str) {
	while (*str) {
		if (isdigit(*str) == 0) {
			return 0;
		}
   		str++;
	}
	return 1;
}
