/*
    includere anche il formato ics, attualmente in xboard.c
    e nel formato ics controllare la promozione

    ics2move
    move2ics
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "quasar.h"
#include "movgen.h"
#include "xboard.h"
#include "make.h"
#include "attacks.h"
#include "search.h"
#include "loadfen.h"

#define PROMOTION	(QUEENPRM|KNIGHTPRM|ROOKPRM|BISHOPPRM)

char promchar(DWORD move)
{
    switch(move>>18)
    {
	case QUEENPRM:
	return 'Q';

	case KNIGHTPRM:
	return 'N';

	case ROOKPRM:
	return 'R';

	case BISHOPPRM:
	return 'B';
    }

    return 'X';
}

DWORD promflags(char x)
{
    switch(toupper(x))
    {
	case 'Q':
	return QUEENPRM;

	case 'N':
	return KNIGHTPRM;
	
	case 'R':
	return ROOKPRM;

	case 'B':
	return BISHOPPRM;
    }

    return 0;
}

char *move2san(struct board_t *brd,DWORD move,char buf[128])
{
    short c,idx,from,to,pcs,cnt,i,row,mcheck,mlm;
    short mlegal[MAXMOVES];
    char pcschar[]={'X','N','B','R','Q','K'};
    DWORD mvs[MAXMOVES];

    from=move&0x3F;
    to=(move>>6)&0x3F;
    pcs=(move>>12)&0x7;
    buf[0]='\0';

    cnt=idx=i=0;
    generate_moves(brd,mvs,&idx);
    for(c=0;c<idx;c++)
    {
	makemove(brd,mvs[c]);
	
	mlegal[c]=FALSE;

	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    mlegal[c]=TRUE;

	    if(mvs[c]==move)
		cnt++;
	}

	unmakemove(brd);
    }

    if(cnt!=1)
	return buf;
    
    makemove(brd,move);
    mcheck=is_in_check(brd,brd->color);
    mlm=legal_moves(brd,NULL);
    unmakemove(brd);

    /*
	primo passaggio:
    
	ci occupiamo subito del "caso particolare" dell'arrocco
    */
    
    if((move>>18)&CASTLING)
    {
	if((to==G1)||(to==G8))
	    snprintf(buf+i,128-i,"O-O");
	else if((to==C1)||(to==C8))
	    snprintf(buf+i,128-i,"O-O-O");

	if(mcheck==TRUE)
	    i+=snprintf(buf+i,128-i,"%c",((mlm==0)?('#'):('+')));

	return buf;
    }
    
    /*
	secondo passaggio:
	
	proviamo a generare una mossa del tipo Nc3 o Bxf6+
	controllando che non ci siano ambiguita'
    */
    
    for(c=cnt=0;c<idx;c++)
    {
	if(((mvs[c]>>12)&0x7)!=pcs)
	    mlegal[c]=FALSE;

	if(((mvs[c]>>6)&0x3F)!=to)
	    mlegal[c]=FALSE;

	if((move>>18)&PROMOTION)
	    if(((mvs[c]>>18)&0xFF)!=((move>>18)&0xFF))
		mlegal[c]=FALSE;

	if(mlegal[c]==TRUE)
	    cnt++;
    }
    
    if(cnt==1)
    {
	if(pcs!=PAWN)
	    i+=snprintf(buf+i,128-i,"%c",pcschar[pcs]);
	
	if((brd->board[to]!=EMPTY)||(brd->ep==to))
	{
	    if(pcs==PAWN)
		i+=snprintf(buf+i,128-i,"%cx",'a'+(from&0x7));
	    else
		i+=snprintf(buf+i,128-i,"x");
	}

	i+=snprintf(buf+i,128-i,"%s",coords[to]);
	
	if((move>>18)&PROMOTION)
	    i+=snprintf(buf+i,128-i,"=%c",promchar(move));

	if(mcheck==TRUE)
	    i+=snprintf(buf+i,128-i,"%c",((mlm==0)?('#'):('+')));

	return buf;
    }

    /*
	terzo passaggio:

	proviamo anche ad indicare la colonna o la riga,
	come ad esempio in Rad1 o N2xc3+
    */

    for(c=cnt=row=0;c<idx;c++)
    {
	if((mlegal[c]==FALSE)||(mvs[c]==move))
	    continue;

	if((mvs[c]&0x7)==(from&0x7))
	    cnt++;

	if(((mvs[c]&0x3F)>>3)==(from>>3))
	    row++;
    }

    if((cnt==0)||(row==0))
    {
	if(pcs!=PAWN)
	    i+=snprintf(buf+i,128-i,"%c",pcschar[pcs]);

	if(cnt==0)
	    i+=snprintf(buf+i,128-i,"%c",'a'+(from&0x7));
	else if(row==0)
	    i+=snprintf(buf+i,128-i,"%c",'1'+7-(from>>3));

	if((brd->board[to]!=EMPTY)||(brd->ep==to))
	    i+=snprintf(buf+i,128-i,"x");

	i+=snprintf(buf+i,128-i,"%s",coords[to]);

	if((move>>18)&PROMOTION)
	    i+=snprintf(buf+i,128-i,"=%c",promchar(move));

	if(mcheck==TRUE)
	    i+=snprintf(buf+i,128-i,"%c",((mlm==0)?('#'):('+')));
	
	return buf;
    }

    /*
	quarto passaggio:

	se la mossa resta ancora ambigua dobbiamo scrivere
	sia la casa di partenza che quella di destinazione    
    */

    i+=snprintf(buf+i,128-i,"%c%s",pcschar[pcs],coords[from]);
    
    if(brd->board[to]!=EMPTY)
	i+=snprintf(buf+i,128-i,"x");

    i+=snprintf(buf+i,128-i,"%s",coords[to]);

    if(mcheck==TRUE)
	i+=snprintf(buf+i,128-i,"%c",((mlm==0)?('#'):('+')));

    return buf;
}

