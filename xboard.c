#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include "quasar.h"
#include "search.h"
#include "attacks.h"
#include "make.h"
#include "movgen.h"
#include "eval.h"
#include "hash.h"
#include "loadfen.h"
#include "notation.h"
#include "loadepd.h"

#define PROMPT	"quasar64> "

const char *coords[64]=
{
    "a8","b8","c8","d8","e8","f8","g8","h8",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a1","b1","c1","d1","e1","f1","g1","h1"
};

short xbmode,post;

void brd2fen(struct board_t *brd)
{
    short c,d,empty=0;
    char pcschars[]={'p','n','b','r','q','k',' '};

    for(c=0;c<=63;c++)
    {
	char x=0;

	for(d=0;d<=5;d++)
	    if(ISSET(brd->pcs[WHITE][d],c))
		x=toupper(pcschars[d]);
	    else if(ISSET(brd->pcs[BLACK][d],c))
		x=pcschars[d];
	
	if((empty!=0)&&(x!=0))
	{
	    printf("%d",empty);
	    empty=0;
	}
	
	if(x!=0)
	    printf("%c",x);
	else
	    empty++;
	
	if((c%8)==7)
	{
	    if(empty!=0)
	    {
		printf("%d",empty);
		empty=0;
	    }
	    
	    if(c!=63)
		printf("/");
	}
    }

    printf(" %c ",((brd->color==WHITE)?('w'):('b')));

    if(brd->castling&CASTLEWK)	printf("K");
    if(brd->castling&CASTLEWQ)	printf("Q");
    if(brd->castling&CASTLEBK)	printf("k");
    if(brd->castling&CASTLEBQ)	printf("q");

    if(brd->ep!=-1)
	printf(" %s",coords[brd->ep]);

    printf("\n");
}

