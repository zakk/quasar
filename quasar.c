#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "quasar.h"
#include "tables.h"
#include "movgen.h"
#include "make.h"
#include "attacks.h"
#include "eval.h"
#include "search.h"
#include "xboard.h"
#include "hash.h"
#include "see.h"

BITBOARD king_attacks[64];
BITBOARD pawn_attacks[2][64];
BITBOARD knight_attacks[64];
BITBOARD rook00attacks[64][256];
BITBOARD rook90attacks[64][256];
BITBOARD bishop45attacks[64][256];
BITBOARD bishop315attacks[64][256];

BITBOARD setmask[64];
BITBOARD clearmask[64];

BITBOARD setmask45[64];
BITBOARD clearmask45[64];
BITBOARD setmask90[64];
BITBOARD clearmask90[64];
BITBOARD setmask315[64];
BITBOARD clearmask315[64];

BITBOARD mask_white_OO,mask_white_OOO,mask_black_OO,mask_black_OOO;

BITBOARD rankbits[8]=
{
    0x00000000000000FFULL,
    0x000000000000FF00ULL,
    0x0000000000FF0000ULL,
    0x00000000FF000000ULL,
    0x000000FF00000000ULL,
    0x0000FF0000000000ULL,
    0x00FF000000000000ULL,
    0xFF00000000000000ULL
};

BITBOARD filebits[8]=
{
    0x0101010101010101ULL,
    0x0202020202020202ULL,
    0x0404040404040404ULL,
    0x0808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL
};

BYTE bitcount[65536];
BYTE first_ones[65536];
BYTE last_ones[65536];

