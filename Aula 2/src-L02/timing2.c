#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NTRIES  1000

int do_something(void) { return 1; }

int main(int argc, char *argv[]) {
    int i, p;
    long elapsed;
    struct timeval t1,t2;

    gettimeofday(&t1, NULL);
    for (i = 0;  i < NTRIES; i++)
    {
	char*args[2];
	switch(fork()){
		case -1: perror("fork");exit(1);
		case 0:  args[0] = strdup("./dummy"); args[1] = NULL;
			 execvp(args[0], args); perror("This should not happen");
			 exit(1);
		dafault: wait(NULL); //wait for child exit
	}	
    }
    gettimeofday(&t2, NULL);
    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
    printf("Elapsed time = %6li us (%g us/call)\n", elapsed, (double)elapsed/NTRIES);
    return 0;
}

