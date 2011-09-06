#include <stdio.h>
#include "quasar.h"
#include "hash.h"
#include "hashcodes.h"

#define HASH_ENTRIES	0x00080000

static struct hash_entry_t hashtable[HASH_ENTRIES];

void init_hash(void)
{
    if(sizeof(HASHCODE)!=8)
    {
	printf("%s: internal error, HASHCODE size is %lu bytes (should be 8)\n",
	       __FILE__,sizeof(HASHCODE));
	return;
    }

    hash_set_invalid();
    
    printf("...hashtable initialization done (%d entries, %lu kbytes)...\n",
           HASH_ENTRIES,HASH_ENTRIES*sizeof(struct hash_entry_t)/1024);
}

void hash_set_invalid(void)
{
    int c;
    
    for(c=0;c<HASH_ENTRIES;c++)
	hashtable[c].valid=INVALID;
}

void hash_reset(void)
{
    int c;
    
    for(c=0;c<HASH_ENTRIES;c++)
	if(hashtable[c].valid==VALID)
	    hashtable[c].valid=STALENESS;
}

struct hash_entry_t *hash_probe(HASHCODE hashcode)
{
    struct hash_entry_t *found=&hashtable[hashcode%HASH_ENTRIES];

    if(found->valid==INVALID)
	return NULL;

    if(found->code!=hashcode)
	return NULL;

    found->valid=VALID;
    return found;
}

short hash_insert(HASHCODE hashcode,int score,DWORD move,short depth,BYTE flags)
{
    struct hash_entry_t *slot=&hashtable[hashcode%HASH_ENTRIES];    

    if((slot->code!=hashcode)&&(slot->valid==VALID)&&(slot->depth>depth))
	return FALSE;

    slot->code=hashcode;
    slot->score=score;
    slot->move=move;
    slot->depth=depth;
    slot->flags=flags;
    slot->valid=VALID;

    return TRUE;
}

HASHCODE compute_hashcode(struct board_t *brd)
{
    HASHCODE ret=0;
    short sq,c;
    
    for(sq=0;sq<=63;sq++)
    {
	for(c=0;c<=5;c++)
	{
	    if(ISSET(brd->pcs[WHITE][c],sq))
		ret^=HASHVALUE(c,sq,WHITE);
	    else if(ISSET(brd->pcs[BLACK][c],sq))
		ret^=HASHVALUE(c,sq,BLACK);
	}
    }
    
    ret^=HASHCASTLE(brd->castling);
    
    if(brd->ep!=-1)
	ret^=HASHEP(brd->ep);

    if(brd->color==BLACK)
	ret^=HASHSIDE;

    return ret;
}
