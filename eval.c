#include <stdlib.h>

#include "quasar.h"
#include "eval.h"
#include "pcsq.h"

BITBOARD passed[2][64];
BITBOARD isolated[8];
BITBOARD maskWBT,maskBBT;

short distance[64][64];

#define KNIGHT_KING_ATTACK	6
#define KNIGHT_MOBILITY		2

#define BISHOP_KING_ATTACK	6
#define BISHOP_MOBILITY		3
#define BISHOP_TRAPPED		175
#define BISHOP_GOOD_PAWN	4

const short bishop_pair[]={20,20,20,20,20,20,20,15,8};

#define ROOK_KING_ATTACK	5
#define ROOK_HALF_OPEN_FILE	7
#define ROOK_OPEN_FILE		11
#define ROOK_ON_7TH		30
#define ROOK_CONNECTED_ON_7TH	16
#define ROOK_MOBILITY		1
#define ROOK_FENCED		14

#define QUEEN_ROOK_ON_7TH	15
#define QUEEN_KING_ATTACK	6
#define QUEEN_OPEN_FILE		4

#define PAWN_DOUBLED		5
#define PAWN_DUO		2
#define BLOCKED_CENTER_PAWN	12

const short passed_score[2][8]={{0,240,192,144,96,48,48,0},{0,48,48,96,144,192,240,0}};
const short pawn_isolated[8]={8,10,12,14,14,12,10,8};
const short pawn_isolated_weaker[8]={22,24,26,28,28,26,24,22};

int eval_knight(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    short d,sq;
    BITBOARD x,tmp;

    tmp=brd->pcs[WHITE][KNIGHT];
    while(tmp)
    {
	sq=LASTONE(tmp);
	
	score+=knightsq[WHITE][sq];
	
	d=distance[sq][brd->king[BLACK]];
	if(d<=3)
	    score+=(4-d)*KNIGHT_KING_ATTACK;
	
	x=knight_attacks[sq]&(~(ctx->pawnctr[BLACK]|brd->friends[WHITE]));
	score+=BITCOUNT(x)*KNIGHT_MOBILITY;

	if((outpost[WHITE][sq]!=0)&&(!(brd->pcs[BLACK][PAWN]&isolated[sq&0x7]&passed[WHITE][sq])))
	{
	    short defenders;
	    
	    score+=outpost[WHITE][sq];
	    
	    x=pawn_attacks[BLACK][sq]&brd->pcs[WHITE][PAWN];
	    defenders=BITCOUNT(x);

	    if(defenders==2)
		score+=outpost[WHITE][sq];
	    else if(defenders==1)
		score+=outpost[WHITE][sq]/2;
	
	    /* blocks center pawn prio:low */
	}

	/* white knight blocks passer prio:medium */

	CLEARBIT(tmp,sq);
    }

    tmp=brd->pcs[BLACK][KNIGHT];
    while(tmp)
    {
	sq=FIRSTONE(tmp);
	
	score-=knightsq[BLACK][sq];
	
	d=distance[sq][brd->king[WHITE]];
	if(d<=3)
	    score-=(4-d)*KNIGHT_KING_ATTACK;
	
	x=knight_attacks[sq]&(~(ctx->pawnctr[WHITE]|brd->friends[BLACK]));
	score-=BITCOUNT(x)*KNIGHT_MOBILITY;

	if((outpost[BLACK][sq]!=0)&&(!(brd->pcs[WHITE][PAWN]&isolated[sq&0x7]&passed[BLACK][sq])))
	{
	    short defenders;
	    
	    score-=outpost[BLACK][sq];

	    x=pawn_attacks[WHITE][sq]&brd->pcs[BLACK][PAWN];
	    defenders=BITCOUNT(x);
	    
	    if(defenders==2)
		score-=outpost[BLACK][sq];
	    else if(defenders==1)
		score-=outpost[BLACK][sq]/2;
	}

	CLEARBIT(tmp,sq);
    }

    return score;
}

