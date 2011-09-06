#include <stdio.h>
#include <string.h>

#include "quasar.h"
#include "hash.h"
#include "eval.h"
#include "attacks.h"
#include "loadfen.h"
#include "xboard.h"

short inarray(char x,char *list)
{
    short c;
    
    for(c=0;list[c]!='\0';c++)
	if(list[c]==x)
	    return c;
    
    return -1;
}

void addpcs(short pbrd[64],short cbrd[64],short base,short x)
{
    short p[12]={PAWN,KNIGHT,BISHOP,ROOK,QUEEN,KING,PAWN,KNIGHT,BISHOP,ROOK,QUEEN,KING};
    short c[12]={BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};

    pbrd[base]=p[x];
    cbrd[base]=c[x];
}

short check_pieces(struct board_t *brd)
{
    short nwpawns,nbpawns,nwpcs,nbpcs;
    BITBOARD tmp;

    /* 
	a) il numero dei pedoni non puo' essere maggiore di 8
	
	b) considerando le promozioni la somma dei pezzi e dei pedoni
	   non puo' superare il numero di 15
    
	c) i pedoni non possono stare in ultima o in prima fila
    */
    
    if((nwpawns=BITCOUNT(brd->pcs[WHITE][PAWN]))>8)
	return FALSE;

    if((nbpawns=BITCOUNT(brd->pcs[BLACK][PAWN]))>8)
	return FALSE;

    tmp=brd->pcs[WHITE][KNIGHT]|brd->pcs[WHITE][BISHOP]|
	brd->pcs[WHITE][ROOK]|brd->pcs[WHITE][QUEEN];

    nwpcs=BITCOUNT(tmp);

    tmp=brd->pcs[BLACK][KNIGHT]|brd->pcs[BLACK][BISHOP]|
	brd->pcs[BLACK][ROOK]|brd->pcs[BLACK][QUEEN];

    nbpcs=BITCOUNT(tmp);

    if((nwpcs+nwpawns)>15)
	return FALSE;

    if((nbpcs+nbpawns)>15)
	return FALSE;
    
    if((brd->pcs[WHITE][PAWN]|brd->pcs[BLACK][PAWN])&(rankbits[0]|rankbits[7]))
	return FALSE;

    return TRUE;
}

short setup_board(struct board_t *brd,short pbrd[64],short cbrd[64])
{
    short pcsval[]={PAWNVAL,KNIGHTVAL,BISHOPVAL,ROOKVAL,QUEENVAL,0,0};
    short c;

    memset(brd,0,sizeof(struct board_t));

    brd->king[WHITE]=brd->king[BLACK]=-1;
    brd->ply=0;

    for(c=0;c<=63;c++)
    {
	brd->board[c]=pbrd[c];

	if((cbrd[c]==WHITE)&&(pbrd[c]!=KING))
	    SETBIT(brd->pcs[WHITE][pbrd[c]],c);
	else if((cbrd[c]==BLACK)&&(pbrd[c]!=KING))
	    SETBIT(brd->pcs[BLACK][pbrd[c]],c);
    
	if(pbrd[c]==KING)
	{
	    if(brd->king[cbrd[c]]==-1)
		brd->king[cbrd[c]]=c;
	    else
		return FALSE;
	}
    }
    
    if((brd->king[WHITE]==-1)||(brd->king[BLACK]==-1))
	return FALSE;

    brd->pcs[WHITE][KING]=setmask[brd->king[WHITE]];
    brd->pcs[BLACK][KING]=setmask[brd->king[BLACK]];

    if(check_pieces(brd)!=TRUE)
	return FALSE;
        
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

    brd->material[WHITE]=brd->material[BLACK]=0;

    for(c=0;c<=63;c++)
    {
	if(cbrd[c]==WHITE)
	    brd->material[WHITE]+=pcsval[pbrd[c]];
	else if(cbrd[c]==BLACK)
	    brd->material[BLACK]+=pcsval[pbrd[c]];
    }

    brd->hashcode=compute_hashcode(brd);
    brd->fifty=0;
    
    return TRUE;
}

short setup_flags(struct board_t *brd,char tomove[16],char castle[16],char ep[16])
{
    short c;

    brd->color=((strncmp(tomove,"b",1)==0)?(BLACK):(WHITE));
    brd->castling=0;
    brd->ep=-1;

    for(c=0;c<=63;c++)
	if(strcmp(coords[c],ep)==0)
	    brd->ep=c;

    brd->castling|=((inarray('K',castle)!=-1)?(CASTLEWK):(0));
    brd->castling|=((inarray('Q',castle)!=-1)?(CASTLEWQ):(0));
    brd->castling|=((inarray('k',castle)!=-1)?(CASTLEBK):(0));
    brd->castling|=((inarray('q',castle)!=-1)?(CASTLEBQ):(0));

    return TRUE;
}

short fen2brd(struct board_t *brd,char *string)
{
    char *target;
    char lines[7][256];
    char tomove[16],castle[16],ep[16];
    char fen[256];
    short c,d,x,y;
    short pbrd[64],cbrd[64];
    
    tomove[0]=castle[0]=ep[0]='\0';
    
    if(sscanf(string,"%255s %16s %16s %16s",fen,tomove,castle,ep)<1)
	return FALSE;

    target=fen;

    for(c=0;c<=63;c++)
	pbrd[c]=cbrd[c]=EMPTY;

    for(c=x=y=0;c<=7;c++)
    {
	x=strspn(target,"pnbrqkPNBRQK12345678");
	y=strcspn(target,"/");

	if((x!=y)||((target+x)>=(fen+255))||(x>=255))
	    return FALSE;

	memcpy(lines[c],target,x);
	lines[c][x]='\0';
	target=(char *)(target+x+1);
    }
    
    for(c=0;c<=7;c++)
    {
	short base=c*8;
	short limit=(c+1)*8;

	for(d=0;lines[c][d]!='\0';d++)
	{
	    if((x=inarray(lines[c][d],"pnbrqkPNBRQK"))!=-1)
		addpcs(pbrd,cbrd,base++,x);
	    else if(inarray(lines[c][d],"12345678")!=-1)
		base+=lines[c][d]-'0';
	    else
		return FALSE;

	    if(base>limit)
		return FALSE;
	}

	if(base!=limit)
	    return FALSE;
    }

    if(setup_board(brd,pbrd,cbrd)!=TRUE)
	return FALSE;
    
    if(setup_flags(brd,tomove,castle,ep)!=TRUE)
	return FALSE;

    if(is_in_check(brd,brd->color^1))
	return FALSE;
    
    return TRUE;
}
