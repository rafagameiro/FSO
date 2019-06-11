#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NTRIES  1000

void *mythread(void *a){
    return NULL;
}


int main(int argc, char *argv[]) {
    int i;
    long elapsed;
    struct timeval t1,t2;
    pthread_t tid;

    gettimeofday(&t1, NULL);
    for (i = 0;  i < NTRIES; i++)
        if ( pthread_create(&tid, NULL, mythread, NULL) == 0 )
		pthread_join( tid, NULL );
        else abort();

    gettimeofday(&t2, NULL);
    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
    printf("Elapsed time = %6li us (%g us/call)\n", elapsed, (double)elapsed/NTRIES);
    return 0;
}