int eval_bishop(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    short d,sq;
    BITBOARD x,tmp;

    const BITBOARD light=0xAA55AA55AA55AA55ULL;
    const BITBOARD dark=0x55AA55AA55AA55AAULL;

    tmp=brd->pcs[WHITE][BISHOP];
    
    if(tmp&(tmp-1))
	score+=bishop_pair[ctx->npawns[WHITE]];
    else if((x=tmp&light))
	score+=BITCOUNT(x)*BISHOP_GOOD_PAWN;
    else if((x=tmp&dark))
	score+=BITCOUNT(x)*BISHOP_GOOD_PAWN;

    while(tmp)
    {
	sq=LASTONE(tmp);
	
	score+=bishopsq[WHITE][sq];
	    
	d=distance[sq][brd->king[BLACK]];
	if(d<=3)
	    score+=(4-d)*BISHOP_KING_ATTACK;	

	x=bishop_attacks(brd,sq)&(~(ctx->pawnctr[BLACK]|brd->friends[WHITE]));
	score+=BITCOUNT(x)*BISHOP_MOBILITY;
	
	/* blocked center pawn  prio:low    */

	CLEARBIT(tmp,sq);
    }

    tmp=brd->pcs[BLACK][BISHOP];

    if(tmp&(tmp-1))
	score-=bishop_pair[ctx->npawns[BLACK]];
    else if((x=tmp&light))
	score-=BITCOUNT(x)*BISHOP_GOOD_PAWN;
    else if((x=tmp&dark))
	score-=BITCOUNT(x)*BISHOP_GOOD_PAWN;

    while(tmp)
    {
	sq=FIRSTONE(tmp);

	score-=bishopsq[BLACK][sq];
	    
	d=distance[sq][brd->king[WHITE]];
	if(d<=3)
	    score-=(4-d)*BISHOP_KING_ATTACK;

	x=bishop_attacks(brd,sq)&(~(ctx->pawnctr[WHITE]|brd->friends[BLACK]));
	score-=BITCOUNT(x)*BISHOP_MOBILITY;

	CLEARBIT(tmp,sq);
    }

    return score;
}

int eval_rook(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    short d,sq,rank,file,mobility;
    BITBOARD x,tmp;

    tmp=brd->pcs[WHITE][ROOK];
    while(tmp)
    {
	sq=LASTONE(tmp);
	file=sq&0x7;
	rank=sq>>3;
	
	score+=rooksq[WHITE][sq];

	d=distance[sq][brd->king[BLACK]];
	if(d<=3)
	    score+=(4-d)*ROOK_KING_ATTACK;
	
	if(ctx->openfiles[file]&WHITEOPEN)
	{
	    if(ctx->openfiles[file]&BLACKOPEN)
		score+=ROOK_OPEN_FILE;
	    else
		score+=ROOK_HALF_OPEN_FILE;
	}
	
	if((rank==1)&&(((brd->king[BLACK]>>3)==0)||(brd->pcs[BLACK][PAWN]&rankbits[1])))
	{
	    BITBOARD rankattacks=rook00attacks[sq][(brd->occ>>s00[sq])&0xFF];
	
	    score+=ROOK_ON_7TH;
	
	    if(rankattacks&(brd->pcs[WHITE][ROOK]|brd->pcs[WHITE][QUEEN]))
		score+=ROOK_CONNECTED_ON_7TH;
	}

	x=rook_attacks(brd,sq)&(~(ctx->pawnctr[BLACK]|brd->friends[WHITE]));
	mobility=BITCOUNT(x);

	if(mobility==0)
	    score-=ROOK_FENCED;
	else if(mobility==1)
	    score-=ROOK_FENCED/2;
	else
	    score+=mobility*ROOK_MOBILITY;
	
	CLEARBIT(tmp,sq);
    }

    tmp=brd->pcs[BLACK][ROOK];
    while(tmp)
    {
	sq=FIRSTONE(tmp);
	file=sq&0x7;
	rank=sq>>3;
	
	score-=rooksq[BLACK][sq];

	d=distance[sq][brd->king[WHITE]];
	if(d<=3)
	    score-=(4-d)*ROOK_KING_ATTACK;
	
	if(ctx->openfiles[file]&BLACKOPEN)
	{
	    if(ctx->openfiles[file]&WHITEOPEN)
		score-=ROOK_OPEN_FILE;
	    else
		score-=ROOK_HALF_OPEN_FILE;
	}
	
	if((rank==6)&&(((brd->king[WHITE]>>3)==7)||(brd->pcs[WHITE][PAWN]&rankbits[6])))
	{
	    BITBOARD rankattacks=rook00attacks[sq][(brd->occ>>s00[sq])&0xFF];
	
	    score-=ROOK_ON_7TH;
	
	    if(rankattacks&(brd->pcs[BLACK][ROOK]|brd->pcs[BLACK][QUEEN]))
		score-=ROOK_CONNECTED_ON_7TH;
	}

	x=rook_attacks(brd,sq)&(~(ctx->pawnctr[WHITE]|brd->friends[BLACK]));
	mobility=BITCOUNT(x);

	if(mobility==0)
	    score+=ROOK_FENCED;
	else if(mobility==1)
	    score+=ROOK_FENCED/2;
	else
	    score-=mobility*ROOK_MOBILITY;

	CLEARBIT(tmp,sq);
    }

    return score;
}