void xboard(struct board_t *brd)
{
    short computer=BLACK,quit=FALSE,force=TRUE,depth=8;
    char cmd[1024],line[1024];
    long int time=32767;

    printf("\n");
    xbmode=FALSE;
    post=TRUE;

    while(quit==FALSE)
    {
	if((computer==brd->color)&&(force==FALSE))
	{	
	    DWORD move=iterate(brd,depth,time*1000);

	    if(move==0)
	    {
		force=TRUE;
	    
		if(is_in_check(brd,WHITE)==TRUE)
		    printf("0-1 {White checkmated}\n");
		else if(is_in_check(brd,BLACK)==TRUE)
		    printf("1-0 {Black checkmated}\n");
		else if(check_draw(brd)==TRUE)
		    printf("1/2-1/2 {Threefold repetition or 50 moves rule}\n");
		else
		    printf("1/2-1/2 {Stalemate}\n");
	    }
	    else
	    {
		char buf[128];
	    
		printf("move %s\n",move2san(brd,move,buf));
		makemove(brd,move);
	
		if(legal_moves(brd,NULL)==0)
		{
		    force=TRUE;
	    
		    if(is_in_check(brd,WHITE)==TRUE)
			printf("0-1 {White checkmated}\n");
		    else if(is_in_check(brd,BLACK)==TRUE)
			printf("1-0 {Black checkmated}\n");
		    else
			printf("1/2-1/2 {Stalemate}\n");
		}

		if(check_draw(brd)==TRUE)
		{
		    force=TRUE;
		    printf("1/2-1/2 {Threefold repetition or 50 moves rule}\n");
		}
	    }
	}
    
	if(xbmode!=TRUE)
	    printf("%s",PROMPT);
	
	fgets(line,1024,stdin);
	
	cmd[0]='\0';
	sscanf(line,"%s",cmd);

	if(strcmp(cmd,"xboard")==0)
	{
	    xbmode=TRUE;

	    signal(SIGTERM,SIG_IGN);
	    signal(SIGINT,SIG_IGN);
	
	    printf("\n");
	    printf("feature myname=\"quasar64\" usermove=1 ping=1 san=1 setboard=1 ");
	    printf("done=1 sigint=0 sigterm=0 colors=1 variants=\"normal\"\n");
	}

	else if(strcmp(cmd,"new")==0)
	{
	    init_board(brd);
	    hash_set_invalid();
	    computer=BLACK;
	    force=FALSE;
	}
    
	else if(strcmp(cmd,"quit")==0)
	{
	    quit=TRUE;
	}
    
	else if(strcmp(cmd,"force")==0)
	{
	    force=TRUE;
	}
    
	else if(strcmp(cmd,"go")==0)
	{
	    computer=brd->color;
	    force=FALSE;
	}

	else if(strcmp(cmd,"white")==0)
	{
	    computer=WHITE;
	}

	else if(strcmp(cmd,"black")==0)
	{
	    computer=BLACK;
	}

	else if((strcmp(cmd,"d")==0)||(strcmp(cmd,"show")==0))
	{
	    printf("\n");
	    print_board(brd);
	    printf("\n");
	}
	
	else if((strcmp(cmd,"depth")==0)||(strcmp(cmd,"sd")==0))
	{
	    short sd=-1;

	    sscanf(line,"%s%hd",cmd,&sd);
	    
	    if((sd>=2)&&(sd<=MAXPLY))
	    {
		depth=sd;
		time=32767;
	    }
	    else
	    {
		printf("invalid depth!\n");
	    }
	}

	else if((strcmp(cmd,"movetime")==0)||(strcmp(cmd,"st")==0))
	{
	    int st=-1;

	    sscanf(line,"%s%i",cmd,&st);
	    
	    if((st>=1)&&(st<=32767))
	    {
		depth=MAXPLY;
		time=st;
	    }
	    else
	    {
		printf("invalid movetime!\n");
	    }
	}
	
	else if(strcmp(cmd,"ping")==0)
	{
	    int n=-1;

	    if(sscanf(line,"%s%d",cmd,&n)!=2)
		printf("invalid ping command!\n");
	    else
		printf("pong %d\n",n);
	}

	else if(strcmp(cmd,"phase")==0)
	{
	    printf("phase=%d\n",PHASE(brd));
	}

	else if(strcmp(cmd,"fen")==0)
	{
	    brd2fen(brd);
	}
	
	else if(strcmp(cmd,"setboard")==0)
	{
	    struct board_t *backup=malloc(sizeof(struct board_t));
	    char *arg=strstr(line,"setboard")+strlen("setboard")+1;

	    while(*arg==' ')
		*arg++;

	    memcpy(backup,brd,sizeof(struct board_t));
	
	    if(fen2brd(brd,arg)!=TRUE)
	    {
		printf("couldn't get the new board layout from the FEN position!\n");
		memcpy(brd,backup,sizeof(struct board_t));
	    }
	    else
	    {
		hash_set_invalid();
		computer=brd->color^1;
		force=FALSE;
		
		printf("\nnew board set:\n\n");
		print_board(brd);
		printf("\n");
	    }

	    if(backup)
		free(backup);
	}
		
	else if(strcmp(cmd,"eval")==0)
	{
	    printf("%d\n",eval(brd,WHITE,-INFINITY,INFINITY));
	}
	
	else if(strcmp(cmd,"undo")==0)
	{
	    if(brd->ply>0)
		unmakemove(brd);
	}

	else if(strcmp(cmd,"post")==0)
	{
	    post=TRUE;
	}

	else if(strcmp(cmd,"nopost")==0)
	{
	    post=FALSE;
	}

	else if(strcmp(cmd,"analyze")==0)
	{
	    analyze(brd,time*1000,depth,0,0);
	}
	
	else if(strcmp(cmd,"runepd")==0)
	{
	    char name[128],time[128];
	    short j;
	    int secs;

	    printf("\nEPD file to load: ");
	    fgets(name,128,stdin);
	    printf("time per position (in seconds): ");
	    fgets(time,128,stdin);
	    
	    for(j=0;(j<=127)&&(name[j]!='\0');j++)
		if(name[j]=='\n')
		    name[j]='\0';

	    if((secs=atoi(time))==0)
		printf("\ninvalid time (please insert a number)\n\n");
	    else
		load_epd(brd,name,secs);
	}

	else if(strcmp(cmd,"usermove")==0)
	{
	    DWORD move;
	    char *str=strstr(line,"usermove")+strlen("usermove")+1;

	    while(*str==' ')
		*str++;
	    
	    str[strcspn(str," \n")]='\0';

	    if((move=san2move(brd,str))!=0)
		makemove(brd,move);
	    else
		printf("invalid move!\n");

	    if(legal_moves(brd,NULL)==0)
	    {
		force=TRUE;
	    
		if(is_in_check(brd,WHITE)==TRUE)
		    printf("0-1 {White checkmated}\n");
		else if(is_in_check(brd,BLACK)==TRUE)
		    printf("1-0 {Black checkmated}\n");
		else
		    printf("1/2-1/2 {Stalemate}\n");
	    }
	
	    if(check_draw(brd)==TRUE)
	    {
		force=TRUE;
		printf("1/2-1/2 {Threefold repetition or 50 moves rule}\n");
	    }
	}
	
	else
	{
	    printf("unknow command: %s\n",cmd);
	}
    }

    /* level */
    /* time commands */
}
