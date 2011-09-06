/*
    special move ordering at root
    quiesce hash ---- quiesce delta cuts
        
    se la ricerca finisce per mancanza di tempo basta
    prendere il valore del ply precedente oppure bisogna tenere conto
    degli alpha/beta cuts alla root
    mancanza di tempo 0.666

    nullmoves+killers problem
*/

#include <stdio.h>
#include "quasar.h"
#include "eval.h"
#include "movgen.h"
#include "make.h"
#include "attacks.h"
#include "search.h"
#include "null.h"
#include "hash.h"
#include "xboard.h"
#include "notation.h"
#include "see.h"
#include "next.h"

int totalnodes;
long maxtime;
short exitnow;
int fhs,fhson1st;

#define ONEPLY		16
#define TWOPLY		32
#define THREEPLY	48
#define FOURPLY		64
#define SIXPLY		96
#define SEVENPLY	112

#define MATE_THREAT_EXT 12
#define CHECK_EXT	ONEPLY
#define MAXEXTS		ONEPLY

#define WINDOW		28
#define MAXQUIESCE	SEVENPLY

/*
    se la profondita' e maggiore di questo valore allora
    le nullmoves vengono fatte con R=3, altrimenti con R=2
*/

#define NULLR3		SIXPLY

#define DRAWSCORE	0
#define CHECKMATE	(-INFINITY+brd->ply)
#define MATE(x)		((x)<=(-INFINITY+MAXPLY))

DWORD killer1[MAXPLY];
DWORD killer2[MAXPLY];
DWORD killer3[MAXPLY];

long int move_history[4096];

void reset_killers(void)
{
    int c;
    
    for(c=0;c<MAXPLY;c++)
    {
	killer1[c]=0;
	killer2[c]=0;
	killer3[c]=0;
    }
}

inline void update_killers(DWORD move,short ply)
{
    if(killer1[ply]==move)
	return;
    
    if(killer2[ply]==move)
    {
	killer2[ply]=killer1[ply];
	killer1[ply]=move;
	return;
    }

    killer3[ply]=killer2[ply];
    killer2[ply]=killer1[ply];
    killer1[ply]=move;
}

void reset_history(void)
{
    int c;
    
    for(c=0;c<=4095;c++)
	move_history[c]=0;
}

inline void update_history(DWORD move)
{
    move_history[move&4095]++;
}

short inline check_draw(struct board_t *brd)
{
    short c,rep=0;
    HASHCODE current=brd->hashcode;
    
    if(brd->fifty>=100)
	return TRUE;

    for(c=2;(c<=brd->fifty)&&((brd->ply-c)>=0);c+=2)
	if(history[brd->ply-c].hashcode==current)
	    if(++rep==2)
		return TRUE;

    return FALSE;
}

int quiesce(struct board_t *brd,short qply,int alpha,int beta)
{
    short c,idx;
    int best,score;
    int mvsval[MAXMOVES];
    DWORD mvs[MAXMOVES];

    totalnodes++;
    if((totalnodes&4095)==0)
    {
	if(getms()>maxtime)
	{
	    exitnow=TRUE;
	    return 0;
	}
    }

    if((qply==0)&&(check_draw(brd)==TRUE))
	return DRAWSCORE;

    best=score=eval(brd,brd->color,alpha,beta);

    if(brd->ply>=(MAXPLY-1))
    {
	printf("%s: search reached maximum ply (%d)!\n",__FILE__,MAXPLY);
	return best;
    }

    if(score>alpha)
	alpha=score;

    if(score>=beta)
	return score;

    if(qply>=MAXQUIESCE)
	return score;

    idx=0;
    generate_captures(brd,mvs,&idx);
    score_qmoves(brd,mvs,mvsval,0,idx);

    for(c=0;c<idx;c++)
    {
	sort_moves(mvs,mvsval,c,idx);
	makemove(brd,mvs[c]);
	
	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    score=-quiesce(brd,qply+ONEPLY,-beta,-alpha);
	    
	    if(exitnow==TRUE)
	    {
		unmakemove(brd);
		return 0;
	    }
	    	    
	    if(score>best)
		best=score;
	    
	    if(score>alpha)
	    {
	        alpha=score;
	    
		if(score>=beta)
		{
	    	    unmakemove(brd);
		    return alpha;
		}
	    }
	}

	unmakemove(brd);
    }

    return best;
}