int eval_queen(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    short d,sq;
    BITBOARD tmp;

    tmp=brd->pcs[WHITE][QUEEN];
    while(tmp)
    {
	sq=LASTONE(tmp);

	score+=queensq[WHITE][sq];

	d=distance[sq][brd->king[WHITE]];
	if(d<=3)
	    score+=(4-d)*(4-d)*QUEEN_KING_ATTACK;
	
	if(ctx->openfiles[sq&0x7]&(WHITEOPEN|BLACKOPEN))
	    score+=QUEEN_OPEN_FILE;    

	if(((sq>>3)==1)&&(((brd->king[BLACK]>>3)==0)||(brd->pcs[BLACK][PAWN]&rankbits[1])))
	{
	    BITBOARD rankattacks=rook00attacks[sq][(brd->occ>>s00[sq])&0xFF];
	
	    if(rankattacks&brd->pcs[WHITE][ROOK])
		score+=QUEEN_ROOK_ON_7TH;
	}

	CLEARBIT(tmp,sq);
    }

    tmp=brd->pcs[BLACK][QUEEN];
    while(tmp)
    {
	sq=FIRSTONE(tmp);

	score-=queensq[BLACK][sq];

	d=distance[sq][brd->king[BLACK]];
	if(d<=3)
	    score-=(4-d)*(4-d)*QUEEN_KING_ATTACK;

	if(ctx->openfiles[sq&0x7]&(WHITEOPEN|BLACKOPEN))
	    score-=QUEEN_OPEN_FILE;

	if(((sq>>3)==6)&&(((brd->king[WHITE]>>3)==7)||(brd->pcs[WHITE][PAWN]&rankbits[6])))
	{
	    BITBOARD rankattacks=rook00attacks[sq][(brd->occ>>s00[sq])&0xFF];
	
	    if(rankattacks&brd->pcs[BLACK][ROOK])
		score-=QUEEN_ROOK_ON_7TH;
	}

	CLEARBIT(tmp,sq);
    }

    return score;
}

