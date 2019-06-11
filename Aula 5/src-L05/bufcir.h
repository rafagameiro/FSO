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

typedef void * OBJ;

/* bufInit - cria o buffer para o num de objectos indicado
 * bufPut - poe elemento no buffer (este nao pode estar cheio)
 * bufGet - tira o proximo elemento do buffer (este nao pode estar vazio)
 * bufEmpty - devolve !=0 se buffer vazio
 * bufFull - devolve !=0 se buffer cheio
 */

extern int bufInit( int nobj, int objSz );

extern void bufPut( OBJ e );

extern OBJ bufGet( void );

extern int bufEmpty( void );

extern int bufFull( void );
