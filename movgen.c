#include "quasar.h"
#include "movgen.h"
#include "attacks.h"

DWORD __castle_wk=((E1)|(G1<<6)|(KING<<12)|(CASTLING<<18));
DWORD __castle_wq=((E1)|(C1<<6)|(KING<<12)|(CASTLING<<18));
DWORD __castle_bk=((E8)|(G8<<6)|(KING<<12)|(CASTLING<<18));
DWORD __castle_bq=((E8)|(C8<<6)|(KING<<12)|(CASTLING<<18));

void generate_moves(struct board_t *brd,DWORD moves[],short *index)
{
    BITBOARD tmp,mvs,xfriends,notfriends,occ,notocc;
    DWORD x;
    short from,to,color,ep;
    
    color=brd->color;
    ep=brd->ep;
    xfriends=brd->friends[color^1];
    notfriends=~brd->friends[color];
    occ=brd->occ;
    notocc=~brd->occ;
    mvs=0;

    if(color==WHITE)
    {
	tmp=(brd->pcs[WHITE][PAWN]>>8)&(notocc);
	while(tmp)
	{
	    to=LASTONE(tmp);

	    if(to<=H8)
	    {
		x=((to+8)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else
	    {
		moves[(*index)++]=((to+8)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}	

	tmp=(brd->pcs[WHITE][PAWN]&rankbits[6])>>8;
	tmp=(tmp)&(notocc);
	tmp=(tmp>>8)&(notocc);
	while(tmp)
	{
	    to=LASTONE(tmp);
	    moves[(*index)++]=((to+16)|(to<<6)|(PAWN<<12)|(PAWNTWO<<18));
	    CLEARBIT(tmp,to);
	}

	mvs=xfriends|((ep!=-1)?(setmask[ep]):(0x0));
	tmp=(brd->pcs[WHITE][PAWN])&(~filebits[7]);
	tmp=(tmp>>7)&mvs;
	while(tmp)
	{
	    to=LASTONE(tmp);
	    
	    if(to<=H8)
	    {
		x=((to+7)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to+7)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to+7)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=(brd->pcs[WHITE][PAWN])&(~filebits[0]);
	tmp=(tmp>>9)&mvs;
	while(tmp)
	{
	    to=LASTONE(tmp);

	    if(to<=H8)
	    {
		x=((to+9)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to+9)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to+9)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[WHITE][ROOK];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(ROOK<<12));
	    
	    mvs=rook_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][BISHOP];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][KNIGHT];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&notfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][QUEEN];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[WHITE];
	mvs=king_attacks[from]&notfriends;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=LASTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}

	if(brd->castling&CASTLEWK)
	{
	    if((!(occ&mask_white_OO))&&
	       (is_attacked(brd,E1,WHITE)==FALSE)&&
	       (is_attacked(brd,F1,WHITE)==FALSE))
		moves[(*index)++]=__castle_wk;
	}

	if(brd->castling&CASTLEWQ)
	{
	    if((!(occ&mask_white_OOO))&&
	       (is_attacked(brd,E1,WHITE)==FALSE)&&
	       (is_attacked(brd,D1,WHITE)==FALSE))
		moves[(*index)++]=__castle_wq;
	}
    }
    else
    {
	tmp=(brd->pcs[BLACK][PAWN]<<8)&(notocc);
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-8)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else
	    {
		moves[(*index)++]=((to-8)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}	
        
	tmp=(brd->pcs[BLACK][PAWN]&rankbits[1])<<8;
	tmp=(tmp)&(notocc);
	tmp=(tmp<<8)&(notocc);
	while(tmp)
	{
	    to=FIRSTONE(tmp);
	    moves[(*index)++]=((to-16)|(to<<6)|(PAWN<<12)|(PAWNTWO<<18));
	    CLEARBIT(tmp,to);
	}

	mvs=xfriends|((ep!=-1)?(setmask[ep]):(0x0));
	tmp=(brd->pcs[BLACK][PAWN])&(~filebits[0]);
	tmp=(tmp<<7)&mvs;
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-7)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to-7)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to-7)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=(brd->pcs[BLACK][PAWN])&(~filebits[7]);
	tmp=(tmp<<9)&mvs;
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-9)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to-9)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to-9)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[BLACK][ROOK];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(ROOK<<12));

	    mvs=rook_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}
	
	tmp=brd->pcs[BLACK][BISHOP];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][KNIGHT];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&notfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][QUEEN];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&notfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[BLACK];
	mvs=king_attacks[from]&notfriends;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=FIRSTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}

	if(brd->castling&CASTLEBK)
	{
	    if((!(occ&mask_black_OO))&&
	       (is_attacked(brd,E8,BLACK)==FALSE)&&
	       (is_attacked(brd,F8,BLACK)==FALSE))
		moves[(*index)++]=__castle_bk;
	}

	if(brd->castling&CASTLEBQ)
	{
	    if((!(occ&mask_black_OOO))&&
	       (is_attacked(brd,E8,BLACK)==FALSE)&&
	       (is_attacked(brd,D8,BLACK)==FALSE))
		moves[(*index)++]=__castle_bq;
	}
    }
}

