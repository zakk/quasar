#include <stdio.h>
#include <string.h>
#include "quasar.h"
#include "notation.h"
#include "make.h"
#include "loadfen.h"
#include "hash.h"
#include "search.h"

short load_epd(struct board_t *brd,char *file,short time)
{
    FILE *f=fopen(file,"r");
    DWORD best,avoid;
    char line[1024];
    char *tmp;
    short solved,failed,l;

    solved=failed=l=0;

    if(!f)
    {
	printf("\nerror loading EPD (file not found)\n\n");
	return FALSE;
    }

    while(!feof(f))
    {
	if(fgets(line,1024,f)==NULL)
	    break;

	best=avoid=0;	
	tmp=line;
	l++;
	
	if((line[0]==';')||(line[0]=='\n')||(line[0]=='\0'))
	    continue;

	if(fen2brd(brd,line)!=TRUE)
	{
	    printf("\nerror parsing EPD (line %d, invalid FEN layout)\n",l);

	    init_board(brd);
	    hash_set_invalid();

	    printf("EPD test interrupted!\n");
	    printf("solved %d out of %d @ %d seconds/move\n\b",solved,solved+failed,time);
	    return FALSE;
	}

	hash_set_invalid();

	if((tmp=strstr(line,"bm"))!=NULL)
	{
	    char tx[128];
	    
	    tmp+=2;	    
	    while(*tmp==' ')
		*tmp++;

	    strncpy(tx,tmp,128);
	    tx[127]='\0';
	    tx[strcspn(tx," ;\n")]='\0';

	    best=san2move(brd,tx);
	}

	if((tmp=strstr(line,"am"))!=NULL)
	{
	    char tx[128];

	    tmp+=2;
	    while(*tmp==' ')
		*tmp++;
	
	    strncpy(tx,tmp,128);
	    tx[127]='\0';
	    tx[strcspn(tx," ;\n")]='\0';

	    avoid=san2move(brd,tx);
	}

	if((best==0)&&(avoid==0))
	{
	    printf("error parsing EPD (line %d, bm/am fields not found)\n",l);

	    init_board(brd);
	    hash_set_invalid();

	    printf("EPD test interrupted!\n");
	    printf("solved %d out of %d @ %d seconds/move\n",solved,solved+failed,time);
	    return FALSE;
	}

	if(analyze(brd,time*1000,MAXPLY-1,best,avoid)==TRUE)
	{
	    solved++;
	    printf("position %d: OK!\n",l);
	}
	else
	{
	    failed++;
	    printf("position %d: FAILED!\n",l);
	}
    }

    printf("EPD test finished, solved %d out of %d @ %d seconds/move\n\n",solved,solved+failed,time);

    if(f)
	fclose(f);

    return TRUE;
}
