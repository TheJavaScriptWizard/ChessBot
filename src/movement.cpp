//Development notes and tracing in comments, remove after production or professionalize notes... This is my way of noting and making understand of your codes connections

#include "movement.hpp"

using namespace std;

/*
uint64_t FILE_A = 0x0101010101010101;
uint64_t FILE_H = 0x8080808080808080;

uint64_t ROW_1  = 0x1010101010101010;
uint64_t ROW_2  = 0x2020202020202020;

uint64_t ROW_7  = 0x7070707070707070;
uint64_t ROW_8  = 0x8080808080808080;

uint64_t FILE_AB = FILE_A | (FILE_A << 1);
uint64_t FILE_GH = FILE_H | (FILE_H >> 1);
*/
//out of obunds
uint64_t AND_MASK = 0xFFFFFFFFFFFFFFFF; // If its not on the bitboard its not applied when AND'ed

uint64_t FILE_A = 0x0101010101010101;
uint64_t FILE_H = 0x8080808080808080;

uint64_t ROW_1  = 0x00000000000000FF;
uint64_t ROW_2  = 0x000000000000FF00;

uint64_t ROW_7  = 0x00FF000000000000;
uint64_t ROW_8  = 0xFF00000000000000;

uint64_t FILE_AB = FILE_A | (FILE_A << 1);
uint64_t FILE_GH = FILE_H | (FILE_H >> 1);

    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; Position Reference, black lowercase, higher rows are black 7-8

    //Knight Moves: Create pre-calculated bitboard attack masks for knights on all 64 squares.
uint64_t KnightAttacks[64];

uint64_t generateKnightAttacks(PiecePosition KNIGHT_POSITION)
{
    uint64_t knightMoves = 0x0000000000000000;
    
    //Prevent wraparounds/illegal moves are edges of the board.


    knightMoves = 
    ((KNIGHT_POSITION << 17) & ~FILE_A) |
    ((KNIGHT_POSITION << 15) & ~FILE_H) |
    ((KNIGHT_POSITION << 10) & ~FILE_AB) |
    ((KNIGHT_POSITION << 6) & ~FILE_GH) |

    ((KNIGHT_POSITION >> 17) & ~FILE_H) |
    ((KNIGHT_POSITION >> 15) & ~FILE_A) |
    ((KNIGHT_POSITION >> 10) & ~FILE_GH) |
    ((KNIGHT_POSITION >> 6) & ~FILE_AB);

    return  knightMoves;
}


void initKnightAttacks() 
{
    for (int i = 0; i < 64; i++) 
    {
        KnightAttacks[i] = generateKnightAttacks(1ULL << i);
    }
}


//King Moves: Create pre-calculated bitboard attack masks for the king on all 64 squares.

uint64_t generateKingAttacks(PiecePosition KING_POSITION)
{
    uint64_t kingAttacks = 0;

    //Prevent wraparounds/illegal moves are edges of the board.

    kingAttacks = 
    ((KING_POSITION << 1) & ~FILE_A) |
    ((KING_POSITION << 9) & ~FILE_A) |
    ((KING_POSITION << 8) & ~ROW_1) |
    ((KING_POSITION << 7) & ~FILE_H) |

    ((KING_POSITION >> 1) & ~FILE_H) |
    ((KING_POSITION >> 9) & ~FILE_H) |
    ((KING_POSITION >> 8) & ~ROW_8) |
    ((KING_POSITION >> 7) & ~FILE_A);

    return  kingAttacks;
}

uint64_t KingAttacks[64];