int alphabeta(struct board_t *brd,short depth,int alpha,int beta,short donull,short incheck)
{
    short legal,first,extensions,doprune;
    DWORD bestmove,hashmove,move;
    int score,delta,best=-INFINITY;
    int oldalpha=alpha;

    struct hash_entry_t *he;
    struct nextctx_t nctx;

    hashmove=bestmove=0;
    score=0;
    legal=0;
    first=TRUE;
    doprune=FALSE;
    extensions=0;
    delta=0;

    if(check_draw(brd)==TRUE)
	return DRAWSCORE;

    totalnodes++;
    if((totalnodes&4095)==0)
    {
	if(getms()>maxtime)
	{
	    exitnow=TRUE;
	    return 0;
	}
    }

    if(brd->ply>=(MAXPLY-1))
    {
	printf("%s: search reached maximum ply (%d)!\n",__FILE__,MAXPLY);
	return eval(brd,brd->color,alpha,beta);
    }
    
    if((he=hash_probe(brd->hashcode))!=NULL)
    {
	if(he->depth>=depth)
	{
	    switch(he->flags)
	    {
		case EXACT:
		return he->score;
		break;

		case LOBOUND:
		if(he->score>=beta)
		    return he->score;
		break;

		case HIBOUND:
		if(he->score<=alpha)
		    return he->score;
		break;
	    }
	}
    
	hashmove=he->move;
    }
    
    if((donull==TRUE)&&(incheck==FALSE))
    {
	short oldep=brd->ep;
	int reduction=((depth>=NULLR3)?(THREEPLY):(TWOPLY));

	makenull(brd);

	if(depth>(ONEPLY+reduction))
	    score=-alphabeta(brd,depth-ONEPLY-reduction,-beta,-beta+1,FALSE,FALSE);
	else
	    score=-quiesce(brd,0,-beta,-beta+1);

	unmakenull(brd,oldep);

	if(score>=beta)
	{
	    alpha=beta;
	    hash_insert(brd->hashcode,alpha,0,depth,LOBOUND);
	    return alpha;
	}
    
	if(MATE(score))
	    extensions+=MATE_THREAT_EXT;
    }

    if(incheck==TRUE)
	extensions+=CHECK_EXT;

    extensions=((extensions>MAXEXTS)?(MAXEXTS):(extensions));

#define RMARGIN		900
#define FMARGIN		300
#define EMARGIN		500
#define MTRL(b)		(b->material[WHITE]-b->material[BLACK])
#define PROMOTE		(QUEENPRM|KNIGHTPRM|ROOKPRM|BISHOPPRM)

    if((extensions==0)&&(incheck==FALSE)&&(depth<FOURPLY)&&(!MATE(alpha))&&(!MATE(-alpha)))
    {
	int matscore=((brd->color==WHITE)?(MTRL(brd)):(-MTRL(brd)));

	delta=alpha-matscore-RMARGIN;

	if(delta>=0)
	    extensions-=ONEPLY;
    
	if((depth+extensions)<TWOPLY)
	{
	    if((delta=alpha-matscore-FMARGIN)>=0)
	    {
		best=matscore+FMARGIN;
		doprune=TRUE;
	    }
	}
	else if((depth+extensions)<THREEPLY)
	{
	    if((delta=alpha-matscore-EMARGIN)>=0)
	    {
		best=matscore+EMARGIN;
		doprune=TRUE;
	    }
	}
    }
    
    nctx.hashmove=hashmove;
    nctx.phase=INIT;

    while((move=getnext(brd,&nctx))!=0)
    {
	makemove(brd,move);

	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    short newdepth=depth+extensions-ONEPLY;
	    short gcheck=is_in_check(brd,brd->color);

	    if((doprune==TRUE)&&(gcheck==FALSE)&&(!((move>>18)&PROMOTE)))
	    {
		if(see(brd,move)<=delta)
		{
		    unmakemove(brd);
		    continue;
		}
	    }

	    if(first==TRUE)
	    {
		if(newdepth<ONEPLY)
	    	    score=-quiesce(brd,0,-beta,-alpha);
		else
	    	    score=-alphabeta(brd,newdepth,-beta,-alpha,TRUE,gcheck);
	    }
	    else
	    {
		if(newdepth<ONEPLY)
	    	    score=-quiesce(brd,0,-alpha-1,-alpha);
		else
	    	    score=-alphabeta(brd,newdepth,-alpha-1,-alpha,TRUE,gcheck);
	    
		if((score>alpha)&&(score<beta))
		{
		    if(newdepth<ONEPLY)
	    		score=-quiesce(brd,0,-beta,-alpha);
		    else
	    		score=-alphabeta(brd,newdepth,-beta,-alpha,TRUE,gcheck);
		}
	    }

	    if(score>best)
	    {
		bestmove=move;
		best=score;

		if(best>alpha)
		{
		    update_history(move);
		    update_killers(move,brd->ply);
		    alpha=best;
		    first=FALSE;
		}

		if(alpha>=beta)
		{
		    unmakemove(brd);
		    hash_insert(brd->hashcode,alpha,bestmove,depth,LOBOUND);
	    
		    fhs++;
		    if(legal==0)
			fhson1st++;
		    
		    return alpha;
		}
	    }

	    if(exitnow==TRUE)
	    {
		unmakemove(brd);
		return 0;
	    }
	    
	    legal++;
	}
	
	unmakemove(brd);
    }    

    if(legal==0)
    {
	score=((is_in_check(brd,brd->color)==TRUE)?(CHECKMATE):(DRAWSCORE));
	hash_insert(brd->hashcode,score,0,MAXPLY,HIBOUND);

	return score;
    }

    if(alpha<=oldalpha)
	hash_insert(brd->hashcode,alpha,bestmove,depth,HIBOUND);
    else
	hash_insert(brd->hashcode,alpha,bestmove,depth,EXACT);

    return alpha;
}

