#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <poll.h>

int main(int argc, char *argv[])
{
    
    pid_t cpid;
    char buffer[256];
    double result;
    double sum = 0.0;
    char *mechanism = argv[4];
    int worker_num = atoi(argv[8]);
    int pipes[worker_num][2];

    printf("worker number is %d\n", worker_num);

    for(int i = 0; i < worker_num; i++){
	if(pipe(pipes[i]) == -1){
	    perror("failed init pipe\n");
	    exit(1);
	};
    }

    pid_t child_pids[worker_num];


    for(int i = 0; i < worker_num; i++){
	printf("start creating pid %d\n", i);
	cpid = fork();
	if(cpid == -1){
	    perror("fork failed\n");
	    exit(1);
	}

	if(cpid == 0){
	    // child process
	    child_pids[i] = getpid();

	    printf("my pid is: %d, my worker no is %d\n", child_pids[i], i);
	    int rs = dup2(pipes[i][0], STDIN_FILENO);
	    int rs2 = dup2(pipes[i][1], STDOUT_FILENO);

	    if(rs < 0 || rs2 < 0)
	    {
		perror("dup failed\n");
		exit(1);
	    }
	    // convert i to string
	    char current_num[48];
	    sprintf(current_num, "%d", i);
	    char *new_argv[] = {
		argv[2],
		argv[5],
		argv[6],
		argv[7],
		current_num,
		NULL
	    };

	    execv(new_argv[0], new_argv);
	    printf("exec is not successful!\n");
	}
    }

    if(strcmp(mechanism, "sequential") == 0){
	for(int i = 0; i < worker_num; i++){
	    waitpid(child_pids[i], NULL, 0);
	    read(pipes[i][0], &result, sizeof(double));
	    sum += result;
	}
    }

    if(strcmp(mechanism, "select") == 0){
	for(int i = 0; i < worker_num; i++){
	    struct timeval tv;
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;

	    fd_set rfds;
	    int retval;
	    FD_ZERO(&rfds);
	    FD_SET(pipes[i][0], &rfds);
	    int MAX_FDN = 12 * 2 + 1 + 1;

	    retval = select(MAX_FDN, &rfds, NULL, NULL, &tv); // 12 fds
	    if(retval == -1)
		perror("select\n");
	    if(FD_ISSET(pipes[i][0], &rfds)){
		printf("select receives\n");
		read(pipes[i][0], &result, sizeof(double));
		sum += result;
		printf("current sum is %f\n", sum);
	    }
	}
    }

    if(strcmp(mechanism, "poll") == 0){
	printf("poll\n");
	struct pollfd pfds[worker_num];
	for(int i = 0; i < worker_num; i++){
	    pfds[i].fd = pipes[i][0];
	    pfds[i].events = POLLIN;
	    pfds[i].revents = 0;
	}

	int counter = 0;
	while(counter != 12){
	    int retval = poll(pfds, 26, 1);
	    for(int i = 0; i < worker_num; i++){
		if(pfds[i].revents & POLLIN){
		    printf("poll receives\n");
		    read(pfds[i].fd, &result, sizeof(double));
		    counter++;
		    sum += result;
		    printf("current sum is %f\n", sum);
		}
	    }
	}
    }

    printf("The mechanism is %s\n", mechanism);
    printf("sum is %f\n", sum);

    return 0;
}
