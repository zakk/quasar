#include <string.h>
#include "quasar.h"
#include "make.h"
#include "null.h"
#include "hash.h"

void makenull(struct board_t *brd)
{
    /*
	ovviamente questa funzione non cerca se e'consentito fare la nullmove,
	la fa e basta.
	controlli se siamo sotto scacco, sul rischio di zugzwang, se abbiamo fatto
	due nullmoves consecutive vanno fatti nel codice di ricerca
    */

    if(brd->ep!=-1)
    	brd->hashcode^=HASHEP(brd->ep);

    brd->ep=-1;
    brd->ply++;
    brd->color^=1;
    brd->hashcode^=HASHSIDE;
}

void unmakenull(struct board_t *brd,short oldep)
{
    if(oldep!=-1)
	brd->hashcode^=HASHEP(oldep);

    brd->ep=oldep;
    brd->ply--;
    brd->color^=1;
    brd->hashcode^=HASHSIDE;
}
