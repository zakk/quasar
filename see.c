#include <stdio.h>
#include <stdlib.h>

#include "quasar.h"
#include "movgen.h"
#include "eval.h"
#include "search.h"
#include "see.h"

BITBOARD ray[64][8];
short directions[64][64];

BITBOARD attacks_to_square(struct board_t *brd,short sq)
{
    BITBOARD bq,rq,kings;

    bq=brd->pcs[WHITE][BISHOP]|brd->pcs[WHITE][QUEEN]|
       brd->pcs[BLACK][BISHOP]|brd->pcs[BLACK][QUEEN];

    rq=brd->pcs[WHITE][ROOK]|brd->pcs[WHITE][QUEEN]|
       brd->pcs[BLACK][ROOK]|brd->pcs[BLACK][QUEEN];

    kings=setmask[brd->king[WHITE]]|setmask[brd->king[BLACK]];

    return((pawn_attacks[WHITE][sq]&brd->pcs[BLACK][PAWN])|
           (pawn_attacks[BLACK][sq]&brd->pcs[WHITE][PAWN])|
	   (knight_attacks[sq]&(brd->pcs[WHITE][KNIGHT]|brd->pcs[BLACK][KNIGHT]))|
	   (bishop_attacks(brd,sq)&bq)|
	   (rook_attacks(brd,sq)&rq)|
	   (king_attacks[sq]&kings));
}

BITBOARD xray(struct board_t *brd,short from,short to,short dir)
{
    BITBOARD a;
    short sq;

    if(!(a=ray[from][dir]&brd->occ))
	return 0ULL;

    sq=((to<from)?(FIRSTONE(a)):(LASTONE(a)));

    if(brd->board[sq]==QUEEN)
	return a;

    if((brd->board[sq]==ROOK)&&(dir>=4))
	return a;

    if((brd->board[sq]==BISHOP)&&(dir<=3))
	return a;

    return 0ULL;
}

#define PROMOTE	(QUEENPRM|KNIGHTPRM|ROOKPRM|BISHOPPRM)

int promotion_value(DWORD move)
{
    switch((move>>18)&0xFF)
    {
        case QUEENPRM:
	return QUEENVAL-PAWNVAL;
	    
        case KNIGHTPRM:
        return KNIGHTVAL-PAWNVAL;
	    
        case ROOKPRM:
        return ROOKVAL-PAWNVAL;
	
        case BISHOPPRM:
        return BISHOPVAL-PAWNVAL;
	
        default:
        printf("error in SEE!\n");
	break;
    }

    return 0;
}

int see(struct board_t *brd,DWORD move)
{
    BITBOARD attacks,tmp;
    int p,captured[64];
    int pcsval[7]={PAWNVAL,KNIGHTVAL,BISHOPVAL,ROOKVAL,QUEENVAL,INFINITY,0};
    short from,to,promote,side,ncapt,d,sq;

    from=move&0x3F;
    to=(move>>6)&0x3F;
    promote=(((move>>18)&PROMOTE)?(TRUE):(FALSE));
    side=brd->color;
    ncapt=1;
    
    attacks=attacks_to_square(brd,to);

    p=((((move>>18)&ENPASSANT))?(PAWNVAL):(pcsval[brd->board[to]]));
    captured[0]=p;

    p=pcsval[brd->board[from]];
    CLEARBIT(attacks,from);

    if(promote==TRUE)
	p=promotion_value(move);

    if((d=directions[to][from])!=-1)
	attacks|=xray(brd,from,to,d);

    side^=1;
    
    while(attacks)
    {
	if(side==WHITE)
	{
	    if((tmp=brd->pcs[WHITE][PAWN]&attacks))		sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[WHITE][KNIGHT]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[WHITE][BISHOP]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[WHITE][ROOK]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[WHITE][QUEEN]&attacks))	sq=FIRSTONE(tmp);
	    else if(setmask[brd->king[WHITE]]&attacks)		sq=brd->king[WHITE];
	    else break;
	}
	else
	{
	    if((tmp=brd->pcs[BLACK][PAWN]&attacks))		sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[BLACK][KNIGHT]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[BLACK][BISHOP]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[BLACK][ROOK]&attacks))	sq=FIRSTONE(tmp);
	    else if((tmp=brd->pcs[BLACK][QUEEN]&attacks))	sq=FIRSTONE(tmp);
	    else if(setmask[brd->king[BLACK]]&attacks)		sq=brd->king[BLACK];
	    else break;
	}

	captured[ncapt]=-captured[ncapt-1]+p;
	p=pcsval[brd->board[sq]];

	CLEARBIT(attacks,sq);
	
	if((d=directions[to][sq])!=-1)
	    attacks|=xray(brd,sq,to,d);

	side^=1;
	ncapt++;
    }

    while(--ncapt)
	if(captured[ncapt]>-captured[ncapt-1])
	    captured[ncapt-1]=-captured[ncapt];

    return captured[0];
}

void init_ray(short n,short incr)
{
    short c,d;

#define FILEDIST(a,b)	abs(((a)&0x7)-((b)&0x7))
#define RANKDIST(a,b)	abs(((a)>>3)-((b)>>3))
#define ONBOARD(x)	(((x)>=0)&&((x)<=63))
#define OK(c,d)		(ONBOARD(c)&&ONBOARD(d)&&(RANKDIST(c,d)<=1)&&(FILEDIST(c,d)<=1))

    for(c=0;c<=63;c++)
	ray[c][n]=0ULL;

    for(c=0;c<=63;c++)
	for(d=c+incr;OK(d,d-incr);d+=incr)
	    SETBIT(ray[c][n],d);
}

void init_see(void)
{
    short c,d,e;

    init_ray(0,-9);
    init_ray(1,-7);
    init_ray(2,+7);
    init_ray(3,+9);
    init_ray(4,-8);
    init_ray(5,+1);
    init_ray(6,+8);
    init_ray(7,-1);    

    for(c=0;c<=63;c++)
    {
	for(d=0;d<=63;d++)
	{
	    directions[c][d]=-1;
	
	    for(e=0;e<=7;e++)
		if(ISSET(ray[c][e],d))
		    directions[c][d]=e;
	}
    }
    
    printf("...static exchange evaluator initialization done...\n");
}
