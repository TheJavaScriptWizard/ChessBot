#pragma once
#include <cstdint>
#include "board.hpp" 

namespace Zobrist {
    void init();

    uint64_t toggle_piece(uint64_t current_hash, Color color, PieceType type, size_t square);
    uint64_t toggle_turn(uint64_t current_hash);
    uint64_t toggle_castling(uint64_t current_hash, CastlingRights rights);
    uint64_t toggle_en_passant(uint64_t current_hash, size_t file);

    uint64_t generate_full_hash(const Game& game);
}
