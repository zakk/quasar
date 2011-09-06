#ifndef __MOVGEN_H__
#define __MOVGEN_H__

#include "quasar.h"

#define MAXMOVES	256

#define QUEENPRM	1
#define ROOKPRM		2
#define KNIGHTPRM	4
#define BISHOPPRM	8
#define CASTLING	16
#define ENPASSANT	32
#define PAWNTWO		64

void generate_moves(struct board_t *brd,DWORD moves[],short *index);
void generate_captures(struct board_t *brd,DWORD moves[],short *index);
void generate_noncaptures(struct board_t *brd,DWORD moves[],short *index);

#endif //__MOVGEN_H__
