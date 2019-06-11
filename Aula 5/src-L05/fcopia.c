/************************************************************************
 * FSO 17/18
 * 
 * copiar ficheiros usando stdio: fopen, fread, fwrite...
 * (versao multithreaded)
 * 
 * (c) 2017 DI-FCT/UNL - Vitor Duarte
 * FSO 1718 - Mest. Int. Eng. Informatica
 *
 * Out 2017 - VD
 ***********************************************************************/

//#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

/** a minha "message box" **/
#include "bufcir.h"

#define NBLK  64  // num blocos
#define BLKSZ 4096  // tamanho de cada bloco

typedef struct {
  int size;
  unsigned char buffer[BLKSZ];
} msg_t;



void put( msg_t *b )	// deve funcionar em concorrencia com o get!
{
  OBJ tmp = malloc( sizeof(msg_t) );
  memcpy(tmp, b, sizeof(msg_t));   // necessita de ser copiado. PORQUE?

  while ( bufFull() )
    ;  		// retirar espera ativa

  bufPut(tmp);

}

msg_t *get( void )	// deve funcionar em concorrencia com o put!
{
  
  while ( bufEmpty() )
    ;  		// retirar espera ativa

  msg_t *r = (msg_t*)bufGet();

  return r;
}


/****/



void aborta( char *msg )
/* aborta o programa apos afixar uma mensagem no STDERR */
{
  if (msg != NULL) fprintf(stderr, "%s\n", msg);
  exit(1);
}




void *ler(void*arg)
{ FILE* fin;
  msg_t buf;

  printf("reading %s\n", arg );
  fin = fopen( arg, "r" );
  if ( fin == NULL ) aborta( "Erro no primeiro ficheiro" );
  while( (buf.size = fread( &(buf.buffer[0]), 1, BLKSZ, fin )) > 0 )
  {
    put( &buf );
  }
  if ( buf.size < 0 ) aborta( "Erro na leitura" );
  buf.size=0;
  put(&buf);	// necessitamos marcar o fim. PORQUE?
  printf("reading done\n" );  
  fclose(fin);
  return NULL;
}

void *escrever(void*arg)
{ FILE * fout;
  int nw;
  msg_t *b;

  printf("writing %s\n", arg );  
  fout = fopen( arg, "w" );
  if ( fout == NULL ) aborta( "Erro no segundo ficheiro" );
  while( (b = get()) != NULL && b->size>0 )
  {
    nw = fwrite( b->buffer, 1, b->size, fout );
    if ( nw < b->size ) aborta( "Erro na escrita" );
    free(b);	// necessita do free. PORQUE?
  }
  printf("writing done\n" );    
  fclose(fout);
  return NULL;
}

int main( int argc, char *argv[] )
{
  pthread_t p, c;

  if ( argc != 3 )
	  aborta( "Numero errado de argumentos [copia f1 f2]" );
  if ( !bufInit( NBLK, sizeof(msg_t) ) )
    aborta( "Erro a criar buffer" );

  if ( pthread_create( &p, NULL, ler, argv[1])!=0 ) aborta("thread1");
  if ( pthread_create( &c, NULL, escrever, argv[2])!=0 ) aborta("thread2");
  pthread_join(p, NULL);
  pthread_join(c, NULL);

  return 0;  /* terminou bem */
}
