#ifndef __QUASAR_H__
#define __QUASAR_H__

#ifdef __STRICT_ANSI__
#define inline __inline__
#endif

typedef unsigned long long BITBOARD;
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

typedef unsigned long long HASHCODE;

extern BITBOARD king_attacks[64];
extern BITBOARD pawn_attacks[2][64];
extern BITBOARD knight_attacks[64];
extern BITBOARD rook00attacks[64][256];
extern BITBOARD rook90attacks[64][256];
extern BITBOARD bishop45attacks[64][256];
extern BITBOARD bishop315attacks[64][256];

#define rook_attacks(brd,sq)	(rook00attacks[sq][(brd->occ>>s00[sq])&0xFF]|	\
				 rook90attacks[sq][(brd->occ90>>s90[sq])&0xFF])

#define bishop_attacks(brd,sq)	(bishop45attacks[sq][(brd->occ45>>s45[sq])&0xFF]|	\
				 bishop315attacks[sq][(brd->occ315>>s315[sq])&0xFF])

#define queen_attacks(brd,sq)	(rook_attacks(brd,sq)|bishop_attacks(brd,sq))

extern BITBOARD rankbits[8];
extern BITBOARD filebits[8];

extern BITBOARD mask_white_OO,mask_white_OOO,mask_black_OO,mask_black_OOO;

extern BITBOARD setmask[64];
extern BITBOARD clearmask[64];

extern BITBOARD setmask45[64];
extern BITBOARD clearmask45[64];
extern BITBOARD setmask90[64];
extern BITBOARD clearmask90[64];
extern BITBOARD setmask315[64];
extern BITBOARD clearmask315[64];

#define SETBIT(bb,x)	bb|=setmask[x]
#define CLEARBIT(bb,x)	bb&=clearmask[x]
#define ISSET(bb,x)	(bb&setmask[x])

#define SETBIT45(bb,x)		bb|=setmask45[x]
#define CLEARBIT45(bb,x)	bb&=clearmask45[x]
#define ISSET45(bb,x)		(bb&setmask45[x])

#define SETBIT90(bb,x)		bb|=setmask90[x]
#define CLEARBIT90(bb,x)	bb&=clearmask90[x]
#define ISSET90(bb,x)		(bb&setmask90[x])

#define SETBIT315(bb,x)		bb|=setmask315[x]
#define CLEARBIT315(bb,x)	bb&=clearmask315[x]
#define ISSET315(bb,x)		(bb&setmask315[x])

extern BYTE bitcount[65536];

#define BITCOUNT(bb)	(bitcount[(bb)&0xFFFF]+bitcount[((bb)>>16)&0xFFFF]+	\
			bitcount[((bb)>>32)&0xFFFF]+bitcount[((bb)>>48)&0xFFFF])

extern BYTE first_ones[65536];
extern BYTE last_ones[65536];

short inline FIRSTONE(BITBOARD x);
short inline LASTONE(BITBOARD x);

inline long getms(void);

#define TRUE	1
#define FALSE	0

enum SQUARES
{
    A8,B8,C8,D8,E8,F8,G8,H8,
    A7,B7,C7,D7,E7,F7,G7,H7,
    A6,B6,C6,D6,E6,F6,G6,H6,
    A5,B5,C5,D5,E5,F5,G5,H5,
    A4,B4,C4,D4,E4,F4,G4,H4,
    A3,B3,C3,D3,E3,F3,G3,H3,
    A2,B2,C2,D2,E2,F2,G2,H2,
    A1,B1,C1,D1,E1,F1,G1,H1
};

#define WHITE	0
#define BLACK	1

#define PAWN	0
#define KNIGHT	1
#define BISHOP	2
#define ROOK	3
#define QUEEN	4
#define KING	5
#define EMPTY	6

#define CASTLEWK	1
#define CASTLEWQ	2
#define CASTLEBK	4
#define CASTLEBQ	8

struct board_t
{
    BITBOARD pcs[2][6];

    short king[2];
    short ply,color,ep,castling,fifty;
    int material[2];

    BITBOARD friends[2];
    BITBOARD occ,occ45,occ90,occ315;

    BYTE board[64];

    HASHCODE hashcode;
};

void init_board(struct board_t *brd);
void print_board(struct board_t *brd);

extern short init90[64];
extern short init45[64];
extern short init315[64];

extern short s00[64];
extern short s90[64];
extern short s45[64];
extern short s315[64];

#endif //__QUASAR_H__