int searchroot(struct board_t *brd,short depth,int alpha,int beta,DWORD *rmove)
{
    short legal,first,extensions;
    int score,best=-INFINITY;
    int oldalpha=alpha;
    DWORD move;

    struct hash_entry_t *he;
    struct nextctx_t nctx;

    score=0;
    legal=0;
    first=TRUE;
    extensions=0;

    if(check_draw(brd)==TRUE)
    {
	*rmove=0;
	return DRAWSCORE;
    }

    totalnodes++;

    if(brd->ply>=(MAXPLY-1))
    {
	printf("%s: search reached maximum ply (%d)!\n",__FILE__,MAXPLY);
	return eval(brd,brd->color,alpha,beta);
    }

    if(is_in_check(brd,brd->color)==TRUE)
	extensions+=CHECK_EXT;

    extensions=((extensions>MAXEXTS)?(MAXEXTS):(extensions));

    if((he=hash_probe(brd->hashcode))!=NULL)
    	nctx.hashmove=he->move;

    nctx.phase=INIT;

    while((move=getnext(brd,&nctx))!=0)
    {
	makemove(brd,move);

	if(is_in_check(brd,brd->color^1)==FALSE)
	{
	    short newdepth=depth+extensions-ONEPLY;
	    short gcheck=is_in_check(brd,brd->color);
	
	    if(first==TRUE)
	    {
		if(newdepth<ONEPLY)
	    	    score=-quiesce(brd,0,-beta,-alpha);
		else
	    	    score=-alphabeta(brd,newdepth,-beta,-alpha,TRUE,gcheck);
	    }
	    else
	    {
		if(newdepth<ONEPLY)
	    	    score=-quiesce(brd,0,-alpha-1,-alpha);
		else
	    	    score=-alphabeta(brd,newdepth,-alpha-1,-alpha,TRUE,gcheck);
	    
		if((score>alpha)&&(score<beta))
		{
		    if(newdepth<ONEPLY)
	    		score=-quiesce(brd,0,-beta,-alpha);
		    else
	    		score=-alphabeta(brd,newdepth,-beta,-alpha,TRUE,gcheck);
		}
	    }

	    if(score>best)
	    {
		*rmove=move;
		best=score;

		if(best>alpha)
		{
		    update_history(move);
		    update_killers(move,brd->ply);
		    alpha=best;
		    first=FALSE;
		}

		if(alpha>=beta)
		{
		    unmakemove(brd);
		    hash_insert(brd->hashcode,alpha,*rmove,depth,LOBOUND);

		    fhs++;
		    if(legal==0)
			fhson1st++;

		    return alpha;
		}
	    }

	    if(exitnow==TRUE)
	    {
		unmakemove(brd);
		return 0;
	    }
	    
	    legal++;
	}

	unmakemove(brd);
    }

    if(legal==0)
    {
	*rmove=0;

	score=((is_in_check(brd,brd->color)==TRUE)?(CHECKMATE):(DRAWSCORE));
	hash_insert(brd->hashcode,score,0,MAXPLY,HIBOUND);

	return score;
    }

    if(alpha<=oldalpha)
	hash_insert(brd->hashcode,alpha,*rmove,depth,HIBOUND);
    else
	hash_insert(brd->hashcode,alpha,*rmove,depth,EXACT);

    return alpha;
}

