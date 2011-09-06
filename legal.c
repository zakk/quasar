#include <stdio.h>
#include <stdlib.h>
#include "quasar.h"
#include "movgen.h"

short islegal(struct board_t *brd,DWORD move)
{
    short pcs,from,to,side;
    BITBOARD atk=0;

    if(move==0)
	return FALSE;

    pcs=(move>>12)&0x7;
    from=move&0x3F;
    to=(move>>6)&0x3F;
    side=brd->color;

    if(!ISSET(brd->pcs[side][pcs],from))
	return FALSE;

    switch(pcs)
    {
	case PAWN:

#define NOPCS(x)	(brd->board[x]==EMPTY)

	if(abs(to-from)==8)
	{
	    if((side==WHITE)&&(from>to)&&(NOPCS(to)))
		return TRUE;
	    else if((side==BLACK)&&(from<to)&&(NOPCS(to)))
		return TRUE;
	}
	
	if((abs(to-from)==16)&&((move>>18)&PAWNTWO))
	{
	    if((side==WHITE)&&(NOPCS(to+8))&&(NOPCS(to)))
		return TRUE;
	    else if((side==BLACK)&&(NOPCS(to-8))&&(NOPCS(to)))
		return TRUE;
	}
	
	/* en passant */
	
	atk=pawn_attacks[side][from];

	break;

	case KING:
	
	/* castling */

	atk=king_attacks[from];
	break;
    
	case KNIGHT:
	atk=knight_attacks[from];
	break;
	
	case BISHOP:
	atk=bishop_attacks(brd,from);
	break;

	case ROOK:	
	atk=rook_attacks(brd,from);
	break;
    
	case QUEEN:
	atk=rook_attacks(brd,from);
	atk|=bishop_attacks(brd,from);
	break;
    
	default:
	printf("%s: bad pcs in islegal()\n",__FILE__);
	break;
    }

    if(!ISSET(atk,to))
	return FALSE;
    
    if(ISSET(brd->friends[side],to))
	return FALSE;

    return TRUE;
}
