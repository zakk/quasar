#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "make.h"

#define INFINITY	32767

extern DWORD killer1[MAXPLY];
extern DWORD killer2[MAXPLY];
extern DWORD killer3[MAXPLY];

extern long int move_history[4096];

DWORD iterate(struct board_t *brd,short depth,long msecs);
int alphabeta(struct board_t *brd,short depth,int alpha,int beta,short donull,short incheck);
short inline legal_moves(struct board_t *brd,DWORD *uniq);
short inline check_draw(struct board_t *brd);
char *getpv(struct board_t *brd,short maxdepth,char buf[1024],short *i);
short analyze(struct board_t *brd,long msecs,short depth,DWORD best,DWORD avoid);

#endif
