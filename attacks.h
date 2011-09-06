#ifndef __ATTACKS_H__
#define __ATTACKS_H__

#include "quasar.h"

short is_attacked(struct board_t *brd,short k,short side);
short is_in_check(struct board_t *brd,short side);

#endif //__ATTACKS_H__
