#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

bool gate_status;
int id;
time_t start_time, current_time, t;

static void child_handler(int signum) {
	current_time = time(NULL);
	t = current_time - start_time;
	switch (signum) {
		case SIGUSR1:
			if (gate_status) printf("[ID=%d/PID=%d/TIME=%lds] The gates are open!\n", id, getpid(), t);
			else printf("[ID=%d/PID=%d/TIME=%lds] The gates are closed!\n", id, getpid(), t);
			break;
		case SIGUSR2:
			if (gate_status) {
				gate_status = false;
				printf("[ID=%d/PID=%d/TIME=%lds] The gates are closed!\n", id, getpid(), t);
			}
			else {
				gate_status = true;
				printf("[ID=%d/PID=%d/TIME=%lds] The gates are open!\n", id, getpid(), t);
			}
			break;
		case SIGALRM:
			alarm(15);
			if (gate_status) printf("[ID=%d/PID=%d/TIME=%lds] The gates are open!\n", id, getpid(), t);
			else printf("[ID=%d/PID=%d/TIME=%lds] The gates are closed!\n", id, getpid(), t);
			break;
		case SIGTERM:
			exit(0);
			break;
	}
}

int main(int argc, char *argv[]) {
	start_time = time(NULL);
	current_time = time(NULL);
	struct sigaction sa;
	sa.sa_handler = child_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	alarm(15);
	if(argv[2][0] == 't') gate_status = true;
	else if(argv[2][0] == 'f') gate_status = false;
	id = atoi(argv[1]);
	t = current_time - start_time;
	if (gate_status) printf("[ID=%d/PID=%d/TIME=%lds] The gates are open!\n", id, getpid(), t);
	else printf("[ID=%d/PID=%d/TIME=%lds] The gates are closed!\n", id, getpid(), t);
	while(1) {
		sleep(1);}
	return 0;
}	

