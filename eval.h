#ifndef __EVAL_H__
#define __EVAL_H__

#include "quasar.h"

#define PAWNVAL		100
#define KNIGHTVAL	350
#define BISHOPVAL	350
#define ROOKVAL		550
#define QUEENVAL	1100

#define PHASE(brd)	(8-((brd->material[WHITE]+brd->material[BLACK])/1150))

struct eval_ctx_t
{
    BITBOARD pawnctr[2];

#define WHITEOPEN	1
#define BLACKOPEN	2

    short openfiles[2];
    short npawns[2];
    short phase;
};

int eval(struct board_t *brd,short side,int alpha,int beta);
void init_eval_bitboards(void);

#endif //__EVAL_H__