void init_attacks_bitboards(void)
{
    BYTE tmp[8][256];
    int sq,map;
    
    for(map=0;map<=255;map++)
    {
	short c,d;
	short bits[8];
	
	for(c=0;c<=7;c++)
	{
	    for(d=0;d<=7;d++)
		bits[d]=((map>>d)&1);
	
	    bits[c]=tmp[c][map]=0;
	    
	    if(c<7)
		for(d=(c+1);(d<=7)&&(bits[d-1]!=1);d++)
		    tmp[c][map]|=(1<<d);
	    
	    if(c>0)
		for(d=(c-1);(d>=0)&&(bits[d+1]!=1);d--)
		    tmp[c][map]|=(1<<d);
	}
    }

#define MAX(a,b)	((a>b)?(a):(b))

#define FILEDIST(a,b)	abs(((a)&0x7)-((b)&0x7))
#define RANKDIST(a,b)	abs(((a)>>3)-((b)>>3))
#define DISTANCE(a,b)	MAX(FILEDIST(a,b),RANKDIST(a,b))

#define ONBOARD(x)	((x>=0)&&(x<=63))

    for(sq=0;sq<=63;sq++)
    {
	short a,offsets[8]={-9,-8,-7,-1,+1,+7,+8,+9};
	
	king_attacks[sq]=0;
	
	for(a=0;a<=7;a++)
	    if((DISTANCE(sq,sq+offsets[a])==1)&&(ONBOARD(sq+offsets[a])))
		SETBIT(king_attacks[sq],sq+offsets[a]);
    }

    for(sq=0;sq<=63;sq++)
    {
	pawn_attacks[WHITE][sq]=0;
	pawn_attacks[BLACK][sq]=0;

	if(((sq&0x7)!=0)&&(ONBOARD(sq-9)))
	    SETBIT(pawn_attacks[WHITE][sq],sq-9);

	if(((sq&0x7)!=7)&&(ONBOARD(sq-7)))
	    SETBIT(pawn_attacks[WHITE][sq],sq-7);

	if(((sq&0x7)!=0)&&(ONBOARD(sq+7)))
	    SETBIT(pawn_attacks[BLACK][sq],sq+7);

	if(((sq&0x7)!=7)&&(ONBOARD(sq+9)))
	    SETBIT(pawn_attacks[BLACK][sq],sq+9);
    }

    for(sq=0;sq<=63;sq++)
    {
	short a,offsets[8]={-17,-15,-10,-6,6,10,15,17};
    
	knight_attacks[sq]=0;
    
	for(a=0;a<=7;a++)
	    if((DISTANCE(sq,sq+offsets[a])<=2)&&(ONBOARD(sq+offsets[a])))
		SETBIT(knight_attacks[sq],sq+offsets[a]);
    }
    
    for(sq=0;sq<=63;sq++)
    {
	for(map=0;map<=255;map++)
	{
	    rook00attacks[sq][map]=0;
	    rook00attacks[sq][map]|=(((BITBOARD)(tmp[sq&0x7][map]))<<s00[sq]);
	}
    
	for(map=0;map<=255;map++)
	{
	    short c;
	    
	    rook90attacks[sq][map]=0;
	    
	    for(c=0;c<=7;c++)
		if((tmp[sq>>3][map]>>c)&1)
		    SETBIT(rook90attacks[sq][map],(sq&0x7)+(c<<3));
	}
	
	for(map=0;map<=255;map++)
	{
	    short d,e,base,target;

	    short stop45[64]=
	    {
		1,1,1,1,1,1,1,3,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		3,2,2,2,2,2,2,2,
	    };

	    bishop45attacks[sq][map]=0;
	
	    for(base=sq;!(stop45[base]&1);base-=9)
		/* nothing */;
	    
	    for(target=base;!(stop45[target]&2);target+=9)
		/* nothing */;
	    
	    for(d=base,e=0;d<=target;d+=9,e++)
		if(d==sq)
		    break;    
	    
	    for(d=0;base<=target;base+=9,d++)
		if((tmp[e][map]>>d)&1)
		    SETBIT(bishop45attacks[sq][map],base);
	}

	for(map=0;map<=255;map++)
	{
	    short d,e,base,target;
	
    	    short stop315[64]=
	    {
		3,2,2,2,2,2,2,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
		1,0,0,0,0,0,0,2,
	        1,0,0,0,0,0,0,2,
	        1,0,0,0,0,0,0,2,
		1,1,1,1,1,1,1,3,
	    };

	    bishop315attacks[sq][map]=0;
	
	    for(base=sq;!(stop315[base]&1);base+=7)
		/* nothing */;
	    
	    for(target=base;!(stop315[target]&2);target-=7)
		/* nothing */;
	    
	    for(d=base,e=0;d>=target;d-=7,e++)
		if(d==sq)
		    break;

	    for(d=0;base>=target;base-=7,d++)
		if((tmp[e][map]>>d)&1)
		    SETBIT(bishop315attacks[sq][map],base);	
	}
    }
    
    printf("...attacks bitboards precalculation done...\n");
}

void init_bitcount_tables(void)
{
    int c,d,e;
    
    for(c=0;c<=65535;c++)
    {
	for(d=e=0;d<=15;d++)
	    if((c>>d)&1)
		e++;
	
	bitcount[c]=e;
    }

    printf("...bitcount initialization done...\n");
}

void init_zmasks(void)
{
    int i,j;

    first_ones[0]=last_ones[0]=16;
    
    for(i=1;i<=65535;i++)
    {
	first_ones[i]=16;
	for(j=0;j<=15;j++)
	{
	    if(i&((WORD)(1)<<j))
	    {
		first_ones[i]=j;
		break;
	    }
	}

	last_ones[i]=16;
	for(j=15;j>=0;j--)
	{
	    if(i&((WORD)(1)<<j))
	    {
		last_ones[i]=j;
		break;
	    }
	}
    }

    printf("...leading/trailing bits precalculation done...\n");
}

