#ifndef __MAKE_H__
#define __MAKE_H__

#include "quasar.h"

#define MAXPLY	512

extern struct board_t history[MAXPLY];

void makemove(struct board_t *brd,DWORD move);
void unmakemove(struct board_t *brd);

#endif //__MAKE_H__
