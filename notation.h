#ifndef __NOTATION_H__
#define __NOTATION_H__

#include "quasar.h"

char *move2san(struct board_t *brd,DWORD move,char buf[128]);
DWORD san2move(struct board_t *brd,char san[128]);

#endif //__NOTATION_H__
