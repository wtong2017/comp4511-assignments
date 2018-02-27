#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define MAX_CMDLINE_LEN 256
#define MAX_PROGRAM_PATH 128

/* function prototypes go here... */
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);
int tokenize_cmd_line(char *tokens[MAX_CMDLINE_LEN], char *cmdline);
int get_program_path(char *paths[MAX_PROGRAM_PATH]);
void run_program(const char *filename, char *const argv[], char *const envp[]);
void my_cd(char *path);
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
	char *tokens[MAX_CMDLINE_LEN];
	int token_num = tokenize_cmd_line(tokens, cmdline);
	// Exit
	if (token_num == 1 && strncmp(tokens[0], "exit", 4) == 0) {
		printf("myshell is terminated with pid %i\n", getpid());
		exit(0);
	}
	// Child cmd from lab
	if (token_num == 2 && strncmp(tokens[0], "child", 5) == 0) {
		if (str_to_int(tokens[1]) == 1) {
			child_command(atoi(tokens[1]));
			return;
		}
	}
	//if (token_num == 1 && strncmp(tokens[0], "cd", 2) == 0) {
	//	my_cd(NULL);
	//	return;
	//}
	// My cd cmd using chdir
	if ((token_num == 1 || token_num == 2) && strncmp(tokens[0], "cd", 2) == 0) {
		my_cd(tokens[1]);
		return;
	}
	// Other cmd
	run_program(tokens[0], tokens, NULL);
	//printf("Invalid command\n");
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

int tokenize_cmd_line(char *tokens[MAX_CMDLINE_LEN], char *cmdline) {
	int i = 0;
	char *pch = strtok(cmdline, " \t");
	tokens[i] = pch;
	while (pch != NULL) {
		//printf("%s\n", pch);
		pch = strtok(NULL, " \t");
		i++;
		tokens[i] = pch;
	}
 	return i;
}

void run_program(const char *filename, char *const argv[], char *const envp[]) {
	pid_t pid = fork();
	if (pid == 0) { // Child
		//printf(filename);
		//if (strncmp(filename, "cd", 2) == 0) {
		//	my_cd(argv[1]);
		//}
		char *paths[MAX_PROGRAM_PATH];
		int path_num = get_program_path(paths);
		int i;
		char path_buffer[512];
		strcpy(path_buffer, "./"); // current directory
		strcat(path_buffer, filename);
		if (-1 == (execve(path_buffer, argv, NULL))) {
			for (i = 0; i < path_num; i++) {
				strcpy(path_buffer, paths[i]);
				strcat(path_buffer, "/");
				strcat(path_buffer, filename);
				//printf("%s\n", path_buffer);
				if (-1 == (execve(path_buffer, argv, NULL)) && i == path_num-1) {
					printf("%s: Command not found.\n", filename);
					//perror("execve");  
        				exit(1);
				}
			}
    		}
		exit(0);
	}
	// Parent
	wait(0);
}

void my_cd(char *path) {
	if (!path) {
		path = getenv("HOME");	
	}
	//printf("Path: %s\n", path);	
	if (chdir(path) == -1) {
		perror("chdir");
		//exit(1);
	}
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

int get_program_path(char *paths[MAX_PROGRAM_PATH]) {
	char* path_var = getenv("PATH");
	int i = 0;
	char *pch = strtok(path_var, ":");
	paths[i] = pch;
	while (pch != NULL) {
		//printf("%s\n", pch);
		pch = strtok(NULL, ":");
		i++;
		paths[i] = pch;
	}
 	return i;
}
