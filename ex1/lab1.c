#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {

	struct stat files;
	if (argc !=2){
		printf("Usage: ./a.out filename\n");
		return 1;
	}
	else if (stat(argv[1], &files) == 0){
		printf("Error: %s already exists\n", argv[1]);
		return 1;
	}
	else if (strcmp(argv[1], "--help")==0){
		printf("Usage: ./a.out filename\n");
		exit(0);
	}

	int fd = open(argv[1], O_CREAT | O_APPEND  | O_WRONLY, 0644);
	if (fd == -1) {
		perror("open");
		return 1;
	}

	int status;
	pid_t child ;
	child = fork();

	if(child<0){
		printf("There are no children\n");
		exit (1);
	}

	if(child==0){
		pid_t child_pid = getpid();
		pid_t parent_pid = getppid();
		char child_buf[100] ;
		snprintf(child_buf, sizeof(child_buf), "[CHILD] getpid()=%d, getppid()=%d\n", child_pid, parent_pid);
		if (write(fd, child_buf, strlen(child_buf)) < strlen(child_buf)) {
			perror("write") ;
			return 1;
		}
		exit(0);
	}
	else {
		pid_t pchild_pid = getpid();
		pid_t pparent_pid = getppid();
		char parent_buf[100];
		wait(&status);
		snprintf(parent_buf, sizeof(parent_buf), "[PARENT] getpid()=%d, getppid()=%d\n", pchild_pid, pparent_pid);
		if (write(fd, parent_buf, strlen(parent_buf)) < strlen(parent_buf)){
			perror("write");
			return 1;
		}
		close(fd);
		exit(0);
	}
	return 0;
}

