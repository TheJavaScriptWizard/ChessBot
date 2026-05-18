//Development notes and tracing in comments, remove after production or professionalize notes... This is my way of noting and making understand of your codes connections

#include "movement.hpp"

using namespace std;


uint64_t FILE_A = 0x0101010101010101;

uint64_t FILE_H = 0x8080808080808080;
uint64_t ROW_1  = 0x1010101010101010;
uint64_t ROW_2  = 0x2020202020202020;
uint64_t ROW_7  = 0x7070707070707070;
uint64_t ROW_8  = 0x8080808080808080;

uint64_t FILE_AB = FILE_A | (FILE_A << 1);
uint64_t FILE_GH = FILE_H | (FILE_H >> 1);

    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; Position Reference, black lowercase, higher rows are black 7-8

    //Knight Moves: Create pre-calculated bitboard attack masks for knights on all 64 squares.

uint64_t generateKnightAttacks(PiecePosition KNIGHT_POSITION)
{
    uint64_t knightMoves = 0;
    
    //Prevent wraparounds/illegal moves are edges of the board.


    knightMoves = 
    (KNIGHT_POSITION << 17 & ~FILE_A) |
    (KNIGHT_POSITION << 15 & ~FILE_H) |
    (KNIGHT_POSITION << 10 & ~FILE_AB) |
    (KNIGHT_POSITION << 6 & ~FILE_GH) |

    (KNIGHT_POSITION >> 17 & ~FILE_H) |
    (KNIGHT_POSITION >> 15 & ~FILE_A) |
    (KNIGHT_POSITION >> 10 & ~FILE_GH) |
    (KNIGHT_POSITION >> 6 & ~FILE_AB);

    return  knightMoves;
}

uint64_t KnightAttacks[64];

void initKnightAttacks() 
{
    for (int i = 1; i <= 64; i++) 
    {
        KnightAttacks[i] = generateKnightAttacks(i);
    }
}


//King Moves: Create pre-calculated bitboard attack masks for the king on all 64 squares.

uint64_t generateKingAttacks(PiecePosition KING_POSITION)
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

    return  kingAttacks;
}

uint64_t KingAttacks[64];

void initKingAttacks() 
{
    for (int i = 1; i <= 64; i++) 
    {
        KingAttacks[i] = generateKingAttacks(i);
    }
}

    //Pawn Moves: Implement single pushes, double pushes (if on starting rank), and diagonal captures using bitwise shifts.

    //Split because attacks are dependat wheras movement isnt - other pieces movement and attacks are them same operation.
uint64_t pawnMovements(PiecePosition PAWN_POSITION, Color color)
{
    uint64_t pawnMovements = 0;

    //Prevent wraparounds/illegal moves are edges of the board.
    if (game.active_color == Color::Black)
    {
        pawnMovements = 
        (PAWN_POSITION >> 16 & ROW_7) | //ROW7 = BLACK starting Row
        (PAWN_POSITION >> 8 & ~ROW_1);
    }

    if (game.active_color == Color::White)
    {
        pawnMovements = 
        (PAWN_POSITION << 16 & ROW_2) | //ROW2 = WHITE starting Row
        (PAWN_POSITION << 8 & ~ROW_8);
    }

    return  pawnMovements;
}

bool canAttackEnPassant(PiecePosition PAWN_POSITION, int direction)
{
    //game.en_passant updated
    /*
         size_t ep_index = ep_rank * 8 + ep_rank;

        game.en_passant = 1ULL << ep_index;
    */
    if (PAWN_POSITION == (game.en_passant.value() >> 1))
    {
        return true;
    }
    else if (PAWN_POSITION == (game.en_passant.value() << 1))
    {
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
    if (game.active_color == Color::Black)
    {
        pawnAttacks = 
        (PAWN_POSITION >> 7 & ~ROW_1) | //ROW7 = BLACK starting Row
        (PAWN_POSITION >> 9 & ~ROW_1);
    }
    
    else if (game.active_color == Color::White)
    {
        pawnAttacks = 
        (PAWN_POSITION << 7 & ~ROW_8) | //ROW2 = WHITE starting Row
        (PAWN_POSITION << 9 & ~ROW_8);
    }

    if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, 1))
    {
        if (game.active_color == Color::Black)
        {
            pawnAttacks = 
            (PAWN_POSITION >> 9 & ~ROW_1);//ROW7 = BLACK starting Row
        }
        
        if (game.active_color == Color::White)
        {
            pawnAttacks = 
            (PAWN_POSITION << 9 & ~ROW_8); //ROW2 = WHITE starting Row
        }
    }

    else if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, -1))
    {
        if (game.active_color == Color::Black)
        {
            pawnAttacks = 
            (PAWN_POSITION >> 7 & ~ROW_1); //ROW7 = BLACK starting Row
        }
        
        if (game.active_color == Color::White)
        {
            pawnAttacks = 
            (PAWN_POSITION << 7 & ~ROW_8); //ROW2 = WHITE starting Row
        }
    }

    return  pawnAttacks;
}

//Sliding Pieces (Rooks, Bishops, Queens): Implement sliding attack generation.

uint64_t rayTraceAttack (PiecePosition currentSquare, int direction, int angle)// -1 back, 1 forward, angle is shift around piece
{
    game.getOccupationBoard();
    uint64_t movements;

    if (direction == 1)
    {
        do {

        movements |= currentSquare << angle & ~ROW_1 & ~FILE_A & ~game.occupationBoard;
        currentSquare = currentSquare << angle & ~ROW_1 & ~FILE_A & ~game.occupationBoard;

        } while ((game.occupationBoard & currentSquare) != 0);
    }
    if (direction == 1)
    {
        do {

        movements |= currentSquare >> angle & ~ROW_1 & ~FILE_A & ~game.occupationBoard; //add current position to map
        currentSquare = currentSquare >> angle & ~ROW_1 & ~FILE_A & ~game.occupationBoard; // set reference location to updated square to verify its not blocked... (count first blcoked position as possible attack... hence do while)
        
        } while ((game.occupationBoard & currentSquare) != 0);
    }

    return movements;
}

uint64_t bishopSlide(PiecePosition BISHOP_POSITION) 
{
    uint64_t bishopMovements;

        bishopMovements = rayTraceAttack(BISHOP_POSITION, 1, 7) |
        rayTraceAttack(BISHOP_POSITION, 1, 9) |
        rayTraceAttack(BISHOP_POSITION, -1, 7) |
        rayTraceAttack(BISHOP_POSITION, -1, 9);

        return bishopMovements;
}
    
uint64_t rookSlide(PiecePosition ROOK_POSITION)
{
    uint64_t rookMovements;

    rookMovements = rayTraceAttack(ROOK_POSITION, 1, 8) |
    rayTraceAttack(ROOK_POSITION, 1, 1) |
    rayTraceAttack(ROOK_POSITION, -1, 8) |
    rayTraceAttack(ROOK_POSITION, -1, 1);

    return rookMovements;
}

uint64_t queenMovement(PiecePosition QUEEN_POSITION)
{
    uint64_t queenMovement = 0;
    return queenMovement = bishopSlide(QUEEN_POSITION) | rookSlide(QUEEN_POSITION);
} 