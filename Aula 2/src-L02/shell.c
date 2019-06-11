#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARGVMAX 100
#define LINESIZE 1024

int makeargv(char *s, char* argv[ARGVMAX]) {
   //pre: argv is
   //out: argv[] points to all words in the string s ()
   //return: number of words pointed to by the elements in argv (or -1 in case of error)

   int ntokens;

   if ( s== NULL || argv == NULL || ARGVMAX == 0)
	return -1;
   ntokens = 0;
   argv[ntokens] = strtok(s, " \t\n");
   while( (argv[ntokens] != NULL) && (ntokens < ARGVMAX)){
	ntokens++;
	argv[ntokens] = strtok(NULL, " \t\n"); //breaks 's' inline at seperators
   }
   argv[ntokens] = NULL; //terminate with NULL reference
   
   return ntokens;
}

void filhoOUT(char* argv[], int p[]) {

   dup2(p[1], 1);
   close(p[0]);
   close(p[1]);
   execvp(argv[0], argv);
 
}

void filhoIN(char* argv[], int p[]) {

   dup2(p[0], 0);
   close(p[1]);
   close(p[0]);
   execvp(argv[0], argv);

}

void runcommand(char* argv[]) {
   
   if(!strcmp(argv[0], "exit"))
	exit(0);

   int pos = -1;
   int i = 0;
   
   while(argv[i] != NULL) {
	if(strchr(argv[i],'|') != NULL)
		pos = i;
	i++;
   }

   if(pos == -1) {

	switch(fork()) {
	case -1 : perror("fork");exit(1);
	case 0:	execvp(argv[0], argv); 
		exit(1);			
	default: wait(NULL); //wait for child exit
	}

   }else {
	char *firstCmd[ARGVMAX] = {};
	char *secondCmd[ARGVMAX] = {};
	int p[2];	

	for(int j = 0; j < pos; j++)
	    firstCmd[j] = argv[j];

	for(int k = pos+1; k < i; k++)
	    secondCmd[k-(pos+1)] = argv[k];

   	if(pipe(p) == -1)
	     abort();

	switch(fork()) {
	case -1 : exit(1);
	case 0: filhoOUT(firstCmd, p);
		exit(1);
	default: 
	  switch(fork()) {
		case -1: exit(1);
		case 0: filhoIN(secondCmd, p);
			exit(1);
		default: break;
	  }
	  close(p[0]);
	  close(p[1]);
	  wait(NULL);
	  wait(NULL);
	}

   }
}

int main() {

  char line[LINESIZE];
  char* av[ARGVMAX];

  printf("> "); fflush(stdout); //writes the prompt on the standard output while ( fgets( line, LINESIZE, stdin) != NULL) {

  while( fgets(line, LINESIZE, stdin) != NULL) {
	if( makeargv(line, av) > 0)
	   runcommand(av);
	printf("> "); fflush(stdout);
  }

  return 0;

}
