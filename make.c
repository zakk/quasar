#include <stdio.h>
#include <string.h>
#include "quasar.h"
#include "movgen.h"
#include "make.h"
#include "eval.h"
#include "hash.h"

/* questo non e' reentrant */
struct board_t history[MAXPLY];

short castle_mask[64]=
{
     7,15,15,15, 3,15,15,11,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    13,15,15,15,12,15,15,14
};

#define PROMOTE	(QUEENPRM|KNIGHTPRM|ROOKPRM|BISHOPPRM)

void makemove(struct board_t *brd,DWORD move)
{
    short from,to,pcs,cap,special;
    BITBOARD aux,aux45,aux90,aux315;
    HASHCODE h=brd->hashcode;

    if((brd->ply+1)>=MAXPLY)
    {
	printf("%s: making a move now exceeds MAXPLY (%d)!\n",__FILE__,MAXPLY);
	return;
    }

    from=(move&0x3F);
    to=((move>>6)&0x3F);
    pcs=((move>>12)&0x7);
    cap=brd->board[to];
    special=((move>>18)&0xFF);

    aux=setmask[from]|setmask[to];
    aux45=setmask45[from]|setmask45[to];
    aux90=setmask90[from]|setmask90[to];
    aux315=setmask315[from]|setmask315[to];

    memcpy(&history[brd->ply],brd,sizeof(struct board_t));
    
    if(brd->ep!=-1)
	h^=HASHEP(brd->ep);
    
    h^=HASHCASTLE(brd->castling);
    
    brd->ep=-1;
    brd->fifty++;

    if(brd->color==WHITE)
    {
	switch(pcs)
	{
	    case PAWN:
	    brd->fifty=0;
	    /* muove il pedone in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->friends[WHITE]^=aux;
	    h^=HASHVALUE(PAWN,from,WHITE);
	    
	    switch(special)
	    {
		case PAWNTWO:
		brd->ep=to+8; /* cambia col nero! */

		case 0: /* nothing special */
		brd->pcs[WHITE][PAWN]^=aux;
		brd->board[from]=EMPTY;
		brd->board[to]=PAWN;
		h^=HASHVALUE(PAWN,to,WHITE);
		break;
	    
		case ENPASSANT:
		{
		    BITBOARD tmp;
		    short target=to+8;

		    tmp=setmask[target];
		    brd->pcs[WHITE][PAWN]^=aux;
		    brd->board[from]=EMPTY;
		    brd->board[to]=PAWN;
		
		    CLEARBIT(brd->occ,target);
		    CLEARBIT45(brd->occ45,target);
		    CLEARBIT90(brd->occ90,target);
		    CLEARBIT315(brd->occ315,target);
		
		    brd->friends[BLACK]^=tmp;
		    brd->pcs[BLACK][PAWN]^=tmp;
		    brd->board[target]=EMPTY; /* cambia col nero */
		    brd->material[BLACK]-=PAWNVAL; /* cambia col nero */
		    h^=HASHVALUE(PAWN,to,WHITE);
		    h^=HASHVALUE(PAWN,target,BLACK);
		}
		break;
	    
		case QUEENPRM:
		brd->pcs[WHITE][PAWN]^=setmask[from];
		brd->pcs[WHITE][QUEEN]|=setmask[to];
		brd->board[to]=QUEEN;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(QUEEN,to,WHITE);
		/* cambia col nero */
		brd->material[WHITE]-=PAWNVAL;
		brd->material[WHITE]+=QUEENVAL;
		break;

		case KNIGHTPRM:
		brd->pcs[WHITE][PAWN]^=setmask[from];
		brd->pcs[WHITE][KNIGHT]|=setmask[to];
		brd->board[to]=KNIGHT;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(KNIGHT,to,WHITE);
		/* cambia col nero */
		brd->material[WHITE]-=PAWNVAL;
		brd->material[WHITE]+=KNIGHTVAL;
		break;

		case ROOKPRM:
		brd->pcs[WHITE][PAWN]^=setmask[from];
		brd->pcs[WHITE][ROOK]|=setmask[to];
		brd->board[to]=ROOK;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(ROOK,to,WHITE);
		/* cambia col nero */
		brd->material[WHITE]-=PAWNVAL;
		brd->material[WHITE]+=ROOKVAL;
		break;

		case BISHOPPRM:
		brd->pcs[WHITE][PAWN]^=setmask[from];
		brd->pcs[WHITE][BISHOP]|=setmask[to];
		brd->board[to]=BISHOP;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(BISHOP,to,WHITE);
		/* cambia col nero */
		brd->material[WHITE]-=PAWNVAL;
		brd->material[WHITE]+=BISHOPVAL;
		break;
	    }

	    break;

	    case KNIGHT:
	    /* muove il cavallo in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[WHITE][KNIGHT]^=aux;
	    brd->friends[WHITE]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=KNIGHT;
	    h^=HASHVALUE(KNIGHT,from,WHITE);
	    h^=HASHVALUE(KNIGHT,to,WHITE);
	    break;

	    case BISHOP:	    
	    /* muove l'alfiere in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[WHITE][BISHOP]^=aux;
	    brd->friends[WHITE]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=BISHOP;
	    h^=HASHVALUE(BISHOP,from,WHITE);
	    h^=HASHVALUE(BISHOP,to,WHITE);
	    break;

	    case ROOK:
	    /* muove la torre in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[WHITE][ROOK]^=aux;
	    brd->friends[WHITE]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=ROOK;
	    h^=HASHVALUE(ROOK,from,WHITE);
	    h^=HASHVALUE(ROOK,to,WHITE);
	    break;

	    case QUEEN:
	    /* muove la donna in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[WHITE][QUEEN]^=aux;
	    brd->friends[WHITE]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=QUEEN;
	    h^=HASHVALUE(QUEEN,from,WHITE);
	    h^=HASHVALUE(QUEEN,to,WHITE);
	    break;
    
	    case KING:
	    /* muove il re in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[WHITE][KING]^=aux;
	    brd->friends[WHITE]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=KING;
	    h^=HASHVALUE(KING,from,WHITE);
	    h^=HASHVALUE(KING,to,WHITE);

	    /* updata il campo con la posizione del re */
	    brd->king[WHITE]=to;
	    
	    /* guarda se questa mossa e' un arrocco */
	    if(special==CASTLING)
	    {
		short rfrom,rto;
		
		if(to==G1) { rfrom=H1; rto=F1; }
		else       { rfrom=A1; rto=D1; }
	    
		/* riposiziona anche la torre! */
		aux=setmask[rfrom]|setmask[rto];
		aux45=setmask45[rfrom]|setmask45[rto];
		aux90=setmask90[rfrom]|setmask90[rto];
		aux315=setmask315[rfrom]|setmask315[rto];
	    	
		brd->occ^=aux;
		brd->occ45^=aux45;
		brd->occ90^=aux90;
		brd->occ315^=aux315;
		brd->friends[WHITE]^=aux;
		brd->pcs[WHITE][ROOK]^=aux;
		brd->board[rfrom]=EMPTY;
		brd->board[rto]=ROOK;

		h^=HASHVALUE(ROOK,rfrom,WHITE);
		h^=HASHVALUE(ROOK,rto,WHITE);

		/* settare un eventuale brd->castled qui */
	    }

	    break;
	}
    
	if(cap==EMPTY)
	    goto done;

	brd->fifty=0;
	aux=setmask[to];
	
	/*
	    il bit e' stato xor-ato prima...
	    adesso bisogna settarlo un'altra volta!! aaaaaargh!
	*/
	
	SETBIT(brd->occ,to);
	SETBIT45(brd->occ45,to);
	SETBIT90(brd->occ90,to);
	SETBIT315(brd->occ315,to);
    
	switch(cap)
	{    
	    case PAWN:
	    brd->friends[BLACK]^=aux;
	    brd->pcs[BLACK][PAWN]^=aux;
	    brd->material[BLACK]-=PAWNVAL;
	    h^=HASHVALUE(PAWN,to,BLACK);
	    break;

	    case KNIGHT:
	    brd->friends[BLACK]^=aux;
	    brd->pcs[BLACK][KNIGHT]^=aux;
	    brd->material[BLACK]-=KNIGHTVAL;
	    h^=HASHVALUE(KNIGHT,to,BLACK);
	    break;

	    case BISHOP:
	    brd->friends[BLACK]^=aux;
	    brd->pcs[BLACK][BISHOP]^=aux;
	    brd->material[BLACK]-=BISHOPVAL;
	    h^=HASHVALUE(BISHOP,to,BLACK);
	    break;

	    case ROOK:
	    brd->friends[BLACK]^=aux;
	    brd->pcs[BLACK][ROOK]^=aux;
	    brd->material[BLACK]-=ROOKVAL;
	    h^=HASHVALUE(ROOK,to,BLACK);
	    break;

	    case QUEEN:
	    brd->friends[BLACK]^=aux;
	    brd->pcs[BLACK][QUEEN]^=aux;
	    brd->material[BLACK]-=QUEENVAL;
	    h^=HASHVALUE(QUEEN,to,BLACK);
	    break;

	    case KING:
	    printf("%s:\tthe black king was captured!\n",__FILE__);
	    printf("\t\tthis is an error condition...\n");
	    printf("piece=%d, from=%d, to %d\n\n",pcs,from,to);
	    break;
	}
    }
    else
    {
	switch(pcs)
	{
	    case PAWN:
	    brd->fifty=0;
	    /* muove il pedone in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->friends[BLACK]^=aux;
	    h^=HASHVALUE(PAWN,from,BLACK);

	    switch(special)
	    {
		case PAWNTWO:
		brd->ep=to-8;

		case 0: /* nothing special */
		brd->pcs[BLACK][PAWN]^=aux;
		brd->board[from]=EMPTY;
		brd->board[to]=PAWN;
		h^=HASHVALUE(PAWN,to,BLACK);
		break;
	    
		case ENPASSANT:
		{
		    BITBOARD tmp;
		    short target=to-8;
		
		    tmp=setmask[target];
		    brd->pcs[BLACK][PAWN]^=aux;
		    brd->board[from]=EMPTY;
		    brd->board[to]=PAWN;
		
		    CLEARBIT(brd->occ,target);
		    CLEARBIT45(brd->occ45,target);
		    CLEARBIT90(brd->occ90,target);
		    CLEARBIT315(brd->occ315,target);
		
		    brd->friends[WHITE]^=tmp;
		    brd->pcs[WHITE][PAWN]^=tmp;
		    brd->board[target]=EMPTY; /* cambia col nero */
		    brd->material[WHITE]-=PAWNVAL; /* cambia col nero */
		    h^=HASHVALUE(PAWN,to,BLACK);
		    h^=HASHVALUE(PAWN,target,WHITE);
		}
		break;
	    
		case QUEENPRM:
		brd->pcs[BLACK][PAWN]^=setmask[from];
		brd->pcs[BLACK][QUEEN]|=setmask[to];
		brd->board[to]=QUEEN;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(QUEEN,to,BLACK);
		/* cambia col nero */
		brd->material[BLACK]-=PAWNVAL;
		brd->material[BLACK]+=QUEENVAL;
		break;

		case KNIGHTPRM:
		brd->pcs[BLACK][PAWN]^=setmask[from];
		brd->pcs[BLACK][KNIGHT]|=setmask[to];
		brd->board[to]=KNIGHT;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(KNIGHT,to,BLACK);
		/* cambia col nero */
		brd->material[BLACK]-=PAWNVAL;
		brd->material[BLACK]+=KNIGHTVAL;
		break;

		case ROOKPRM:
		brd->pcs[BLACK][PAWN]^=setmask[from];
		brd->pcs[BLACK][ROOK]|=setmask[to];
		brd->board[to]=ROOK;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(ROOK,to,BLACK);
		/* cambia col nero */
		brd->material[BLACK]-=PAWNVAL;
		brd->material[BLACK]+=ROOKVAL;
		break;

		case BISHOPPRM:
		brd->pcs[BLACK][PAWN]^=setmask[from];
		brd->pcs[BLACK][BISHOP]|=setmask[to];
		brd->board[to]=BISHOP;
		brd->board[from]=EMPTY;
		h^=HASHVALUE(BISHOP,to,BLACK);
		/* cambia col nero */
		brd->material[BLACK]-=PAWNVAL;
		brd->material[BLACK]+=BISHOPVAL;
		break;
	    }
	    break;

	    case KNIGHT:
	    /* muove il cavallo in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[BLACK][KNIGHT]^=aux;
	    brd->friends[BLACK]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=KNIGHT;
	    h^=HASHVALUE(KNIGHT,from,BLACK);
	    h^=HASHVALUE(KNIGHT,to,BLACK);
	    break;

	    case BISHOP:	    
	    /* muove l'alfiere in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[BLACK][BISHOP]^=aux;
	    brd->friends[BLACK]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=BISHOP;
	    h^=HASHVALUE(BISHOP,from,BLACK);
	    h^=HASHVALUE(BISHOP,to,BLACK);
	    break;

	    case ROOK:
	    /* muove la torre in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[BLACK][ROOK]^=aux;
	    brd->friends[BLACK]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=ROOK;
	    h^=HASHVALUE(ROOK,from,BLACK);
	    h^=HASHVALUE(ROOK,to,BLACK);
	    break;

	    case QUEEN:
	    /* muove la donna in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[BLACK][QUEEN]^=aux;
	    brd->friends[BLACK]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=QUEEN;
	    h^=HASHVALUE(QUEEN,from,BLACK);
	    h^=HASHVALUE(QUEEN,to,BLACK);
	    break;
    
	    case KING:
	    /* muove il re in tutte le bitboards */
	    brd->occ^=aux;
	    brd->occ45^=aux45;
	    brd->occ90^=aux90;
	    brd->occ315^=aux315;
	    brd->pcs[BLACK][KING]^=aux;
	    brd->friends[BLACK]^=aux;
	    brd->board[from]=EMPTY;
	    brd->board[to]=KING;
	    h^=HASHVALUE(KING,from,BLACK);
	    h^=HASHVALUE(KING,to,BLACK);

	    /* updata il campo con la posizione del re */
	    brd->king[BLACK]=to;
	    
	    /* guarda se questa mossa e' un arrocco */
	    if(special==CASTLING)
	    {
		short rfrom,rto;
		
		if(to==G8) { rfrom=H8; rto=F8; }
		else       { rfrom=A8; rto=D8; }
	    
		/* riposiziona anche la torre! */
		aux=setmask[rfrom]|setmask[rto];
		aux45=setmask45[rfrom]|setmask45[rto];
		aux90=setmask90[rfrom]|setmask90[rto];
		aux315=setmask315[rfrom]|setmask315[rto];
	    	
		brd->occ^=aux;
		brd->occ45^=aux45;
		brd->occ90^=aux90;
		brd->occ315^=aux315;
		brd->friends[BLACK]^=aux;
		brd->pcs[BLACK][ROOK]^=aux;
		brd->board[rfrom]=EMPTY;
		brd->board[rto]=ROOK;

		h^=HASHVALUE(ROOK,rfrom,BLACK);
		h^=HASHVALUE(ROOK,rto,BLACK);
	    }

	    break;
	}
    
	if(cap==EMPTY)
	    goto done;

	brd->fifty=0;	
	aux=setmask[to];

	SETBIT(brd->occ,to);
	SETBIT45(brd->occ45,to);
	SETBIT90(brd->occ90,to);
	SETBIT315(brd->occ315,to);
    
	switch(cap)
	{    
	    case PAWN:
	    brd->friends[WHITE]^=aux;
	    brd->pcs[WHITE][PAWN]^=aux;
	    brd->material[WHITE]-=PAWNVAL;
	    h^=HASHVALUE(PAWN,to,WHITE);
	    break;

	    case KNIGHT:
	    brd->friends[WHITE]^=aux;
	    brd->pcs[WHITE][KNIGHT]^=aux;
	    brd->material[WHITE]-=KNIGHTVAL;
	    h^=HASHVALUE(KNIGHT,to,WHITE);
	    break;

	    case BISHOP:
	    brd->friends[WHITE]^=aux;
	    brd->pcs[WHITE][BISHOP]^=aux;
	    brd->material[WHITE]-=BISHOPVAL;
	    h^=HASHVALUE(BISHOP,to,WHITE);
	    break;

	    case ROOK:
	    brd->friends[WHITE]^=aux;
	    brd->pcs[WHITE][ROOK]^=aux;
	    brd->material[WHITE]-=ROOKVAL;
	    h^=HASHVALUE(ROOK,to,WHITE);
	    break;

	    case QUEEN:
	    brd->friends[WHITE]^=aux;
	    brd->pcs[WHITE][QUEEN]^=aux;
	    brd->material[WHITE]-=QUEENVAL;
	    h^=HASHVALUE(QUEEN,to,WHITE);
	    break;

	    case KING:
	    printf("%s:\tthe white king was captured!\n",__FILE__);
	    printf("\t\tthis is an error condition...\n");
	    printf("piece=%d, from=%d, to %d\n\n",pcs,from,to);
	    break;
	}
    }

    done:

    brd->castling&=castle_mask[from];
    brd->castling&=castle_mask[to];

    h^=HASHCASTLE(brd->castling);

    if(brd->ep!=-1)
	h^=HASHEP(brd->ep);
    
    h^=HASHSIDE;
    
    brd->hashcode=h;
    brd->ply++;
    brd->color^=1;
}

void unmakemove(struct board_t *brd)
{
    if(brd->ply<=0)
    {
	printf("%s: requested to unmake a move from the initial position!",__FILE__);
	return;
    }
    
    memcpy(brd,&history[brd->ply-1],sizeof(struct board_t));
}
