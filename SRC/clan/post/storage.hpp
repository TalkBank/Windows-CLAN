/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* storage.hpp
	version 0.99 : 21 octobre 1997
	This handle virtual memory access (either in memory or on disk)
	- there is only allocation function. Everything is freed globally at the end of the program.
*/

#ifndef __STORAGE_HPP__
#define __STORAGE_HPP__

#define Ko (1024L)
#define Mo (1024L*1024L)

typedef long32 address;	/* this is a relative memory location */

// this is for error recovery.
extern jmp_buf mark;              /* Address for long32 jump to jump to */

const int vmRealMemory = 1;
const int vmFileMemory = 2;
const int vmMachineDependentVirtualMemory = 3;

const address NILMEMORY = 0L;

/*** Fonctions d'accès disponibles dans tous les programmes ***/
address vmAllocate( int nbbytes );
void vmRead( void* adr, address vmadr, int nbbytes );
void vmWrite( const void* adr, address vmadr, int nbbytes );

/*** Fonctions d'initialisation et de sauvegarde (acces limite de preference) ***/
int vmCreate( FNType* vmexternalname, int vmtype, long32 vmavesize, int reversed, long32 vminitsize = 0L );
int vmOpen(  FNType* vmexternalname, int vmtype, long32 vmavesize, int reversed, int RW = 1 );
void vmSave();
void vmClose();

/*** read and write memory the normal way or in reverse order, depending on the setting of a global flag ***/
short int vmReadShort( address vmadr );
int vmReadInt( address vmadr );
long32 vmReadLong( address vmadr );
double vmReadDouble( address vmadr );
void	vmWriteShort( short int v, address vmadr );
void	vmWriteInt( int v, address vmadr );
void	vmWriteLong( long32 v, address vmadr );
void	vmWriteDouble( double v, address vmadr );

int vmIsReversed(); // returns 1 if memory is upside-down
void vmSetReversed(int reversed); // sets the reverse flag

void flipLong(Int4 *num);
void flipShort(Int2 *num);

/*** Management of a local pool of memory ***/
char* dynalloc(unsigned size);
void  dynreset();
void  dynfreeall();

#endif /* __STORAGE_HPP__ */
