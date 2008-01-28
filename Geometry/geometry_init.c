/*******************************************************************************
*
* File geometry_init.c
*
* Inizialization of geometry structures
*
*******************************************************************************/

#include "geometry.h"
#include "global.h"
#include "error.h"
#include <stdlib.h>

static int init=1;
static int *alloc_mem=NULL;

static void free_memory() {
	if(alloc_mem!=NULL) {
		free(alloc_mem);
		alloc_mem=NULL;
		iup=idn=NULL;
		ipt=NULL;
		ipt_4d=NULL;
	}
}

void geometry_init() {
	if (init) {
		int *cur;
		size_t req_mem=0;
		req_mem+=2*4*VOLUME; /* for iup and idn */
		req_mem+=VOLUME;     /* for ipt */
		req_mem+=VOLUME;     /* for ipt_4d */

		alloc_mem=malloc(req_mem*sizeof(int));
		error((alloc_mem==NULL),1,"geometry_init [geometry_init.c]",
         "Cannot allocate memory");

		cur=alloc_mem;
#define ALLOC(ptr,size) ptr=cur; cur+=(size) 

		/* iup and idn */
		ALLOC(iup,4*VOLUME);
		ALLOC(idn,4*VOLUME);
		/* ipt */
		ALLOC(ipt,VOLUME);
		/* ipt_4d */
		ALLOC(ipt_4d,VOLUME);

#undef ALLOC

		atexit(&free_memory);

		init=0;
	}
}










