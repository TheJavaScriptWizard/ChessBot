//Development notes and tracing in comments, remove after production or professionalize notes... This is my way of noting and making understand of your codes connections
#include "board.hpp"
#include "zobrist.hpp"

using namespace std;

    FILE_A = 0x0101010101010101;
    FILE_H = 0x8080808080808080;
    ROW_1  = 0x1010101010101010;
    ROW_2  = 0x2020202020202020;
    ROW_8  = 0x0808080808080808;
    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; Position Reference, black lowercase, higher rows are black 7-8

    //Knight Moves: Create pre-calculated bitboard attack masks for knights on all 64 squares.

uint64_t knightAttacks(PiecePosition KNIGHT_POSITION)
{
    uint64_t knightAttacks = 0;
    
    //Prevent wraparounds/illegal moves are edges of the board.
    FILE_AB = FILE_A | (FILE_A << 1);
    FILE_GH = FILE_H | (FILE_H >> 1);

    knightAttacks = (KNIGHT_POS << 17 & ~FILE_A) |
    (KNIGHT_POSITION << 15 & ~FILE_H) |
    (KNIGHT_POSITION << 10 & ~FILE_AB) |
    (KNIGHT_POSITION << 6 & ~FILE_GH) |
    (KNIGHT_POSITION >> 17 & ~FILE_H) |
    (KNIGHT_POSITION >> 15 & ~FILE_A) |
    (KNIGHT_POSITION >> 10 & ~FILE_GH) |
    (KNIGHT_POSITION >> 6 & ~FILE_AB);

    return  knightAttacks
}
    //King Moves: Create pre-calculated bitboard attack masks for the king on all 64 squares.

uint64_t kingAttacks(PiecePosition KING_POSITION)
{
    uint64_t kingAttacks = 0;

    //Prevent wraparounds/illegal moves are edges of the board.

    kingAttacks = 
    (KING_POSITION << 1 & ~FILE_A) |
    (KING_POSITION << 9 & ~FILE_A) |
    (KING_POSITION << 8 & ~ROW_1) |
    (KING_POSITION << 7 & ~FILE_H) |

    (KING_POSITION >> 1 & ~FILE_H) |
    (KING_POSITION >> 9 & ~FILE_H) |
    (KING_POSITION >> 8 & ~ROW_8) |
    (KING_POSITION >> 7 & ~FILE_A);

    return  kingAttacks
}


    //Pawn Moves: Implement single pushes, double pushes (if on starting rank), and diagonal captures using bitwise shifts.

    //Split because attacks are dependat wheras movement isnt - other pieces movement and attacks are them same operation.
uint64_t pawnMovements(PiecePosition PAWN_POSITION, Color color)
{
    uint64_t pawnMovements = 0;

    //Prevent wraparounds/illegal moves are edges of the board.
    if (active_color == Color::Black)
    {
        pawnMovements = 
        (PAWN_POSITION >> 16 & ROW_7) | //ROW7 = BLACK starting Row
        (PAWN_POSITION >> 8 & ~ROW_1);
    }

    if (active_color == Color::White)
    {
        pawnMovements = 
        (PAWN_POSITION << 16 & ROW_2) | //ROW2 = WHITE starting Row
        (PAWN_POSITION << 8 & ~ROW_8);
    }

    return  pawnMovements
}

bool canAttackEnPassant(PiecePosition PAWN_POSITION, int direction)
{
    //game.en_passant updated
    /*
         size_t ep_index = ep_rank * 8 + ep_rank;

        game.en_passant = 1ULL << ep_index;
    */
    if (direction == -1)
    {
        PAWN_POSITION == game.en_passant >> 1;
        return true;
    }
    else if (direction == 1)
    {
        PAWN_POSITION == game.en_passant << 1;
        return true;
    }
    else
    {
        return false;
    }

}

uint64_t pawnAttacks(PiecePosition PAWN_POSITION, Color color)
{
    uint64_t pawnAttacks = 0;

    //Prevent wraparounds/illegal moves are edges of the board.
    
    //Is this function meant to check if theres a enemy pawn or is that checked and then this function entered?
    if (active_color == Color::Black)
    {
        pawnMovements = 
        (PAWN_POSITION >> 7 & ~ROW_1) | //ROW7 = BLACK starting Row
        (PAWN_POSITION >> 9 & ~ROW_1);
    }
    
    else if (active_color == Color::White)
    {
        pawnMovements = 
        (PAWN_POSITION << 7 & ~ROW_8) | //ROW2 = WHITE starting Row
        (PAWN_POSITION << 9 & ~ROW_8);
    }

    if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, 1))
    {
        if (active_color == Color::Black)
        {
            pawnMovements = 
            (PAWN_POSITION >> 9 & ~ROW_1);//ROW7 = BLACK starting Row
        }
        
        if (active_color == Color::White)
        {
            pawnMovements = 
            (PAWN_POSITION << 9 & ~ROW_8); //ROW2 = WHITE starting Row
        }
    }

    else if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, -1))
    {
        if (active_color == Color::Black)
        {
            pawnMovements = 
            (PAWN_POSITION >> 7 & ~ROW_1); //ROW7 = BLACK starting Row
        }
        
        if (active_color == Color::White)
        {
            pawnMovements = 
            (PAWN_POSITION << 7 & ~ROW_8); //ROW2 = WHITE starting Row
        }
    }



    return  pawnAttacks
}

    //Sliding Pieces (Rooks, Bishops, Queens): Implement sliding attack generation.