short findcoord(char *str)
{
    short c;

    for(c=0;c<=63;c++)
	if(strncmp(coords[c],str,2)==0)
	    return c;

    return -1;
}

DWORD san2move(struct board_t *brd,char san[128])
{
    short c,d,idx,to,from,pcs,file,row,found,cap;
    short mvalid[MAXMOVES];
    char buf[128];
    char *str;
    DWORD promotion,mvs[MAXMOVES];
    
    to=from=file=row=-1;
    cap=FALSE;
    pcs=PAWN;
    promotion=0;

    for(c=0;c<=127;c++)
	buf[c]=san[c];
    
    buf[127]='\0';
    str=buf;
    
    /*
	primo passaggio:
	
	cerchiamo di individuare subito gli arrocchi
    */

    if(strncmp(buf,"O-O",strlen("O-O"))==0)
    {
	pcs=KING;
	from=((brd->color==WHITE)?(E1):(E8));
	to=((brd->color==WHITE)?(G1):(G8));
	
	goto third;
    }
    
    if(strncmp(buf,"O-O-O",strlen("O-O-O"))==0)
    {
	pcs=KING;
	from=((brd->color==WHITE)?(E1):(E8));
	to=((brd->color==WHITE)?(C1):(C8));
    
	goto third;
    }
    
    /*
	secondo passaggio:
	
	togliamo i caratteri inutili "x+#" e 
	leggiamo la posizione SAN "normale"
    */

    for(c=0;c<strlen(buf);c++)
    {	
	if((buf[c]=='+')||(buf[c]=='#')||(buf[c]=='x'))
	{
	    if(buf[c]=='x')
		cap=TRUE;
	
	    for(d=c;d<strlen(buf);d++)
		buf[d]=buf[d+1];
	}

	if((buf[c]=='=')&&(strlen(buf)==(c+2)))
	{
	    promotion=promflags(buf[c+1]);
	    buf[c]='\0';
	}
    }

    if(strspn(buf,"123456789NBRQKabcdefgh")!=strlen(buf))
	return 0;

    if((c=inarray(*san,"PNBRQK"))!=-1)
    {
	if(c==PAWN)
	    return 0;

	pcs=c;
	*str++;
    }

    if((strlen(str)>=5)||(strlen(str)<=1))
	return 0;
    
    if(strlen(str)==4)
    {
	if((from=findcoord(str))==-1)
	    return 0;

	if((to=findcoord(str+2))==-1)
	    return 0;
    }
    
    if(strlen(str)==3)
    {	
	row=inarray(*str,"12345678");
	file=inarray(*str,"abcdefgh");

	if((to=findcoord(str+1))==-1)
	    return 0;
    }

    if(strlen(str)==2)
    {
	if((to=findcoord(str))==-1)
	    return 0;    
    }
    
    third:
        
    /*
	terzo passaggio:
	
	generiamo tutte le mosse possibili ed escludiamo
	tutte quelle che non combaciano con i dati in
	nostro possesso
    */
    
    idx=0;
    
    if(cap==TRUE)
	generate_captures(brd,mvs,&idx);
    else
	generate_moves(brd,mvs,&idx);
    
    for(c=0;c<idx;c++)
    {
	makemove(brd,mvs[c]);
	
	mvalid[c]=FALSE;
	
	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    mvalid[c]=TRUE;
	
	    if(((mvs[c]>>6)&0x3F)!=to)
		mvalid[c]=FALSE;
	    else if(((mvs[c]>>12)&0x7)!=pcs)
		mvalid[c]=FALSE;
	    else if((from!=-1)&&((mvs[c]&0x3F)!=from))
		mvalid[c]=FALSE;
	    else if((row!=-1)&&(((mvs[c]&0x3F)>>3)!=(7-row)))
		mvalid[c]=FALSE;
	    else if((file!=-1)&&((mvs[c]&0x7)!=file))
		mvalid[c]=FALSE;
	    else if(((mvs[c]>>18)&0xF)!=promotion)
		mvalid[c]=FALSE;
	}

	unmakemove(brd);
    }

    /*
	quarto passaggio:
	
	scorriamo tutta la lista delle mosse per trovare
	la mossa che combacia con i dati letti
	se piu' di una mossa combacia la posizione SAN era ambigua
    */

    for(c=found=0;c<idx;c++)
	if(mvalid[c]==TRUE)
	    found++;

    if(found>1)
	return 0;

    for(c=found=0;c<idx;c++)
	if(mvalid[c]==TRUE)
	    return mvs[c];    

    return 0;
}
