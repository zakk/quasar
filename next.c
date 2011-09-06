#include <stdio.h>
#include "quasar.h"
#include "movgen.h"
#include "search.h"
#include "see.h"
#include "next.h"
#include "legal.h"
#include "eval.h"

void sort_moves(DWORD mvs[MAXMOVES],int mvsval[MAXMOVES],short current,short idx)
{
    short best,c;
    int bestscore;

    best=current;
    bestscore=-INFINITY;

    for(c=current;c<idx;c++)
    {
	if(mvsval[c]>bestscore)
	{
	    bestscore=mvsval[c];
	    best=c;
	}
    }

    if(best!=current)
    {
	DWORD oldmove;
	int oldval;
	
	oldmove=mvs[current];
	mvs[current]=mvs[best];
	mvs[best]=oldmove;
	
	oldval=mvsval[current];
	mvsval[current]=mvsval[best];
	mvsval[best]=oldval;
    }
}

void score_captures(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end)
{
    short c;
    
    for(c=start;c<end;c++)
    {
	mvsval[c]=see(brd,mvs[c]);
    }
}

void score_qmoves(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end)
{
    short c;
    
    for(c=start;c<end;c++)
    {
	if((mvs[c]>>18)&QUEENPRM)
	    mvsval[c]=QUEENVAL;
	else if((mvs[c]>>18)&(KNIGHTPRM|ROOKPRM|BISHOPPRM))
	    mvsval[c]=0;
	else
	    mvsval[c]=see(brd,mvs[c]);
    }
}

void score_boring(struct board_t *brd,DWORD mvs[],int mvsval[],int start,int end)
{
    short c;
    
    for(c=start;c<end;c++)
    {
	if(brd->board[(mvs[c]>>6)&0x3F]!=EMPTY)
	    continue;

	if((mvs[c]>>18)&ENPASSANT)
	    continue;

	mvsval[c]=move_history[mvs[c]&4095];
    }
}

DWORD getnext(struct board_t *brd,struct nextctx_t *ctx)
{
    DWORD move;
    int value;

    switch(ctx->phase)
    {
	case INIT:

	ctx->killer1=killer1[brd->ply];
	ctx->killer2=killer2[brd->ply];
	ctx->killer3=killer3[brd->ply];

	case GETHASH:

	ctx->phase=GENCAPS;
	if(islegal(brd,ctx->hashmove)==TRUE)
	    return ctx->hashmove;
	
	case GENCAPS:

	ctx->phase=GETCAPS;
	ctx->nextmove=0;
	ctx->index=0;
	generate_captures(brd,ctx->moves,&ctx->index);
	score_captures(brd,ctx->moves,ctx->values,0,ctx->index);

	case GETCAPS:

	while(ctx->nextmove<ctx->index)
	{
	    sort_moves(ctx->moves,ctx->values,ctx->nextmove,ctx->index);
	    move=ctx->moves[ctx->nextmove];
	    value=ctx->values[ctx->nextmove];

#define THRESHOLD	-55

	    if(value<THRESHOLD)
		break;
	    
	    ctx->nextmove++;
	    if(move==ctx->hashmove)
		continue;
	    
	    if((move==ctx->killer1)||(move==ctx->killer2)||(move==ctx->killer3))
	    	continue;

	    return move;
	}

	case KILLER1:

	ctx->phase=KILLER2;
	if((ctx->killer1!=ctx->hashmove)&&(islegal(brd,ctx->killer1)==TRUE))
	    return ctx->killer1;

	case KILLER2:

	ctx->phase=KILLER3;
	if((ctx->killer2!=ctx->hashmove)&&(islegal(brd,ctx->killer2)==TRUE))
	    return ctx->killer2;

	case KILLER3:

	ctx->phase=GENBORING;
	if((ctx->killer3!=ctx->hashmove)&&(islegal(brd,ctx->killer3)==TRUE))
	    return ctx->killer3;

	case GENBORING:

	ctx->phase=GETBORING;
	generate_noncaptures(brd,ctx->moves,&ctx->index);
	score_boring(brd,ctx->moves,ctx->values,ctx->nextmove,ctx->index);

	case GETBORING:

	while(ctx->nextmove<ctx->index)
	{
	    sort_moves(ctx->moves,ctx->values,ctx->nextmove,ctx->index);
	    move=ctx->moves[ctx->nextmove];
	    ctx->nextmove++;

	    if(move==ctx->hashmove)
		continue;
	    
	    if((move==ctx->killer1)||(move==ctx->killer2)||(move==ctx->killer3))
		continue;

	    return move;
	}

	case END:
	ctx->phase=END;
	return 0;
    
	default:
	printf("%s: invalid phase in getnext\n",__FILE__);
	return 0;
    }
}