void initKingAttacks() 
{
    for (int i = 0; i < 64; i++) 
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
        ((PAWN_POSITION >> 16) & ROW_7) | //ROW7 = BLACK starting Row
        ((PAWN_POSITION >> 8) & ~ROW_1);
    }

    if (game.active_color == Color::White)
    {
        pawnMovements = 
        ((PAWN_POSITION << 16) & ROW_2) | //ROW2 = WHITE starting Row
        ((PAWN_POSITION << 8) & ~ROW_8);
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
    uint64_t pawnAttacks = 0ULL;

    //Prevent wraparounds/illegal moves are edges of the board.

    //Is this function meant to check if theres a enemy pawn or is that checked and then this function entered?
    if (game.active_color == Color::Black)
    {
        pawnAttacks = 
        ((PAWN_POSITION >> 7) & ~FILE_A) | //ROW7 = BLACK starting Row
        ((PAWN_POSITION >> 9) & ~FILE_A);
    }
    
    else if (game.active_color == Color::White)
    {
        pawnAttacks = 
        ((PAWN_POSITION << 7) & ~FILE_H) | //ROW2 = WHITE starting Row
        ((PAWN_POSITION << 9) & ~FILE_H);
    }

    if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, 1))
    {
        if (game.active_color == Color::Black)
        {
            pawnAttacks = 
            ((PAWN_POSITION >> 9) & ~FILE_A);//ROW7 = BLACK starting Row
        }
        
        if (game.active_color == Color::White)
        {
            pawnAttacks = 
            ((PAWN_POSITION << 9) & ~FILE_H); //ROW2 = WHITE starting Row
        }
    }

    else if (game.en_passant != nullopt && canAttackEnPassant(PAWN_POSITION, -1))
    {
        if (game.active_color == Color::Black)
        {
            pawnAttacks = 
            ((PAWN_POSITION >> 7) & ~FILE_A); //ROW7 = BLACK starting Row
        }
        
        if (game.active_color == Color::White)
        {
            pawnAttacks = 
            ((PAWN_POSITION << 7) & ~FILE_H); //ROW2 = WHITE starting Row
        }
    }

    return  pawnAttacks;
}

//Sliding Pieces (Rooks, Bishops, Queens): Implement sliding attack generation.

//AND MASK filters off board moves (vertical)
//Wrap functions checks (horizontal) off board moves (That wrap around)
uint64_t rayTraceAttack (PiecePosition startingSquare, int direction, int angle)// -1 back, 1 forward, angle is shift around piece
{
    uint64_t movements = 0ULL;
    uint64_t currentSquare = startingSquare;
    uint64_t targetSquare = 0ULL;

    if (direction == 1)
    {
        do {
        targetSquare = ((currentSquare << angle) & AND_MASK); 

        if (targetSquare == 0) break; //If its off the board, break loop

        if (((targetSquare & FILE_A) && (currentSquare & FILE_H))) break;
        
        if (((targetSquare & FILE_H) && (currentSquare & FILE_A))) break;


        movements |= targetSquare; //otherwise add move to set
        
        currentSquare = targetSquare; //after all conditions update to newCurrentSquare
        } while ((~game.occupationBoard & targetSquare ) != 0);
    }

    if (direction == -1)
    {
        do {
        targetSquare = ((currentSquare >> angle) & AND_MASK); 

        if (targetSquare == 0) break; //If its off the board, break loop

        if (((targetSquare & FILE_A) && (currentSquare & FILE_H))) break;
        
        if (((targetSquare & FILE_H) && (currentSquare & FILE_A))) break;


        movements |= targetSquare; //otherwise add move to set
        
        currentSquare = targetSquare; //after all conditions update to newCurrentSquare
        } while ((~game.occupationBoard & targetSquare ) != 0);
    }
    return movements;
}

uint64_t bishopSlide(PiecePosition BISHOP_POSITION) 
{
    uint64_t bishopMovements = 0ULL;

        bishopMovements = rayTraceAttack(BISHOP_POSITION, 1, 7) |
        rayTraceAttack(BISHOP_POSITION, 1, 9) |
        rayTraceAttack(BISHOP_POSITION, -1, 7) |
        rayTraceAttack(BISHOP_POSITION, -1, 9);

        return bishopMovements;
}
    
uint64_t rookSlide(PiecePosition ROOK_POSITION)
{
    uint64_t rookMovements = 0ULL;

    rookMovements = rayTraceAttack(ROOK_POSITION, 1, 8) |
    rayTraceAttack(ROOK_POSITION, 1, 1) |
    rayTraceAttack(ROOK_POSITION, -1, 8) |
    rayTraceAttack(ROOK_POSITION, -1, 1);

    return rookMovements;
}

uint64_t queenMovement(PiecePosition QUEEN_POSITION)
{
    uint64_t queenMovement = 0ULL;
    return queenMovement = bishopSlide(QUEEN_POSITION) | rookSlide(QUEEN_POSITION);
} 