int eval_pawn(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int pscore,sscore;
    short sq,file;
    BITBOARD tmp;

    pscore=sscore=0;

    tmp=brd->pcs[WHITE][PAWN];
    while(tmp)
    {
	sq=LASTONE(tmp);
	file=sq&0x7;

	pscore+=pawnsq[WHITE][sq];

	if((brd->pcs[WHITE][PAWN]^setmask[sq])&filebits[file])
	    sscore-=PAWN_DOUBLED;
	
	if((file<7)&&(ISSET(brd->pcs[WHITE][PAWN],sq+1)))
	    sscore+=PAWN_DUO;
	
	if(!(passed[WHITE][sq]&brd->pcs[BLACK][PAWN]))
	    sscore+=((passed_score[WHITE][sq>>3]*ctx->phase)/12);
	
	if(!(isolated[file]&brd->pcs[WHITE][PAWN]))
	{
	    if(ctx->openfiles[file]&BLACKOPEN)
		sscore-=pawn_isolated_weaker[file];
	    else
		sscore-=pawn_isolated[file];
	}
			
	CLEARBIT(tmp,sq);
    }

    tmp=brd->pcs[BLACK][PAWN];
    while(tmp)
    {
	sq=FIRSTONE(tmp);
	file=sq&0x7;

	pscore-=pawnsq[BLACK][sq];

	if((brd->pcs[BLACK][PAWN]^setmask[sq])&filebits[file])
	    sscore+=PAWN_DOUBLED;

	if((file<7)&&(ISSET(brd->pcs[BLACK][PAWN],sq+1)))
	    sscore-=PAWN_DUO;

	if(!(passed[BLACK][sq]&brd->pcs[WHITE][PAWN]))
	    sscore-=((passed_score[BLACK][sq>>3]*ctx->phase)/12);

	if(!(isolated[file]&brd->pcs[BLACK][PAWN]))
	{
	    if(ctx->openfiles[file]&WHITEOPEN)
		sscore+=pawn_isolated_weaker[file];
	    else
		sscore+=pawn_isolated[file];
	}

	CLEARBIT(tmp,sq);
    }

    /* weak pawns              prio:high */
    /* rook attacks weak pawns prio:low  */
    /* stage                   prio:high */
            
    return (pscore+sscore);
}
/*
int eval_king(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    short wk,bk;
    short *rtable[2][64];

    wk=brd->king[WHITE];
    bk=brd->king[BLACK];

    if(OPENING(ctx->phase))
	rtable=
    else if(MIDGAME(ctx->phase))
	rtable=
    else if(LATEMID(ctx->phase))
	rtable=
    else
	rtable

    return score;
}
*/
int eval_lazy(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    BITBOARD tmp;
    
    tmp=brd->pcs[WHITE][BISHOP];

    if((tmp!=0)&&(tmp&maskWBT))
    {
	if((ISSET(tmp,A7))&&ISSET(brd->pcs[BLACK][PAWN],B6))
	    score-=BISHOP_TRAPPED;
	else if((ISSET(tmp,B8))&&ISSET(brd->pcs[BLACK][PAWN],C7))
	    score-=BISHOP_TRAPPED;
	else if((ISSET(tmp,H7))&&ISSET(brd->pcs[BLACK][PAWN],G6))
	    score-=BISHOP_TRAPPED;
	else if((ISSET(tmp,G8))&&ISSET(brd->pcs[BLACK][PAWN],F7))
	    score-=BISHOP_TRAPPED;
    }

    tmp=brd->pcs[BLACK][BISHOP];
    
    if((tmp!=0)&&(tmp&maskBBT))
    {
	if((ISSET(tmp,A2))&&ISSET(brd->pcs[WHITE][PAWN],B3))
	    score+=BISHOP_TRAPPED;
	else if((ISSET(tmp,B1))&&ISSET(brd->pcs[WHITE][PAWN],C2))
	    score+=BISHOP_TRAPPED;
	else if((ISSET(tmp,H2))&&ISSET(brd->pcs[WHITE][PAWN],G3))
	    score+=BISHOP_TRAPPED;
	else if((ISSET(tmp,G1))&&ISSET(brd->pcs[WHITE][PAWN],F2))
	    score+=BISHOP_TRAPPED;    
    }
    
    return score;
}

int eval_development(struct board_t *brd,struct eval_ctx_t *ctx)
{
    int score=0;
    
    BITBOARD blocked;

    blocked=brd->occ;
    blocked&=((brd->pcs[WHITE][PAWN]&rankbits[6])>>8);
    blocked&=(filebits[4]|filebits[5]);

    if(blocked!=0)
    {
	if(blocked&(blocked-1))
	    score-=BLOCKED_CENTER_PAWN*2;
	else
	    score-=BLOCKED_CENTER_PAWN;
    }

    blocked=brd->occ;
    blocked&=((brd->pcs[BLACK][PAWN]&rankbits[1])<<8);
    blocked&=(filebits[4]|filebits[5]);

    if(blocked!=0)
    {
	if(blocked&(blocked-1))
	    score+=BLOCKED_CENTER_PAWN*2;
	else
	    score+=BLOCKED_CENTER_PAWN;
    }

    /* thematic */
    /* early queen */
    /* castling */

    return 0;
}

