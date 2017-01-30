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
#include <sys/epoll.h>

int main(int argc, char *argv[])
{
    
    pid_t cpid;
    double sum = 0.0;
    char *mechanism = argv[4];
    int worker_num = atoi(argv[8]);
    int pipes[worker_num][2];
    int max_fd = 0;

    for(int i = 0; i < worker_num; i++){
	if(pipe(pipes[i]) == -1){
	    perror("failed init pipe\n");
	    exit(1);
	};
	max_fd = (max_fd > pipes[i][0])? max_fd : pipes[i][0];
    }

    pid_t child_pids[worker_num];

    for(int i = 0; i < worker_num; i++){
	cpid = fork();
	if(cpid == -1){
	    perror("fork failed\n");
	    exit(1);
	}

	if(cpid == 0){
	    // child process
	    child_pids[i] = getpid();

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
	    double results[3];
	    waitpid(child_pids[i], NULL, 0);
	    read(pipes[i][0], results, sizeof(results));
	    printf("worker %d: %d^%d / %d!, %f\n", (int)results[1], (int)results[0], 
		    (int)results[1], (int)results[1], results[2]); //worker 3: 2^3 / 3! : 1.3333
	    sum += results[2];
	}
    }

    if(strcmp(mechanism, "select") == 0){
	fd_set rfds;
	int retval;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	int counter = 0;

	while(counter != worker_num){
	    FD_ZERO(&rfds);
	    for(int i = 0; i < worker_num; i++){
		FD_SET(pipes[i][0], &rfds);
		max_fd = (max_fd > pipes[i][0])? max_fd : pipes[i][0];
	    }
	    retval = select(max_fd + 1, &rfds, NULL, NULL, &tv); 

	    if(retval < 0){
		perror("select\n");
	    } else if(retval > 0){
		for(int i = 0; i < worker_num; i++){
		    if(FD_ISSET(pipes[i][0], &rfds)){
			double results[3];
			read(pipes[i][0], results, sizeof(results));
			printf("worker %d: %d^%d / %d!, %f\n", (int)results[1], (int)results[0], 
				(int)results[1], (int)results[1], results[2]); //worker 3: 2^3 / 3! : 1.3333
			sum += results[2];
			counter++;
		    }
		}
	    }
	}
    }

    if(strcmp(mechanism, "poll") == 0){
	struct pollfd pfds[worker_num];
	for(int i = 0; i < worker_num; i++){
	    pfds[i].fd = pipes[i][0];
	    pfds[i].events = POLLIN;
	    pfds[i].revents = 0;
	}

	int counter = 0;
	while(counter != worker_num){
	    int MAX_FDN = worker_num * 3;
	    int retval = poll(pfds, max_fd, 1);
	    double results[3];
	    for(int i = 0; i < worker_num; i++){
		if(pfds[i].revents & POLLIN){
		    read(pfds[i].fd, &results, sizeof(results));
		    counter++;
		    sum += results[2];
		    printf("worker %d: %d^%d / %d!, %f\n", (int)results[1], (int)results[0], 
			    (int)results[1], (int)results[1], results[2]); //worker 3: 2^3 / 3! : 1.3333
		}
	    }
	}
    }

    if(strcmp(mechanism, "epoll") == 0){
	int retfd = epoll_create(1024); 
	if(retfd < 0)
	    perror("epoll\n");

	struct epoll_event revents[worker_num];

	for(int i = 0; i < worker_num; i++){
	    revents[i].events = EPOLLIN;
	    revents[i].data.fd = pipes[i][0];
	    int res = epoll_ctl(retfd, EPOLL_CTL_ADD, 
		    pipes[i][0], &revents[i]);
	}

	int counter = 0;

	while(counter != worker_num){
	    double results[3];
	    int nfds = epoll_wait(retfd, revents, 1, 1);
	    if(nfds < 0)
		perror("epoll events\n");

	    for(int i = 0; i < nfds; i++){
		int fd = revents[i].data.fd;
		read(fd, &results, sizeof(results));
		counter++;
		sum += results[2];
		printf("worker %d: %d^%d / %d!, %f\n", (int)results[1], (int)results[0], 
			(int)results[1], (int)results[1], results[2]); //worker 3: 2^3 / 3! : 1.3333
	    }
	}
	close(retfd);
    }

    printf("The mechanism is %s\n", mechanism);
    printf("sum is %f\n", sum);

    return 0;
}
