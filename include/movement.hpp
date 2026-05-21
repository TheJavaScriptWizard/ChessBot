#pragma once
#include "board.hpp" 

extern Game game;

extern uint64_t KnightAttacks[64];
extern uint64_t KingAttacks[64];

void initKnightAttacks();
void initKingAttacks();

uint64_t generateKnightAttacks(PiecePosition KNIGHT_POSITION);
uint64_t generateKingAttacks(PiecePosition KING_POSITION);

uint64_t pawnMovements(PiecePosition PAWN_POSITION, Color color);
uint64_t pawnAttacks(PiecePosition PAWN_POSITION, Color color);

uint64_t rayTraceAttack (PiecePosition startingSquare, int direction, int angle)


uint64_t bishopSlide(PiecePosition BISHOP_POSITION);
uint64_t rookSlide(PiecePosition ROOK_POSITION);
uint64_t queenMovement(PiecePosition QUEEN_POSITION);

bool canAttackEnPassant(PiecePosition PAWN_POSITION, int direction);
