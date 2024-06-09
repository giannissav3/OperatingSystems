#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

int n;
pid_t pid[100];
bool term = false;
char gates[100];

static void handler(int signum){
	switch (signum) {
		case SIGUSR1:
			for (int i=0; i<n; i++) {
				if(kill(pid[i], SIGUSR1) == -1) {
					perror("kill");
					exit(1);
				}
			}
			break;
		case SIGUSR2:
			for (int i=0; i<n; i++) {
				if(kill(pid[i], SIGUSR2) == -1) {
					perror("kill");
					exit(1);
				}
			}
			break;
		case SIGTERM:
			term = true;
			int count=n;
			int wstatus;
			for (int i=0; i<n; i++) {
				printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), count);
				if(kill(pid[i], SIGTERM) == -1) {
					perror("kill");
					exit(1);
				}
				count--;
				if(waitpid(pid[i], &wstatus, 0) == -1) {
					perror("waitpid");
					exit(1);
				}
				else printf("[PARENT/PID=%d] Child with PID=%d terminated succesfully with exit status code %d!\n", getpid(), pid[i], WEXITSTATUS(wstatus));
			}
			printf("All children exited, terminating as well\n");
			exit(0);
			break;
		case SIGCHLD:
			if (term==false) {
				int term_id;
				int wstatus;
				pid_t term_pid = waitpid(-1, &wstatus, WUNTRACED);
				if (term_pid < 0) {
					perror("waitpid");
					exit(1);
				}
				else if (term_pid > 0) {
					if (WIFSTOPPED(wstatus)) {
						printf("Child with PID=%d stopped, but it will continue\n", term_pid);
						if(kill(term_pid, SIGCONT) == -1) {
							perror("kill");
							exit(1);
						}
					}
					else if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) {
						for(int i=0; i<n; i++) {
							if(pid[i] == term_pid) term_id = i;
						}	
						printf("[PARENT/PID=%d] Child %d with PID=%d exited\n", getpid(), term_id, term_pid);
						pid_t new_pid = fork();
						if (new_pid<0) {
							perror("fork");
							exit(1);
						}
						else if (new_pid==0) {
							char id[10];
							snprintf(id, sizeof(id), "%d", term_id);
							char *arg_list[] = {"./child", id, &gates[term_id], NULL};
							execv("./child", arg_list);
						}
						else{
							printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state '%c'\n", getpid(), term_id, new_pid, gates[term_id]);
							pid[term_id] = new_pid;
						}
					}
				}
			}
			break;
	}
}

int main(int argc, char *argv[]) {
	//arguments check//
	if (argc != 2) {
		fprintf(stderr, "Usage: %s ffttf (#f=number of closed gates, #t=number of open gates)\n", argv[0]);
		return 1;
	}
	n = strlen(argv[1]);
	for (int i=0; i<n; i++) {
		gates[i] = argv[1][i];
		if ((argv[1][i] != 'f') && (argv[1][i] != 't')) {
			fprintf(stderr, "Second argument should contain only 'f' for closed gates and 't' for open gates\n");
			return 1;
		}
	}
	struct sigaction action;
	action.sa_handler = handler;
	action.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &action, NULL);
	sigaction(SIGUSR2, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGCHLD, &action, NULL);
	for (int i=0; i<n; i++) {
		pid[i] = fork();
		if (pid[i]<0) {
			perror("fork");
			return 1;
		}
		else if (pid[i]==0) {
			char id[10];
			snprintf(id, sizeof(id), "%d", i);
			char* arg_list[] = {"./child", id, &gates[i], NULL};
			execv("./child", arg_list);
		}
		else {
			printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, pid[i], gates[i]);
		}
	}
	while (1) {
		sleep(1);}
	return 0;

}

