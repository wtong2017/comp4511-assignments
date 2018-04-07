# TODO list:
### xmerge.c
- [x] P1-FS: Use get_fs() and set_fs(KERNEL_DS) for file I/O in kernel 
- [x] P1-Copy: Use copy_from_user for the input paramater
- [x] P1-Open: Use sys_open correctly 
- [x] P1-Loop: Use a loop (with a buffer) read input files and write to the output file
- [x] P1-ByteCount: Return tge correct bytecount (if no error)
- [x] P1-Error: Some error handling is implemented (e.g. file can't be opened/read/write)

### test_xmerge.c 
- [x] Handle -a
- [x] Handle -c
- [x] Handle -t
- [x] Handle -e
- [ ] Handle -m
- [x] Handle -h
