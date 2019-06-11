#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

char* string;
char* bufr;
int numOfThreads;
int size;

void fatal_error(char *str){
  perror(str);
  exit(1);
}

void print_result(const char* filename, const char* string, const unsigned int result) {
    printf("Number of times the string \"%s\" occurs in %s: %u\n", string, filename, result);
}

void* countWord(void* arg) {
   int j = 0;
   long counter = 0;
   int limit = ((long) arg) + (size / numOfThreads);
   for(int i = (long) arg; i < limit; i++){
	 if(j == strlen(string)) {
		counter++;
		i--;
		j = 0;
	   }
	   if(bufr[i] == string[j])
		j++; 
	   else
		j = 0;

	if(i == (limit - 1) && j != 0) {
		while(i < strlen(bufr) && j != 0){
		    if(j == strlen(string)) {
			counter++;
			j = 0;
			continue;
	  	    }
	 	    if(bufr[++i] == string[j])
			j++;
       		    else
			j = 0;
		}
        }
	
  }

   return (void*) counter;
}

void* countLastWord(void* arg) {
	printf("aqui??\n");
   int j = 0;
   long counter = 0;
	printf("i: %ld\n", (long) arg);
   for(int i = (long) arg; i < strlen(bufr); i++){
	printf("começa..certo??\n");
	   if(j == strlen(string))
		counter++;

	   if(bufr[i] == string[j])
		j++; 
	   else
		j = 0;
	printf("tamanho do bufr: %ld e i: %d\n", strlen(bufr), i);
	printf("tamanho da string: %ld e j: %d\n", strlen(string), j);
  }

   return (void*) counter;
}

int main(int argc, char *argv[]) {
   string = argv[1];
   unsigned int result = 0;
   int fr;
   struct stat stat;
   pthread_t threads[numOfThreads];

   if ( (numOfThreads = atoi(argv[3])) <= 0 ) 
	fatal_error("atoi");

   fr = open(argv[2], O_RDONLY);
   if (fr < 0) 
	fatal_error("open");

   if( fstat(fr, &stat) != 0) 
	fatal_error("fstat");

   size = stat.st_size;

   bufr = mmap(NULL, size, PROT_READ, MAP_SHARED, fr, 0);
   if( bufr == MAP_FAILED) 
	fatal_error("mmap");
  
   long offset = 0;
   for(int i = 0; i < numOfThreads; i++){
	if(i == numOfThreads - 1 && size % numOfThreads != 0){
		printf("finalmente!!\n");
	   if(pthread_create(&threads[i], NULL, countLastWord,(void*)offset))
		fatal_error("pthread_create"); 
	}else{
	    if(pthread_create(&threads[i], NULL, countWord,(void*)offset))
		fatal_error("pthread_create");
	}
	offset += (size/numOfThreads);
   }
   
   for(int i = 0; i < numOfThreads; i++){
	void* count;
	if(pthread_join(threads[i],&count))
		fatal_error("pthread_join");
	result += (long) count;
   }
  
   print_result(argv[2], string, result);
   
   munmap(bufr, size);

   close(fr);

   return 0;
}