void generate_captures(struct board_t *brd,DWORD moves[],short *index)
{
    BITBOARD tmp,mvs,xfriends,notocc;
    DWORD x;
    short from,to,color,ep;

    color=brd->color;
    ep=brd->ep;
    xfriends=brd->friends[color^1];
    notocc=~brd->occ;
    mvs=0;

    if(color==WHITE)
    {
	tmp=(brd->pcs[WHITE][PAWN]>>8)&(notocc);
	while(tmp)
	{
	    to=LASTONE(tmp);

	    if(to<=H8)
	    {
		x=((to+8)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }

	    CLEARBIT(tmp,to);
	}	

	mvs=xfriends|((ep!=-1)?(setmask[ep]):(0x0));
	tmp=(brd->pcs[WHITE][PAWN])&(~filebits[7]);
	tmp=(tmp>>7)&mvs;
	while(tmp)
	{
	    to=LASTONE(tmp);
	    
	    if(to<=H8)
	    {
		x=((to+7)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to+7)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to+7)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=(brd->pcs[WHITE][PAWN])&(~filebits[0]);
	tmp=(tmp>>9)&mvs;
	while(tmp)
	{
	    to=LASTONE(tmp);

	    if(to<=H8)
	    {
		x=((to+9)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to+9)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to+9)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[WHITE][ROOK];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(ROOK<<12));
	    
	    mvs=rook_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][BISHOP];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][KNIGHT];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&xfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][QUEEN];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[WHITE];
	mvs=king_attacks[from]&xfriends;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=LASTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}
    }
    else
    {
	tmp=(brd->pcs[BLACK][PAWN]<<8)&(notocc);
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-8)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }

	    CLEARBIT(tmp,to);
	}

	mvs=xfriends|((ep!=-1)?(setmask[ep]):(0x0));
	tmp=(brd->pcs[BLACK][PAWN])&(~filebits[0]);
	tmp=(tmp<<7)&mvs;
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-7)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to-7)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to-7)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=(brd->pcs[BLACK][PAWN])&(~filebits[7]);
	tmp=(tmp<<9)&mvs;
	while(tmp)
	{
	    to=FIRSTONE(tmp);

	    if(to>=A1)
	    {
		x=((to-9)|(to<<6)|(PAWN<<12));
		moves[*index]=((x)|(QUEENPRM<<18));
		moves[(*index)+1]=((x)|(ROOKPRM<<18));
		moves[(*index)+2]=((x)|(KNIGHTPRM<<18));
		moves[(*index)+3]=((x)|(BISHOPPRM<<18));
		*index+=4;
	    }
	    else if(ep==to)
	    {
		moves[(*index)++]=((to-9)|(to<<6)|(PAWN<<12)|(ENPASSANT<<18));
	    }
	    else
	    {
		moves[(*index)++]=((to-9)|(to<<6)|(PAWN<<12));
	    }

	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[BLACK][ROOK];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(ROOK<<12));

	    mvs=rook_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}
	
	tmp=brd->pcs[BLACK][BISHOP];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][KNIGHT];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&xfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][QUEEN];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&xfriends;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[BLACK];
	mvs=king_attacks[from]&xfriends;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=FIRSTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}
    }
}

