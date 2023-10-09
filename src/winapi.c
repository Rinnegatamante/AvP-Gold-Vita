#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <vitasdk.h>

#include "fixer.h"

size_t _mbclen(const unsigned char *s)
{
	return strlen((const char *)s);
}

HANDLE CreateFile(const char *file, int mode, int x, int y, int flags, int flags2, int z)
{
	int fd;
	
	printf("CreateFile(%s, %d, %d, %d, %d, %d, %d)\n", file, mode, x, y, flags, flags2, z);

	switch(mode) {
		case GENERIC_READ:
			if (flags != OPEN_EXISTING) {
				printf("CreateFile: GENERIC_READ flags = %d\n", flags);
				exit(EXIT_FAILURE);
			}
 			fd = open(file, O_RDONLY);
 			if (fd == -1) {
 				/* perror("CreateFile"); */
 				return INVALID_HANDLE_VALUE;
 			}
			break;
		case GENERIC_WRITE:
			if (flags != CREATE_ALWAYS) {
				printf("CreateFile: GENERIC_WRITE flags = %d\n", flags);
				exit(EXIT_FAILURE);
			}
			fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
			if (fd == -1) {
				perror("CreateFile");
				return INVALID_HANDLE_VALUE;
			}
			break;
		case GENERIC_READ|GENERIC_WRITE:
		default:
			printf("CreateFile: unknown mode %d\n", mode);
			exit(EXIT_FAILURE);
	}
		
	return (HANDLE)fd;
}

HANDLE CreateFileA(const char *file, int write, int x, int y, int flags, int flags2, int z)
{
	return CreateFile(file, write, x, y, flags, flags2, z);
}

int WriteFile(HANDLE file, const void *data, int len, void *byteswritten, int lpOverlapped)
{
	unsigned long *bw, i;
	
	printf("WriteFile(%d, %p, %d, %p, %d)\n", file, data, len, byteswritten, lpOverlapped);

	bw = (unsigned long *)byteswritten;
	*bw = 0;
	
	i = write(file, data, len);
	if (i == -1) {
		return 0;
	} else {
		*bw = i;
		return 1;
	}
}

int ReadFile(HANDLE file, void *data, int len, void *bytesread, int lpOverlapped)
{
	unsigned long *br, i;
	
	printf("ReadFile(%d, %p, %d, %p, %d)\n", file, data, len, bytesread, lpOverlapped);

	br = (unsigned long *)bytesread;
	*br = 0;
	
	i = read(file, data, len);
	if (i == -1) {
		return 0;
	} else {
		*br = i;
		return 1;
	}
}

int GetFileSize(HANDLE file, int lpFileSizeHigh)
{
	struct stat buf;
	
	printf("GetFileSize(%d, %d)\n", file, lpFileSizeHigh);
	
	if (fstat(file, &buf) == -1)
		return -1;
	return buf.st_size;
}

int CloseHandle(HANDLE file)
{

	printf("CloseHandle(%d)\n", file);
	
	if (close(file) == -1) 
		return 0;
	else
		return 1;
	
	return 0;
}

int DeleteFile(const char *file)
{

	printf("DeleteFile(%s)\n", file);
	
	if (unlink(file) == -1)
		return 0;
	else
		return 1;
}

int DeleteFileA(const char *file)
{
	return DeleteFile(file);
}

int GetDiskFreeSpace(int x, unsigned long *a, unsigned long *b, unsigned long *c, unsigned long *d)
{
	printf("GetDiskFreeSpace(%d, %p, %p, %p, %p)\n", x, a, b, c, d);

	return 0;
}

int CreateDirectory(char *dir, int lpSecurityAttributes)
{

	printf("CreateDirectory(%s, %d)\n", dir, lpSecurityAttributes);

	if (sceIoMkdir(dir, 0777) == -1)
		return 0;
	else
		return 1;
}

int MoveFile(const char *newfile, const char *oldfile)
{
	printf("MoveFile(%s, %s)\n", newfile, oldfile);
	
	return 0;
}

int MoveFileA(const char *newfile, const char *oldfile)
{
	return MoveFile(newfile, oldfile);
}

int CopyFile(const char *newfile, const char *oldfile, int x)
{
	printf("CopyFile(%s, %s, %d)\n", newfile, oldfile, x);
	
	return 0;
}

int GetFileAttributes(const char *file)
{
	printf("GetFileAttributes(%s)\n", file);
	
	return 0;
}

int GetFileAttributesA(const char *file)
{
	return GetFileAttributes(file);
}

unsigned int SetFilePointer(HANDLE file, int x, int y, int z)
{
	printf("SetFilePointer(%d, %d, %d, %d)\n", file, x, y, z);
	
	return 0;
}

int SetEndOfFile(HANDLE file)
{
	printf("SetEndOfFile(%d)\n", file);
	
	return 0;
}

/* time in miliseconds */
unsigned int timeGetTime()
{
	static struct timeval tv0;
	struct timeval tv1;
	int secs, usecs;
	
	if (tv0.tv_sec == 0) {
		gettimeofday(&tv0, NULL);
	
		return 0;
	}
	
	gettimeofday(&tv1, NULL);
	
	secs = tv1.tv_sec - tv0.tv_sec;
	usecs = tv1.tv_usec - tv0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}

	return secs * 1000 + (usecs / 1000);
}

unsigned int GetTickCount()
{
	return timeGetTime();
}