short init_all(void)
{
    short c;

    if(sizeof(BYTE)!=1)
    {
	printf("%s: internal error, BYTE size is %lu bytes (should be 1)\n",
	       __FILE__,sizeof(BYTE));
	return 1;
    }

    if(sizeof(WORD)!=2)
    {
	printf("%s: internal error, WORD size is %lu bytes (should be 2)\n",
	       __FILE__,sizeof(WORD));
	return 1;
    }

    if(sizeof(DWORD)!=4)
    {
	printf("%s: internal error, WORD size is %lu bytes (should be 4)\n",
	       __FILE__,sizeof(WORD));
	return 1;
    }

    if(sizeof(BITBOARD)!=8)
    {
	printf("%s: internal error, BITBOARD size is %lu bytes (should be 8)\n",
	       __FILE__,sizeof(BITBOARD));
	return 1;
    }

    for(c=0;c<=63;c++)
    {
	setmask[c]=(((BITBOARD)(1))<<c);
	clearmask[c]=~setmask[c];

	setmask45[c]=(((BITBOARD)(1))<<init45[c]);
	clearmask45[c]=~setmask45[c];

	setmask90[c]=(((BITBOARD)(1))<<init90[c]);
	clearmask90[c]=~setmask90[c];

	setmask315[c]=(((BITBOARD)(1))<<init315[c]);
	clearmask315[c]=~setmask315[c];
    }

    mask_white_OO=setmask[F1]|setmask[G1];
    mask_white_OOO=setmask[B1]|setmask[C1]|setmask[D1];
    mask_black_OO=setmask[F8]|setmask[G8];
    mask_black_OOO=setmask[B8]|setmask[C8]|setmask[D8];

    init_attacks_bitboards();
    init_bitcount_tables();
    init_zmasks();
    init_eval_bitboards();
    init_hash();
    init_see();

    printf("...initialization completed...\n\n");
    return 0;
}

void init_board(struct board_t *brd)
{
    short c;

    memset(brd,0,sizeof(struct board_t));

    for(c=A2;c<=H2;c++)
	SETBIT(brd->pcs[WHITE][PAWN],c);
    
    SETBIT(brd->pcs[WHITE][ROOK],A1);
    SETBIT(brd->pcs[WHITE][KNIGHT],B1);
    SETBIT(brd->pcs[WHITE][BISHOP],C1);
    SETBIT(brd->pcs[WHITE][QUEEN],D1);
    SETBIT(brd->pcs[WHITE][KING],E1);
    SETBIT(brd->pcs[WHITE][BISHOP],F1);
    SETBIT(brd->pcs[WHITE][KNIGHT],G1);
    SETBIT(brd->pcs[WHITE][ROOK],H1);

    for(c=A7;c<=H7;c++)
	SETBIT(brd->pcs[BLACK][PAWN],c);

    SETBIT(brd->pcs[BLACK][ROOK],A8);
    SETBIT(brd->pcs[BLACK][KNIGHT],B8);
    SETBIT(brd->pcs[BLACK][BISHOP],C8);
    SETBIT(brd->pcs[BLACK][QUEEN],D8);
    SETBIT(brd->pcs[BLACK][KING],E8);
    SETBIT(brd->pcs[BLACK][BISHOP],F8);
    SETBIT(brd->pcs[BLACK][KNIGHT],G8);
    SETBIT(brd->pcs[BLACK][ROOK],H8);

    brd->king[WHITE]=E1;
    brd->king[BLACK]=E8;

    brd->ply=0;
    brd->color=WHITE;
    brd->ep=-1;
    brd->castling=CASTLEWK|CASTLEWQ|CASTLEBK|CASTLEBQ;

    brd->friends[WHITE]=brd->friends[BLACK]=0x0;
    
    for(c=0;c<=5;c++)
    {
	brd->friends[WHITE]|=brd->pcs[WHITE][c];
	brd->friends[BLACK]|=brd->pcs[BLACK][c];
    }

    brd->occ=brd->friends[WHITE]|brd->friends[BLACK];

    for(c=0;c<=63;c++)
    {
	if(ISSET(brd->occ,c))
	{
	    SETBIT45(brd->occ45,c);
	    SETBIT90(brd->occ90,c);
	    SETBIT315(brd->occ315,c);
	}
    }
    
    for(c=0;c<=63;c++)
	brd->board[c]=EMPTY;
    
    brd->board[A8]=ROOK;
    brd->board[B8]=KNIGHT;
    brd->board[C8]=BISHOP;
    brd->board[D8]=QUEEN;
    brd->board[E8]=KING;
    brd->board[F8]=BISHOP;
    brd->board[G8]=KNIGHT;
    brd->board[H8]=ROOK;

    for(c=0;c<=7;c++)
	brd->board[A7+c]=PAWN;

    brd->board[A1]=ROOK;
    brd->board[B1]=KNIGHT;
    brd->board[C1]=BISHOP;
    brd->board[D1]=QUEEN;
    brd->board[E1]=KING;
    brd->board[F1]=BISHOP;
    brd->board[G1]=KNIGHT;
    brd->board[H1]=ROOK;

    for(c=0;c<=7;c++)
	brd->board[A2+c]=PAWN;

    brd->material[WHITE]=brd->material[BLACK]=
    (PAWNVAL*8)+(KNIGHTVAL*2)+(BISHOPVAL*2)+(ROOKVAL*2)+QUEENVAL;

    brd->hashcode=compute_hashcode(brd);
    brd->fifty=0;
}