void generate_noncaptures(struct board_t *brd,DWORD moves[],short *index)
{
    BITBOARD tmp,mvs,xfriends,occ,notocc;
    DWORD x;
    short from,to,color,ep;
    
    color=brd->color;
    ep=brd->ep;
    xfriends=brd->friends[color^1];
    occ=brd->occ;
    notocc=~brd->occ;
    mvs=0;

    if(color==WHITE)
    {
	tmp=(brd->pcs[WHITE][PAWN]>>8)&(notocc);
	while(tmp)
	{
	    to=LASTONE(tmp);
	    CLEARBIT(tmp,to);

	    if(to<=H8)
		continue;	
	    
	    moves[(*index)++]=((to+8)|(to<<6)|(PAWN<<12));
	}

	tmp=(brd->pcs[WHITE][PAWN]&rankbits[6])>>8;
	tmp=(tmp)&(notocc);
	tmp=(tmp>>8)&(notocc);
	while(tmp)
	{
	    to=LASTONE(tmp);
	    moves[(*index)++]=((to+16)|(to<<6)|(PAWN<<12)|(PAWNTWO<<18));
	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[WHITE][ROOK];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(ROOK<<12));
	    
	    mvs=rook_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][BISHOP];
        while(tmp)
        {
	    from=LASTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][KNIGHT];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&notocc;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[WHITE][QUEEN];
	while(tmp)
	{
	    from=LASTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=LASTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[WHITE];
	mvs=king_attacks[from]&notocc;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=LASTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}

	if(brd->castling&CASTLEWK)
	{
	    if((!(occ&mask_white_OO))&&
	       (is_attacked(brd,E1,WHITE)==FALSE)&&
	       (is_attacked(brd,F1,WHITE)==FALSE))
		moves[(*index)++]=__castle_wk;
	}

	if(brd->castling&CASTLEWQ)
	{
	    if((!(occ&mask_white_OOO))&&
	       (is_attacked(brd,E1,WHITE)==FALSE)&&
	       (is_attacked(brd,D1,WHITE)==FALSE))
		moves[(*index)++]=__castle_wq;
	}
    }
    else
    {
	tmp=(brd->pcs[BLACK][PAWN]<<8)&(notocc);
	while(tmp)
	{
	    to=FIRSTONE(tmp);
	    CLEARBIT(tmp,to);

	    if(to>=A1)
		continue;

	    moves[(*index)++]=((to-8)|(to<<6)|(PAWN<<12));
	}	
        
	tmp=(brd->pcs[BLACK][PAWN]&rankbits[1])<<8;
	tmp=(tmp)&(notocc);
	tmp=(tmp<<8)&(notocc);
	while(tmp)
	{
	    to=FIRSTONE(tmp);
	    moves[(*index)++]=((to-16)|(to<<6)|(PAWN<<12)|(PAWNTWO<<18));
	    CLEARBIT(tmp,to);
	}

	tmp=brd->pcs[BLACK][ROOK];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(ROOK<<12));

	    mvs=rook_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}
	
	tmp=brd->pcs[BLACK][BISHOP];
        while(tmp)
        {
	    from=FIRSTONE(tmp);
	    x=((from)|(BISHOP<<12));

	    mvs=bishop_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][KNIGHT];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(KNIGHT<<12));

	    mvs=knight_attacks[from]&notocc;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	tmp=brd->pcs[BLACK][QUEEN];
	while(tmp)
	{
	    from=FIRSTONE(tmp);
	    x=((from)|(QUEEN<<12));

	    mvs=queen_attacks(brd,from)&notocc;
	    while(mvs)
	    {
		to=FIRSTONE(mvs);
		moves[(*index)++]=((x)|(to<<6));
		CLEARBIT(mvs,to);
	    }
	
	    CLEARBIT(tmp,from);
	}

	from=brd->king[BLACK];
	mvs=king_attacks[from]&notocc;
	x=((from)|(KING<<12));
	while(mvs)
	{
    	    to=FIRSTONE(mvs);
	    moves[(*index)++]=((x)|(to<<6));
    	    CLEARBIT(mvs,to);
	}

	if(brd->castling&CASTLEBK)
	{
	    if((!(occ&mask_black_OO))&&
	       (is_attacked(brd,E8,BLACK)==FALSE)&&
	       (is_attacked(brd,F8,BLACK)==FALSE))
		moves[(*index)++]=__castle_bk;
	}

	if(brd->castling&CASTLEBQ)
	{
	    if((!(occ&mask_black_OOO))&&
	       (is_attacked(brd,E8,BLACK)==FALSE)&&
	       (is_attacked(brd,D8,BLACK)==FALSE))
		moves[(*index)++]=__castle_bq;
	}
    }
}
