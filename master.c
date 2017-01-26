#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char *argv[])
{
    
    pid_t cpid;
    char buffer[256];
    double result;
    double sum = 0.0;
    char *mechanism = argv[4];
    int worker_num = atoi(argv[8]);

    printf("worker number is %d\n", worker_num);

    int pipes[2];
    for(int i = 0; i < worker_num; i++){
	if(pipe(pipes) == -1){
	    perror("failed init pipe\n");
	    exit(1);
	};
    }

    pid_t child_pids[worker_num];


    for(int i = 0; i < atoi(argv[8]); i++){
	printf("start creating pid %d\n", i);
	cpid = fork();
	if(cpid == -1){
	    perror("fork failed\n");
	    exit(1);
	}

	if(cpid == 0){
	    // child process
	    child_pids[i] = getpid();

	    printf("my pid is: %d\n", child_pids[i]);
	    int rs = dup2(pipes[0], STDIN_FILENO);
	    int rs2 = dup2(pipes[1], STDOUT_FILENO);
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
	    read(pipes[0], &result, sizeof(double));
	    sum += result;
	}
    }

    printf("The mechanism is %s", mechanism);
    printf("sum is %f\n", sum);

    return 0;
}
