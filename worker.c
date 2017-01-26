//compile it with -lm flag to receive math library
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>

double better_fac(double product, double counter, double max_count)
{
    if(counter > max_count){
	return product;
    } else {
	return better_fac(product * counter, counter + 1, max_count);
    }
};

double fac(double x)
{
    return better_fac(1, 1, x);
    // by doing counter we avoid linear growth in recursive calling
    // fac(n) = n * fac(n - 1)
}



int main(int argc, char* argv[])
{
    struct stat sb;
    // expected format of the argc[] would be ./worker -x 2 -n 3
    if(argc != 5)
    {
	perror("wrong number of argument!\n");
	exit(1);
    }

    int x = atof(argv[2]);
    int n = atof(argv[4]);
    double result = pow(x, n) / fac(n);

    if(isatty(STDIN_FILENO)){
	printf("x^n / n! : %f\n", result);
    } else {
	write(STDOUT_FILENO, &result, sizeof(double));
    }

    return 0;
}