void init_eval_ctx(struct board_t *brd,struct eval_ctx_t *ctx)
{
    short c;
    BITBOARD tmp;

    tmp=brd->pcs[WHITE][PAWN];
    ctx->pawnctr[WHITE]=((tmp&~filebits[7])>>7);
    ctx->pawnctr[WHITE]|=((tmp&~filebits[0])>>9);

    tmp=brd->pcs[BLACK][PAWN];
    ctx->pawnctr[BLACK]=((tmp&~filebits[0])<<7);
    ctx->pawnctr[BLACK]|=((tmp&~filebits[7])<<9);

    ctx->npawns[WHITE]=BITCOUNT(brd->pcs[WHITE][PAWN]);
    ctx->npawns[BLACK]=BITCOUNT(brd->pcs[BLACK][PAWN]);

    for(c=0;c<=7;c++)
    {
	ctx->openfiles[c]=0;
    
	if(!(filebits[c]&brd->pcs[WHITE][PAWN]))
	    ctx->openfiles[c]|=WHITEOPEN;

	if(!(filebits[c]&brd->pcs[BLACK][PAWN]))
	    ctx->openfiles[c]|=BLACKOPEN;
    }

    ctx->phase=PHASE(brd);
}

int eval_main(struct board_t *brd,int alpha,int beta)
{
    int score=0;

    struct eval_ctx_t ctx;

    init_eval_ctx(brd,&ctx);

    score+=brd->material[WHITE];
    score-=brd->material[BLACK];

    /* draw/won   prio:high */
    /* bad trades prio:high */

    score+=eval_lazy(brd,&ctx);
    score+=eval_pawn(brd,&ctx);

    /* chiamarla solo in apertura e se non abbiamo arroccato */
    if(ctx.phase<=2)
	score+=eval_development(brd,&ctx);
    
    /* crafty like tropism count prio:high */
    /* score+=eval_king_safety(brd,&ctx); */

#define LAZY_EVAL_MARGIN	150

    if((score>(beta+LAZY_EVAL_MARGIN))||(score<(alpha-LAZY_EVAL_MARGIN)))
    {
	return score;
    }

    score+=eval_knight(brd,&ctx);
    score+=eval_bishop(brd,&ctx);
    score+=eval_rook(brd,&ctx);
    score+=eval_queen(brd,&ctx);

    /*
	drawish				prio:medium
	king eval			prio:critic
	king safety			prio:high
	passed pawns evaluation		prio:medium
    */

    return score;
}

int eval(struct board_t *brd,short side,int alpha,int beta)
{
    if(side==WHITE)
	return eval_main(brd,alpha,beta);
    else
	return -eval_main(brd,-beta,-alpha);
    
    /* never reached... only to satisfy gcc */
    return 0;
}

void init_eval_bitboards(void)
{
    short c,d;
    
    for(c=0;c<=63;c++)
    {
	for(d=(c-8);(d>>3)!=0;d-=8)
	{
	    if((c&0x7)!=0)
		SETBIT(passed[WHITE][c],d-1);

	    if((c&0x7)!=7)
		SETBIT(passed[WHITE][c],d+1);
	
	    SETBIT(passed[WHITE][c],d);
	}

	for(d=(c+8);(d>>3)!=7;d+=8)
	{
	    if((c&0x7)!=0)
		SETBIT(passed[BLACK][c],d-1);

	    if((c&0x7)!=7)
		SETBIT(passed[BLACK][c],d+1);
	
	    SETBIT(passed[BLACK][c],d);
	}
    }
    
    isolated[0]=filebits[1];
    isolated[7]=filebits[6];

    for(c=1;c<=6;c++)
	isolated[c]=filebits[c-1]|filebits[c+1];

    for(c=0;c<=63;c++)
    {
	for(d=0;d<=63;d++)
	{
	    short z1=abs((c&0x7)-(d&0x7));
	    short z2=abs((c>>3)-(d>>3));
	
	    distance[c][d]=((z1>z2)?(z1):(z2));
	}
    }

    maskWBT=setmask[A7]|setmask[B8]|setmask[H7]|setmask[G8];
    maskBBT=setmask[A2]|setmask[B1]|setmask[H2]|setmask[G1];
}
