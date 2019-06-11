#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define SIZE (200*1024*1024)

int *array;
int count = 0;
int tofind = 3;

void docount( void ){
	for (int i=0; i < SIZE; i++) {
		if(array[i] == tofind){
			count++;
		}
	}
}

int main( int argc, char *argv[]){
	struct timeval t1,t2;

	array= (int *)malloc(SIZE*sizeof(int));
	tofind = 3;

	srand(0);
	for (int i=0; i < SIZE; i++) {
		array[i] = rand() % 4;
	}

	gettimeofday(&t1, NULL);
	docount( );
	gettimeofday(&t2, NULL);


	printf("Count of %d = %d\n", tofind, count);
	printf("Elapsed time (ms) = %lf\n", 
		((t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec))/1000.0 );

	return 0;
}