short inline legal_moves(struct board_t *brd,DWORD *uniq)
{
    short c,idx,legal;
    DWORD mvs[MAXMOVES];

    idx=legal=0;
    generate_moves(brd,mvs,&idx);
    for(c=0;c<idx;c++)
    {
        makemove(brd,mvs[c]);

        if(is_in_check(brd,brd->color^1)==FALSE)
        {
	    if(uniq!=NULL)
		*uniq=mvs[c];

	    legal++;
	}

	unmakemove(brd);
    }

    return legal;
}

DWORD iterate(struct board_t *brd,short depth,long msecs)
{
    int alpha,beta,score,oldscore;
    long start,end;
    float lasted;
    char pv[1024];

    DWORD bestmove,oldbestmove;
    DWORD uniq=0;
    short c,i;

    bestmove=oldbestmove=0;
    
    switch(legal_moves(brd,&uniq))
    {
	case 0:
	return 0;
	break;
    
	case 1:
	return uniq;
	break;
    }

    reset_history();
    reset_killers();
    hash_reset();

    alpha=-INFINITY;
    beta=INFINITY;
    totalnodes=0;
    fhs=fhson1st=0;

    start=getms();
    maxtime=start+msecs;
    
    exitnow=FALSE;

    if(xbmode==FALSE)
	printf("\n\tdepth\tscore\ttime\tPV\n\n");

    for(c=2;(c<=depth)&&(exitnow!=TRUE);c++)
    {	
	oldscore=score;
	oldbestmove=bestmove;

	if((xbmode==TRUE)&&(post==TRUE))
	    printf("%d\t",c);
	else if(xbmode==FALSE)
	    printf("\t<%d>\t",c);

	/*
	    questo algoritmo si chiama aspiration search

	    a ply 2 fa una ricerca con alpha/beta settati a +/- INFINITY,
	    successivamente usa come alpha/beta lo score della ricerca
	    precedente aumentato/diminuito di circa un terzo di pedone (vedi WINDOW)

	    se c'e' un fail-high o un fail-low viene rifatta la ricerca con
	    alpha (in caso di fail-low) o beta (in caso di fail-high) settati
	    a +/- INFINITY

	    nel caso che la ricerca prima desse un fail-high e dopo un fail-low
	    (o viceversa) si fa una ricerca con bounds settati a +/- INFINITY
	    (fortunatamente questo caso, che si chiama search instability, e' molto raro)
	*/
    
	score=searchroot(brd,c*ONEPLY,alpha,beta,&bestmove);

	if((score<=alpha)||(score>=beta))
	{	
	    if(score<=alpha)
	    {
		alpha=-INFINITY;
		beta=score+1;
	    }
	    else if(score>=beta)
	    {
		alpha=score-1;
		beta=INFINITY;
	    }

	    score=searchroot(brd,c*ONEPLY,alpha,beta,&bestmove);

	    if((score<=alpha)||(score>=beta))
		score=searchroot(brd,c*ONEPLY,-INFINITY,INFINITY,&bestmove);
	}

	i=0;
	lasted=(float)(getms()-start);
	getpv(brd,c-1,pv,&i);
	
	if(exitnow)
	    printf("...unfinished...\n");
	else if((xbmode==TRUE)&&(post==TRUE))
	    printf("%d\t%.0f\t%d\t%s\n",score,lasted*10,totalnodes,pv);
	else if(xbmode==FALSE)
	    printf("%.2f\t%.2f\t%s\n",(float)(score)/(float)(100.0),lasted/1000,pv);

	if((bestmove==0)||(MATE(score))||(MATE(-score)))
	    break;

	alpha=score-WINDOW;
	beta=score+WINDOW;
    }
    
    /*
	se la ricerca e' finita per mancanza di tempo i risultati non sono
	ovviamente affidabili e bisogna prendere per buoni quelli del
	ply precedente.
    */
    
    if(exitnow==TRUE)
    {
	bestmove=oldbestmove;
	score=oldscore;
    }

    end=getms();
    lasted=((float)(end-start)/(float)(1000));
    
    printf("\nsearch took %f seconds, searched %d nodes @ %f nodes/sec\n",
           lasted,totalnodes,((float)(totalnodes)/(float)(lasted)));

    printf("ordering efficiency: %d%%\n",((fhs==0)?(100):(fhson1st*100/fhs)));

    return bestmove;
}

