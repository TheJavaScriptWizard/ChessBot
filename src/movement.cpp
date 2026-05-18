#include "board.hpp"
#include "zobrist.hpp"

using namespace std;


    //Knight Moves: Create pre-calculated bitboard attack masks for knights on all 64 squares.

uint64_t knightAttacks(PiecePosition KNIGHT_POS)
{
    uint64_t KnightAttacks = 0;
    
    FILE_A = 0x0101010101010101;
    FILE_H = 0x8080808080808080;
    //Prevent wraparounds/illegal moves are edges of the board.
    FILE_AB = FILE_A | (FILE_A << 1);
    FILE_GH = FILE_H | (FILE_H >> 1);

    KnightAttacks = (KNIGHT_POS << 17 & ~FILE_A) |
    (KNIGHT_POS << 15 & ~FILE_H) |
    (KNIGHT_POS << 10 & ~FILE_AB) |
    (KNIGHT_POS << 6 & ~FILE_GH) |
    (KNIGHT_POS >> 17 & ~FILE_H) |
    (KNIGHT_POS >> 15 & ~FILE_A) |
    (KNIGHT_POS >> 10 & ~FILE_GH) |
    (KNIGHT_POS >> 6 & ~FILE_AB);

    return  KnightAttacks
}
    //King Moves: Create pre-calculated bitboard attack masks for the king on all 64 squares.

uint64_t kingAttacks(PiecePosition KING_POS)
{
    uint64_t KnightAttacks = 0;

    FILE_A = 0x0101010101010101;
    FILE_H = 0x8080808080808080;
    ROW_1  = 0x10101010101010101
    ROW_8  = 0x0808080808080808
    //Prevent wraparounds/illegal moves are edges of the board.

    KnightAttacks = 
    (KING_POS << 1 & ~FILE_A) |
    (KING_POS << 9 & ~FILE_A) |
    (KING_POS << 8 & ~ROW_1) |
    (KING_POS << 7 & ~FILE_H) |

    (KING_POS >> 1 & ~FILE_H) |
    (KING_POS >> 9 & ~FILE_H) |
    (KING_POS >> 8 & ~ROW_8) |
    (KING_POS >> 7 & ~FILE_A);

    return  KnightAttacks
}


    //Pawn Moves: Implement single pushes, double pushes (if on starting rank), and diagonal captures using bitwise shifts.





    //Sliding Pieces (Rooks, Bishops, Queens): Implement sliding attack generation.
