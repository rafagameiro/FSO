#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER 100


void sys_error( char *msg ) {
	perror(msg);
	exit(1);
}

/* Função que cria e executa o comando definido no parâmetro cmd
   (e.g. "wc", "sort", ou outro) devolvendo o canal ligado ao seu stdin
   através do qual o processo pai vai enviar o conteúdo do ficheiro  */
int launch(char *cmd)
{
	pid_t pid;	// identificador do processo filho
	int fd[2];	
	if(pipe(fd) < 0)
	    sys_error("pipe");
		
	//TODO: c)


	if( (pid=fork()) == -1 ) sys_error(cmd);	// cria novo processo
	else if(pid == 0){
	
		close(0);
		dup(fd[0]);
		close(fd[1]);
		close(fd[0]);

		execlp(cmd, cmd, NULL);
		sys_error(cmd);
	}
		
	//TODO: b), c)
	return fd[1];

}


int main(int argc, char *argv[])
{
	int fdin;	// descritor para ler do ficheiro de entrada
	int f1, f2;
	int num = 10;
	unsigned char bufr[BUFFER];
	//TODO: a)

	if(argc < 4) {
		printf("Uso: %s ficheiro_entrada comando1 comando2\n", argv[0]);
		return 0;
	}
	//TODO: a)
	// abertura do ficheiro de entrada
	fdin = open(argv[1], O_RDONLY);
	if(fdin < 0)
	   sys_error("open");

	// lançamento dos comandos e obter os canais
	f1 = launch(argv[2]);
	f2 = launch(argv[3]);

	// Leitura do ficheiro e envio dos seus dados para processamento
	while((num = read(fdin, bufr, BUFFER)) > 0) {
		printf("ola");
		write(f1, bufr, num);
		write(f2, bufr, num);
		num *=2;
	}

	//TODO: b)
	// O programa tem de esperar a conclusão dos dois comandos

	close(f1);
	close(f2);	
	
	wait(NULL);
	wait(NULL);

	close( fdin );	// fecha o ficheiro de entrada
	return 0;
}