short analyze(struct board_t *brd,long msecs,short depth,DWORD best,DWORD avoid)
{
    int alpha,beta,score,oldscore;
    long start,end;
    float lasted;
    short c,i;
    char buf[1024];
    DWORD bestmove,oldbestmove;
    
    reset_history();
    reset_killers();
    hash_reset();

    alpha=-INFINITY;
    beta=INFINITY;
    totalnodes=oldscore=0;
    fhs=fhson1st=0;
    exitnow=FALSE;
    bestmove=oldbestmove=0;

    start=getms();
    maxtime=start+msecs;

    printf("\ndoing analysis @ depth %d, maximum time %lu seconds:\n\n",depth,msecs/1000);
    printf("\tdepth\tscore\ttime\tPV\n\n");

    for(c=2;(c<=depth)&&(exitnow!=TRUE);c++)
    {
	oldscore=score;
	oldbestmove=bestmove;

	printf("\t<%d>\t",c);

	score=searchroot(brd,c*ONEPLY,alpha,beta,&bestmove);

	if((score<=alpha)||(score>=beta))
	{	
	    if(score<=alpha)
	    {
		alpha=-INFINITY;
		beta=score+1;
	    }
	    else if(score>=beta)
	    {
		alpha=score-1;
		beta=INFINITY;
	    }

	    score=searchroot(brd,c*ONEPLY,alpha,beta,&bestmove);

	    if((score<=alpha)||(score>=beta))
		score=searchroot(brd,c*ONEPLY,-INFINITY,INFINITY,&bestmove);
	}

	lasted=((float)(getms()-start)/(float)(1000));
	buf[0]=i=0;

	if(exitnow==TRUE)
	    printf("...unfinished...\n");
	else
	    printf("%.2f\t%.2f\t%s\n",(score)/(100.0),lasted,getpv(brd,c-1,buf,&i));

	alpha=score-WINDOW;
	beta=score+WINDOW;
    }

    if(exitnow==TRUE)
    {
	bestmove=oldbestmove;
	score=oldscore;
    }

    end=getms();
    lasted=((float)(end-start)/(float)(1000));

    printf("\nanalysis took %f seconds, searched %d nodes @ %f nodes/sec\n",
           lasted,totalnodes,((float)(totalnodes)/(float)(lasted)));

    printf("ordering efficiency: %d%%\n",((fhs==0)?(100):(fhson1st*100/fhs)));

    if((best!=0)&&(bestmove==best))
        return TRUE;
	
    if((avoid!=0)&&(bestmove!=avoid))
        return TRUE;

    return FALSE;
}

char *getpv(struct board_t *brd,short maxdepth,char buf[1024],short *i)
{
    struct hash_entry_t *he;
    char tmp[128];
    DWORD move;
    
    if(check_draw(brd)==TRUE)
    {
	*i+=snprintf(buf+(*i),1024-(*i),"1/2-1/2");
	return NULL;
    }

    if(legal_moves(brd,NULL)==0)
    {
	if(is_in_check(brd,brd->color)==TRUE)
	    *i+=snprintf(buf+(*i),1024-(*i),((brd->color==WHITE)?("0-1"):("1-0")));
	else
	    *i+=snprintf(buf+(*i),1024-(*i),"1/2-1/2");	    

	return NULL;
    }

    if((he=hash_probe(brd->hashcode))==NULL)
	return NULL;

    if((move=he->move)==0)
	return NULL;

    *i+=snprintf(buf+(*i),1024-(*i),"%s ",move2san(brd,move,tmp));

    makemove(brd,move);

    if(maxdepth>=1)
	getpv(brd,maxdepth-1,buf,i);

    unmakemove(brd);

    return buf;
}
