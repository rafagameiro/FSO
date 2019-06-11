
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// declarações globais
int number_workers;
long *results;
int done = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//TODO: b) 



long some_computation(long i) {
   //sleep(1);
   return i+1;	// fake work
}



void* worker(void *arg) {
      long id = (long)(arg); 
      results[id] = some_computation(id);
    //TODO: b)
      pthread_mutex_lock(&mutex);
	done++;
	if(done == number_workers)
		pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
      return NULL;
}

void* reducer(void *arg) {
    //TODO: b)
    pthread_mutex_lock(&mutex);
    while(done != number_workers)
	pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);


    // Este código só pode executar depois de todos os threads worker
    // terem terminado de executar a função some_computation
    long result = 0;
    for (int i = 0; i < number_workers; i++)
        result += results[i];

    return (void*) result;	//TODO: c)
}


int main( int argc, char *argv[] ) {
  // declarações locais
  number_workers = atoi(argv[1]);
  results = malloc(number_workers * sizeof(long));	//TODO: a)
  //TODO: a) 
  pthread_t worktid[number_workers];
  //TODO: a) 
  pthread_t redutid;



  // lança number_workers threads para executarem a função worker
  // TODO: a)
  for(int i = 0; i < number_workers; i++)
	pthread_create(&worktid[number_workers], NULL, worker, (void*) ((long)i));



  // lança um thread para executar a função reducer
  // TODO: a)
  pthread_create(&redutid, NULL, reducer, NULL);


  for (int i = 0; i < number_workers; i++)
     pthread_join(worktid[i], NULL);

  long finalresult;
  pthread_join(redutid, (void*) &finalresult);	//TODO: c)

  printf ("result: %ld\n", finalresult);
  return 0;
}


