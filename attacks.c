#include "quasar.h"

short is_attacked(struct board_t *brd,short k,short side)
{
    if(side==WHITE)
    {
/*
	if(((brd->pcs[BLACK][PAWN]&(~filebits[0]))<<7)&setmask[k])
	    return TRUE;

	if(((brd->pcs[BLACK][PAWN]&(~filebits[7]))<<9)&setmask[k])
	    return TRUE;
*/
	if(pawn_attacks[WHITE][k]&brd->pcs[BLACK][PAWN])
	    return TRUE;

	if(brd->pcs[BLACK][KNIGHT]&knight_attacks[k])
	    return TRUE;

	if((brd->pcs[BLACK][BISHOP]|brd->pcs[BLACK][QUEEN])&bishop_attacks(brd,k))
	    return TRUE;

	if((brd->pcs[BLACK][ROOK]|brd->pcs[BLACK][QUEEN])&rook_attacks(brd,k))
	    return TRUE;
	
	if(king_attacks[brd->king[BLACK]]&setmask[k])
	    return TRUE;
    }
    else
    {
/*
	if(((brd->pcs[WHITE][PAWN]&(~filebits[7]))>>7)&setmask[k])
	    return TRUE;

	if(((brd->pcs[WHITE][PAWN]&(~filebits[0]))>>9)&setmask[k])
	    return TRUE;
*/
	if(pawn_attacks[BLACK][k]&brd->pcs[WHITE][PAWN])
	    return TRUE;

	if(brd->pcs[WHITE][KNIGHT]&knight_attacks[k])
	    return TRUE;

	if((brd->pcs[WHITE][BISHOP]|brd->pcs[WHITE][QUEEN])&bishop_attacks(brd,k))
	    return TRUE;

	if((brd->pcs[WHITE][ROOK]|brd->pcs[WHITE][QUEEN])&rook_attacks(brd,k))
	    return TRUE;
	
	if(king_attacks[brd->king[WHITE]]&setmask[k])
	    return TRUE;
    }
    
    return FALSE;
}


short is_in_check(struct board_t *brd,short side)
{
    short k;

    if(side==WHITE)
    {
	k=brd->king[WHITE];
/*
	if(((brd->pcs[BLACK][PAWN]&(~filebits[0]))<<7)&setmask[k])
	    return TRUE;

	if(((brd->pcs[BLACK][PAWN]&(~filebits[7]))<<9)&setmask[k])
	    return TRUE;
*/
	if(pawn_attacks[WHITE][k]&brd->pcs[BLACK][PAWN])
	    return TRUE;

	if(brd->pcs[BLACK][KNIGHT]&knight_attacks[k])
	    return TRUE;

	if((brd->pcs[BLACK][BISHOP]|brd->pcs[BLACK][QUEEN])&bishop_attacks(brd,k))
	    return TRUE;

	if((brd->pcs[BLACK][ROOK]|brd->pcs[BLACK][QUEEN])&rook_attacks(brd,k))
	    return TRUE;

	if(king_attacks[k]&setmask[brd->king[BLACK]])
	    return TRUE;
    }
    else
    {
	k=brd->king[BLACK];
/*
	if(((brd->pcs[WHITE][PAWN]&(~filebits[7]))>>7)&setmask[k])
	    return TRUE;

	if(((brd->pcs[WHITE][PAWN]&(~filebits[0]))>>9)&setmask[k])
	    return TRUE;
*/
	if(pawn_attacks[BLACK][k]&brd->pcs[WHITE][PAWN])
	    return TRUE;

	if(brd->pcs[WHITE][KNIGHT]&knight_attacks[k])
	    return TRUE;

	if((brd->pcs[WHITE][BISHOP]|brd->pcs[WHITE][QUEEN])&bishop_attacks(brd,k))
	    return TRUE;

	if((brd->pcs[WHITE][ROOK]|brd->pcs[WHITE][QUEEN])&rook_attacks(brd,k))
	    return TRUE;
	
	if(king_attacks[k]&setmask[brd->king[WHITE]])
	    return TRUE;
    }
    
    return FALSE;
}
