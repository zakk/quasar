#ifndef __NEXT_H__
#define __NEXT_H__

#include "quasar.h"
#include "movgen.h"

#define INIT		0
#define GETHASH		1
#define GENCAPS		2
#define GETCAPS		3
#define KILLER1		4
#define KILLER2		5
#define KILLER3		6
#define GENBORING	7
#define	GETBORING	8
#define END		9

struct nextctx_t 
{
    DWORD hashmove;
    DWORD moves[MAXMOVES];
    DWORD killer1,killer2,killer3;

    short nextmove,index,phase;
    int values[MAXMOVES];
};

void sort_moves(DWORD mvs[MAXMOVES],int mvsval[MAXMOVES],short current,short idx);
void score_captures(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end);
void score_qmoves(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end);
void score_boring(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end);
DWORD getnext(struct board_t *brd,struct nextctx_t *ctx);

#endif //__NEXT_H__
