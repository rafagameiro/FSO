#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

void fatal_error(char *str){
  perror(str);
  exit(1);
}

void mmapCopy(int fr, int fw, int size, int offSet) {  
    char *bufr, *bufw;
    
    bufr = mmap(NULL, size, PROT_READ, MAP_SHARED, fr, offSet);
    if( bufr == MAP_FAILED) 
	fatal_error("mmap");

    bufw = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fw, offSet);
    if(bufw == MAP_FAILED)
	fatal_error("mmap");
 
    memcpy(bufw, bufr, size);

   munmap(bufr, size);
   munmap(bufw, size);
}

void mmapCopyRec(int fr, int fw, int size, int totalSize, int offSet) {
    if(totalSize < size)
	return mmapCopy(fr, fw, totalSize, offSet);
    else {
	mmapCopy(fr, fw, size, offSet);
	offSet += size;
	totalSize -= size;
	return mmapCopyRec(fr, fw, size, totalSize, offSet);
    }	
}

int main(int argc, char *argv[]) {
    struct stat stat;
    int fr, fw, bsize, psize;

    psize = sysconf(_SC_PAGE_SIZE);

    if (argc != 4) {
	printf("Numero errado de argumentos [copia tamanho f1 f2]");
	exit(1);
    }
  
    if ( (bsize = atoi( argv[1] )) <= 0 ) 
	fatal_error("atoi");

    fr = open(argv[2], O_RDONLY, 0);
    if(fr < 0)
	fatal_error("open");

    fw = open(argv[3], O_RDWR|O_CREAT|O_TRUNC);
    if(fw < 0)
	fatal_error("open");

    if(fstat(fr, &stat) != 0 )
	fatal_error("fstat");

    ftruncate(fw, stat.st_size);
   
    if(psize > bsize && psize > stat.st_size)
    	mmapCopy(fr, fw, stat.st_size, 0);
    else if(psize > bsize && psize < stat.st_size)
	mmapCopyRec(fr, fw, bsize, stat.st_size, 0);
    else
	mmapCopyRec(fr, fw, psize, stat.st_size, 0);

    close(fr);
    close(fw);

    return 0;
}

