
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

void Getrlimit( int limitType, struct rlimit *r){
	int res = getrlimit( limitType, r);
	if(res < 0){ perror("getrlimit"); exit(1);}
}

int main(int argc, char *argv[])
{
    struct rlimit r;

    r.rlim_cur = 10000*1024;
    setrlimit(RLIMIT_AS, &r);
	
    char array[9*1024*1024];
   
    

    Getrlimit( RLIMIT_AS, &r);
    printf("Maximum size of process virtual addres space; soft limit = %u, hard limit = %u\n", 
               (unsigned int)r.rlim_cur, (unsigned int)r.rlim_max);

    Getrlimit( RLIMIT_DATA, &r);
    printf("Maximum size of process's data segment(intialized, uninitiliazed, heap); soft limit = %u, hard limit = %u\n",
               (unsigned int)r.rlim_cur, (unsigned int)r.rlim_max);

    Getrlimit( RLIMIT_STACK, &r);
    printf("Maximum size of process stack; soft limit = %u, hard limit = %u\n", 
               (unsigned int)r.rlim_cur, (unsigned int)r.rlim_max);
    return 0;
}

