#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
	
#define SYS_XMERGE 355
#define MAXLEN 100

int main(int argc, char *argv[]) {
	void *v[6];
	void *pt;
	unsigned int num_files;
	char* outfile;
	mode_t mode;
	int i, ofile_count, ret, oflags;
	char **infiles;

	// Minimun: ./test_xmerge -h (print short usage string)
	if (argc < 2) {
		printf("Not enough arguments\n");
		return 0;
	}
	num_files = argc - 3;
	mode = 0664; // Default mode
	
	// Check flags
	// Assume only one flag can be used
	oflags = 0;
	if (strncmp(argv[1], "-a", 2) == 0) {
		oflags |= O_APPEND;
	}
	else if (strncmp(argv[1], "-c", 2) == 0) {
		oflags |= O_CREAT;
	}
	else if (strncmp(argv[1], "-t", 2) == 0) {
		oflags |= O_TRUNC;
	}
	else if (strncmp(argv[1], "-e", 2) == 0) {
		oflags |= O_EXCL;
	}
	else if (strncmp(argv[1], "-m", 2) == 0) {
		if (argc < 3) {
			printf("Not enough arguments\n");
			return 0;
		}
		mode = strtol(argv[2], NULL, 8);
		num_files -= 1;
	}
	else if (strncmp(argv[1], "-h", 2) == 0) {
		printf("Usage: ./test_xmerge [flags] outfile infile infile2 ...\n");
		return 0;
	}
	else {
		printf("Invalid flags\n");
		return 0;
	}

	if (num_files < 1) {
		printf("Not enough arguments\n");
		return 0;
	}

	infiles = (char **)malloc(num_files * sizeof(char *));
	for (i = 0; i < num_files; i++) {
		infiles[i] = (char *)malloc(MAXLEN * sizeof(char));
		strncpy(infiles[i], argv[3+i], MAXLEN);
	}
	outfile = (char *)malloc(MAXLEN * sizeof(char));
	strncpy(outfile, argv[2], MAXLEN);
	/* printf("Outfile: %s\n", outfile);
	for (i = 0; i < num_files; i++)
		printf("%i, %s\n", i, infiles[i]); */			

	// Pack the arguments
	v[0] = outfile;
	v[1] = infiles;
	v[2] = &num_files;
	v[3] = &oflags;
	v[4] = &mode;
	v[5] = &ofile_count;
	pt = v;
	ret = syscall(SYS_XMERGE, pt, sizeof(v)/sizeof(v[0]));
	if (ret < 0) {
		perror("SYS_XMERGE");
		return 0;
	}
	
	printf("In total, %i files have been successfully merged!\nThe total read size: %i bytes.\n", num_files, ofile_count);

	// Free memory
	for (i = 0; i < num_files; i++)
		free(infiles[i]);
	free(infiles);
	free(outfile);
	return 0;
}