void print_board(struct board_t *brd)
{
    short c,d;
    char pcschars[]={'p','n','b','r','q','k',' '};
    
    for(c=0;c<=63;c++)
    {
	char x='.';
    
	for(d=0;d<=5;d++)
	    if(ISSET(brd->pcs[WHITE][d],c))
		x=toupper(pcschars[d]);
	    else if(ISSET(brd->pcs[BLACK][d],c))
		x=pcschars[d];
	
	if((c%8)==7)
	    printf("%c\n",x);
	else
	    printf("%c",x);
    }
}

void dumpbb(BITBOARD x)
{
    short c;
    
    printf("\n");
        
    for(c=0;c<=63;c++)
    {
	printf("%c",(ISSET(x,c)?('1'):('0')));
	
	if((c%8)==7)
	    printf("\n");
    }

    printf("\n");
}

void dump8(BYTE x)
{
    short c;
    
    printf("\n");
        
    for(c=0;c<=7;c++)
	printf("%c",(((x>>c)&1)?('1'):('0')));
    
    printf("\n");
}

short inline FIRSTONE(BITBOARD x)
{
    if(x&65535)
	return first_ones[x&65535];

    if((x>>16)&65535)
	return (first_ones[(x>>16)&65535]+16);

    if((x>>32)&65535)
	return (first_ones[(x>>32)&65535]+32);

    return (first_ones[x>>48]+48);
}

short inline LASTONE(BITBOARD x)
{
    if((x>>48)&65535)
	return (last_ones[x>>48]+48);    

    if((x>>32)&65535)
	return (last_ones[(x>>32)&65535]+32);

    if((x>>16)&65535)
	return (last_ones[(x>>16)&65535]+16);

    return last_ones[x&65535];
}

int nodes;

void perft(struct board_t *brd,short ply)
{
    short c,idx=0;
    DWORD mvs[MAXMOVES];
    
    generate_moves(brd,mvs,&idx);
    for(c=0;c<idx;c++)
    {
	makemove(brd,mvs[c]);

	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    nodes++;

	    if((ply-1)!=0)
		perft(brd,ply-1);
	}

	unmakemove(brd);
    }
}

#include <sys/timeb.h>

inline long getms(void)
{
    struct timeb tb;
    ftime(&tb);
    return ((tb.time*1000)+tb.millitm);
}

void test(struct board_t *brd)
{
    long start,end;
    float lasted;

    nodes=0;

    start=getms();
    perft(brd,5);
    end=getms();

    lasted=((float)(end-start)/(float)(1000));

    printf("\n\nperft(5)=%d\n",nodes);
    printf("search took %f seconds, at %f nodes per second\n\n",
           lasted,((float)(nodes)/(float)(lasted)));
}

int main(void)
{
    struct board_t *brd=malloc(sizeof(struct board_t));
    
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);
    fflush(NULL);
    
    printf("\n\tquasar v0.0.1\n");
    printf("\tby zakk <zakk666@tiscali.it>\n\n");
    
    if(init_all()!=0)
	return 0;
    
    init_board(brd);
    print_board(brd);

    xboard(brd);

    if(brd)
	free(brd);
    
    return 0;
}
