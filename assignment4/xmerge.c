#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/errno.h>

#define MAXLEN 100

asmlinkage long sys_xmerge(void* args, size_t argslen) {
	unsigned int i;
	// Allocate one page as a temporary buffer
	char buf[MAXLEN];
	// Arguments packed inside void*
	void **ptr; // void* to void**
	__user const char *outfile; // name of the output file
	__user const char **infiles; // names of input files (array)
	unsigned int num_files; // number of input files
	int oflags; // the open flag for the outfile
	mode_t mode; // permission mode for newly created outfile
	__user int *ofile_count; // the number of files that have been read into outfile
	// Return value from other syscall
	int ret;
	// Store the name of files
	char *outfileName = kmalloc(MAXLEN, GFP_KERNEL);
	char *infileName;
	// Handle file system
	mm_segment_t old_fs;
	int in_fd, out_fd, bytes;
	int count = 0;
	
	if (args == NULL) {
		// null arguments
		printk(KERN_ERR "Null arguments\n");
		return -1;
	}

	ptr = args;
	if (argslen < 6) {
		// missing arguments passed
		printk(KERN_ERR "Missing arguments passed\n");
		return -1;
	}

	// Read input argument
	// Handle output file
	ofile_count = (int *)ptr[5];
	outfile = (const char *)ptr[0];
	ret = strncpy_from_user((char *)outfileName, outfile, MAXLEN) > 0;
	if (ret < 0) {
		return ret;
	}
	else {
		printk(KERN_INFO "outfile: %s\n", (char *)outfileName);
	}
	ret = get_user(oflags, (int *)ptr[3]);
	if (ret < 0) {
		return ret;
	}
	ret = copy_from_user(&mode, (mode_t *)ptr[4], sizeof(mode_t));
	if (ret < 0) {
		return ret;
	}

	// Handle input files
	infiles = (const char **)ptr[1];
	ret = get_user(num_files, (unsigned int *)ptr[2]);
	if (ret != 0) {
		return ret;
	}

	// Start file I/O
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	out_fd = sys_open(outfileName, oflags, mode);
	if (out_fd < 0) {
		return out_fd;
	}

	for (i = 0; i < num_files; i++) {
		infileName = kmalloc(MAXLEN, GFP_KERNEL); 
		ret = strncpy_from_user((char *)infileName, infiles[i], PAGE_SIZE) > 0;
		if (ret < 0) {
			return ret;
		}
		printk(KERN_INFO "infile %i: %s\n", i, (char *)infileName);
		
		in_fd = sys_open(infileName, O_RDONLY, 0);
		if (in_fd < 0) {
			return in_fd;
		}

		while (1) {
			bytes = sys_read(in_fd, buf, MAXLEN);
			if (bytes < 0) {
				return bytes;
			}
			else if (bytes == 0) {
				break;
			}
			count += bytes;
			printk(KERN_INFO "%s", buf);
			sys_write(out_fd, buf, strlen(buf));		
		}
		sys_close(in_fd);
		kfree(infileName);
	}
	sys_close(out_fd);
	set_fs(old_fs);

	printk("Count: %i\n", count);
	put_user(count, ofile_count); // Write the count into user space
	kfree(outfileName);
	return 0;
}
