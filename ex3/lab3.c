#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>


int main(int argc, char* argv[]) {
	//argument checks, mesages etc.
	time_t t;
	srand((unsigned) time(&t));
	bool rr = false;
	bool random = false;
	int n; //arithmos paidiwn
	if (argc == 2) {
		rr = true;
		n = atoi(argv[1]);
		if (n <= 0) {
			fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
			exit(1);
		}
	}
	else if (argc == 3) {
		n = atoi(argv[1]);
		if (strcmp(argv[2], "--random") == 0) random=true;
		else if (strcmp(argv[2], "--round-robin") == 0) rr = true;
		else {
			fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
			exit(1);
		}
	}
	else {
		fprintf(stderr, "Usage: ask3 <nChildren> [--random] [--round-robin]\n");
		exit(1);
	}

	int **pdp2c = (int **)malloc(sizeof(int *)*n);
	for (int i=0; i<n; i++) {
		pdp2c[i] = (int *)malloc(sizeof(int)*2);
		if (pipe(pdp2c[i]) == -1) {
			perror("pipe");
			exit(1);
		}
	}//dimiourgia pipe parent to child
	int **pdc2p = (int **)malloc(sizeof(int *)*n);
	for (int i =0; i<n; i++) {
		pdc2p[i] = (int *)malloc(sizeof(int)*2);
		if (pipe(pdc2p[i]) == -1) {
			perror("pipe");
			exit(1);
		}
	}//dimiourgia pipe child to parent
	
	//meta mia fork gia dimiourgia paidiwn
	pid_t *pid = malloc(n*sizeof(pid_t));
	for (int i=0; i<n; i++) {
		pid[i] = fork();
		if (pid[i] < 0) {
			perror("fork");
			exit(1);
		}
		else if (pid[i] == 0) {
			//child
			int val;
			close(pdp2c[i][1]);
			close(pdc2p[i][0]);
			while (1) {
				if (read(pdp2c[i][0], &val, sizeof(int)) == -1) {
					perror("read");
					exit(1);
				}
				printf("[Child %d] [%d] Child received %d!\n", i+1, getpid(), val);
				val--;
				sleep(10);
				if (write(pdc2p[i][1], &val, sizeof(int)) == -1) {
					perror("write");
					exit(1);
				}
				printf("[Child %d] [%d] Child finished hard work, writing back %d\n", i+1, getpid(), val);
			}
		}
		else {
			close(pdp2c[i][0]);
			close(pdc2p[i][1]);
		}
	}
	int ready;
	char buff[50];
	ssize_t s;
	int child_to_work = 0;

	struct pollfd *pollfds = malloc((n+1)*sizeof(struct pollfd));
	pollfds[0].fd = 1;
	pollfds[0].events = POLLIN;
	for (int i=1; i<=n; i++) {
		pollfds[i].fd = pdc2p[i-1][0];
		pollfds[i].events = POLLIN;
	}
	while(1) {
		ready = poll(pollfds, n+1, -1);
		if (ready == -1) {
			perror("poll");
			exit(1);
		}
		if (pollfds[0].revents & POLLIN) {
			s = read(pollfds[0].fd, buff, sizeof(buff));
			if (s == -1) {
				perror("read");
				exit(1);
			}
			if(buff[s-1] == '\n') {
				buff[s-1] = '\0';
				s--;
			}
			if (strcmp(buff, "exit") == 0) {
				int count=n;
				int wstatus;
				for (int i=0; i<n; i++) {
					printf("Waiting for %d children to exit\n", count);
					if(kill(pid[i], SIGTERM) == -1) {
						perror("kill");
						exit(1);
					}
					count--;
					if (waitpid(pid[i], &wstatus, 0) == -1) {
						perror("wait");
						exit(1);
					}
				}
				printf("All children terminated\n");
				for (int i=0; i<n; i++) {
					close(pdp2c[i][1]);
					close(pdc2p[i][0]);
				}
				free(pdp2c);
				free(pdc2p);
				exit(0);
			}
			bool is_int = true;
			for (int i=0; i<s; i++) {
				if(isdigit(buff[i]) == 0) {
					is_int = false;
					break;
				}
			}
			if(is_int) {
				int job = atoi(buff);
				if (random) {
					child_to_work = rand() % n;
					if (write(pdp2c[child_to_work][1], &job, sizeof(int)) == -1) {
						perror("write");
						exit(1);
					}
				}
				else if(rr) {
					if (write(pdp2c[child_to_work][1], &job, sizeof(int)) == -1) {
						perror("write");
						exit(1);
					}
					if (child_to_work == n-1) child_to_work = 0;
					else child_to_work++;
				}
			}
			else printf("Type a number to send job to a child!\n");
		}
		for (int i=1; i<=n; i++) {
			if (pollfds[i].revents & POLLIN) {
				int val;
				if (read(pdc2p[i-1][0], &val, sizeof(int)) == -1) {
					perror("read");
					exit(1);
				}
				printf("[PARENT] Parent received %d from child %d\n", val, i);
			}
		}
	}
	free(pdp2c);
	free(pdc2p);
	return 0;
}





















