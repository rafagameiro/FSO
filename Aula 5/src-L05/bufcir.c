/************************************************
 *  bufcir.h
 *
 *  Interface para um buffer circular
 *  Implementacao em bufcir.c
 * (baseado no bufcir AC)
 *
 * (c) 2017 DI-FCT/UNL - Vitor Duarte
 * FSO 1718 - Mest. Int. Eng. Informatica
 *
 * Out 2017 - VD
 ************************************************/

/* cada elemento no buffer do tipo OBJ (qualquer referencia)*/

/* se compilado com DEBUG definido sao verificadas assercoes
 * na entrada das operacoes bufPut e bufGet
 */
#define DEBUG

#include <stdlib.h>
#include "bufcir.h"

#ifndef DEBUG
  #define NDEBUG
#endif

#include <assert.h>



static OBJ *theBuf;  // deve ser criado com bufInit
static int put=0, get=0, num=0;
static int bufSz=0;

int bufInit( int nobj, int objSz )
{
  theBuf = (OBJ*) malloc( nobj*sizeof(OBJ) );
  if ( theBuf==NULL ) return 0; //KO
  bufSz = nobj;

  //alternativa: o buffer tem os objectos já alocados
  /*for ( int i = 0; i<bufSz; i++ ){
    theBuf[i]=(OBJ)malloc(objSz);
    if ( theBuf[i]==NULL ) return 0; //ERROR
  }*/
  return 1; //OK
}


void bufPut( OBJ e )
{
  assert( num<bufSz );

  theBuf[put] = e;
  put = (put+1)%bufSz;
  num += 1;
}


OBJ bufGet( void )
{
    OBJ c;
    if (num==0) return NULL;
    assert( num>0 );

    num -= 1;
    c = theBuf[get];
    get = (get+1)%bufSz;
    return c;
}


int bufEmpty( void )
{ return num==0; }


int bufFull( void )
{ return num==bufSz; }
