#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_CMDLINE_LEN 256
#define MAX_PROGRAM_PATH 128

/* function prototypes go here... */
void handler(int signum);
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);
int tokenize_cmd_line(char *tokens[MAX_CMDLINE_LEN], char *cmdline);
int get_program_path(char *paths[MAX_PROGRAM_PATH]);
void run_program(const char *filename, char *argv[], char *const envp[], int last);
void my_cd(char *path);
void my_pipe(char *tokens[MAX_CMDLINE_LEN], int pos);
int find_pipe(char *tokens[MAX_CMDLINE_LEN], int last);
void my_exec(char *tokens[MAX_CMDLINE_LEN]); // Handle I/O redirection
//void child_command();

/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	signal(SIGCHLD, handler);

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
	//if (token_num == 2 && strncmp(tokens[0], "child", 5) == 0) {
	//	if (str_to_int(tokens[1]) == 1) {
	//		child_command(atoi(tokens[1]));
	//		return;
	//	}
	//}
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
	run_program(tokens[0], tokens, NULL, token_num - 1);
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

void run_program(const char *filename, char *argv[], char *const envp[], int last) {
	int background = 0;
	int child_command = 0;
	if (strncmp(argv[last], "&", 1) == 0) {
		argv[last] = NULL;
		background = 1;
	}
	if (strncmp(filename, "child", 5) == 0) {
		if (str_to_int(argv[1])) {
			child_command = 1;
		} 
		else {
			printf("Invalid argument\n");
			return;
		}
	}
	pid_t pid = fork();
	if (pid == 0) { // Child
		//printf(filename);
		//if (background) {
		//	printf("\nbackground process pid %i is started.\n", getpid());
		//}
		int pipe_pos = find_pipe(argv, last);
		if (pipe_pos != -1) {
			my_pipe(argv, pipe_pos);
		}
		else {
			if (child_command) {
				printf("child pid %d is started\n", getpid());
				sleep(atoi(argv[1]));
				exit(0);
			}
			my_exec(argv);
			//char *paths[MAX_PROGRAM_PATH];
			//int path_num = get_program_path(paths);
			//int i;
			//char path_buffer[512];
			//strcpy(path_buffer, "./"); // current directory
			//strcat(path_buffer, filename);
			// Find in current directory first
			//if (-1 == (execve(path_buffer, argv, NULL))) {
			//	for (i = 0; i < path_num; i++) {
			//		strcpy(path_buffer, paths[i]);
			//		strcat(path_buffer, "/");
			//		strcat(path_buffer, filename);
			//		//printf("%s\n", path_buffer);
					// Find in PATH next
			//		if (-1 == (execve(path_buffer, argv, NULL)) && i == path_num-1) {
			//			printf("%s: Command not found.\n", filename);
						//perror("execve");  
        		//			exit(1);
			//		}
			//	}
    			//}
		}
		exit(0);
	}
	// Parent
	if (background == 0) {
		if (child_command) {
			int child_status;
			pid_t child_pid = wait(&child_status);
			printf("child pid %d is terminated with status %i\n", child_pid, child_status);
		}
		else {
			wait(0);
		}
	}
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

//void child_command(int time) {
//	int child_status;
//	pid_t pid = fork();
//	if (pid == 0) { // Child
//		printf("child pid %d is started\n", getpid());
//		sleep(time);
//		exit(0);
//	}
//	// Parent
//	pid_t child_pid = wait(&child_status);
//	printf("child pid %d is terminated with status %i\n", child_pid, child_status);
//}

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

void handler(int signum) {
	int child_status;
	pid_t child_pid;
     
	child_pid = waitpid(-1, child_status, WNOHANG);
	//if (child_pid > 0) {
	//	printf("\nbackground process pid %d is terminated with status %i\n", child_pid, child_status);
		//show_prompt();
	//}
}

int find_pipe(char *tokens[MAX_CMDLINE_LEN], int last) {
	int i;
	for (i = last; i >= 0; i--) {
		if (strncmp(tokens[i], "|", 1) == 0) {
			return i;
		}
	}
	return -1;
}

void my_pipe(char *tokens[MAX_CMDLINE_LEN], int pos) {
	/* Base case */
	if (pos == -1) {
		my_exec(tokens);
	}

	int pfds[2];
	pipe(pfds);
	pid_t pid = fork();
	if (pid == 0) { /* child */
		close(1); /* close stdout */
		dup(pfds[1]);   /* make stdout as pipe input*/
		close(pfds[0]); /* don't need this */
		tokens[pos] = NULL;
		my_pipe(tokens, find_pipe(tokens, pos-1));
	} 
	else { /* The parent process */ 
		close(0); /* close stdin */
		dup(pfds[0]);  /* make stdin as pipe output*/
		close(pfds[1]); /* don't need this */
		wait(0);
		my_exec(tokens+pos+1);
	}
}

void my_exec(char *tokens[MAX_CMDLINE_LEN]) {
	//printf("In my_exec\n");
	int i;
	for (i = 0; tokens[i] != NULL; i++) {
		//printf("%s\n", tokens[i]);
		if (strncmp(tokens[i], "<", 1) == 0) {
			//printf("Input redirection to %s\n", tokens[i+1]);
			int readfd = open(tokens[i+1], O_RDONLY | O_CREAT, 0664);
			close(0);
			dup2(readfd, 0);
			close(readfd);
			tokens[i] = NULL;
		}
		else if (strncmp(tokens[i], ">", 1) == 0) {
			//printf("Output redirection to %s\n", tokens[i+1]);
			int writefd = open(tokens[i+1], O_WRONLY | O_CREAT, 0664);
			close(1);
			close(2); // close stderr
			dup2(writefd, 1);
			dup2(writefd, 2);
			close(writefd);
			tokens[i] = NULL;
		}
	}
	execvp(tokens[0], tokens);
} 
