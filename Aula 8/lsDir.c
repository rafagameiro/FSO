#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

void sys_error(char* msg) {
 perror(msg);
 exit(1);
}

int main(int argc, char* argv[]) {

   DIR* dir;
   struct dirent* fileDir;
   struct stat stats;
   char filename = alloc(sizeof());

   if(argc != 2)
	dir = opendir(".");
   else {
   	dir = opendir(argv[1]);
	filename = argv[1];
   }
   if(dir == NULL)
	sys_error("opendir");
   
   printf("%s\n", filename);
   while((fileDir = readdir(dir)) != NULL) {
	strcat(filename, fileDir->d_name);
	printf("%s\n", filename);
	if(stat(filename, &stats) != -1) {
	     if(S_ISDIR(stats.st_mode))
		   printf("%lu: %u %u:%u %s (dir)\n", stats.st_ino, stats.st_uid, localtime(&stats.st_mtime)->tm_hour, localtime(&stats.st_mtime)->tm_min, fileDir->d_name);
	     else
		   printf("%lu: %u %u:%u %s (%ld)\n", stats.st_ino, stats.st_uid, localtime(&stats.st_mtime)->tm_hour, localtime(&stats.st_mtime)->tm_min, fileDir->d_name, stats.st_size);
	}
   }

  closedir(dir);

  return 0;
}
