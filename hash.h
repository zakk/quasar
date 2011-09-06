#ifndef __HASH_H__
#define __HASH_H__

#include "quasar.h"

#define EXACT		0
#define LOBOUND		1
#define HIBOUND		2

#define INVALID		1
#define STALENESS	2
#define VALID		3

extern const HASHCODE hcodes[64][16];

#define HASHVALUE(pcs,sq,side)	(hcodes[sq][(pcs)|(side<<3)])
#define HASHCASTLE(flags)	(hcodes[flags][7])
#define HASHEP(target)		(hcodes[target][15])
#define HASHSIDE		(hcodes[0][15])

/*
    l'attributo ((packed)) su ogni campo della struttura
    impedisce alcune ottimizzazioni del gcc, che non hanno
    un riscontro prestazionale decente, e aumentano del 20%
    l'occupazione dell'hashtable (diminuendo quindi del 20%
    gli slot disponibili a parità di memoria allocata)
*/

struct hash_entry_t
{
    HASHCODE code	__attribute__ ((packed));
    
    int score		__attribute__ ((packed));
    short depth		__attribute__ ((packed));

    DWORD move		__attribute__ ((packed));
    BYTE flags		__attribute__ ((packed));
    BYTE valid		__attribute__ ((packed));
};

void init_hash(void);
void hash_set_invalid(void);
void hash_reset(void);
struct hash_entry_t *hash_probe(HASHCODE hashcode);
short hash_insert(HASHCODE hashcode,int score,DWORD move,short depth,BYTE flags);
HASHCODE compute_hashcode(struct board_t *brd);

#endif
