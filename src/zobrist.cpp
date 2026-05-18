#include "zobrist.hpp"
#include <random>

namespace Zobrist {
    uint64_t piece_keys[2][6][64];
    uint64_t turn_key;
    uint64_t castling_keys[16];
    uint64_t en_passant_keys[8];

    void init() {
        std::mt19937_64 rng(1337); 
        std::uniform_int_distribution<uint64_t> dist;

        for (int c = 0; c < 2; ++c) {
            for (int p = 0; p < 6; ++p) {
                for (int s = 0; s < 64; ++s) {
                    piece_keys[c][p][s] = dist(rng);
                }
            }
        }

        turn_key = dist(rng);

        for (int i = 0; i < 16; ++i) {
            castling_keys[i] = dist(rng);
        }

        for (int i = 0; i < 8; ++i) {
            en_passant_keys[i] = dist(rng);
        }
    }

    uint64_t toggle_piece(uint64_t current_hash, Color color, PieceType type, size_t square) {
        return current_hash ^ piece_keys[static_cast<int>(color)][static_cast<int>(type)][square];
    }

    uint64_t toggle_turn(uint64_t current_hash) {
        return current_hash ^ turn_key;
    }

    uint64_t toggle_castling(uint64_t current_hash, CastlingRights rights) {
        return current_hash ^ castling_keys[static_cast<uint8_t>(rights)];
    }

    uint64_t toggle_en_passant(uint64_t current_hash, size_t file) {
        return current_hash ^ en_passant_keys[file];
    }

    uint64_t generate_full_hash(const Game& game) {
        uint64_t hash = 0;

        for (size_t i = 0; i < game.squares.size(); ++i) {
            if (game.squares[i].has_value()) {
                const Piece& piece = game.pieces[game.squares[i].value()];
                hash = toggle_piece(hash, piece.color, piece.piece_type, i);
            }
        }

        if (game.active_color == Color::Black) {
            hash = toggle_turn(hash);
        }

        hash = toggle_castling(hash, game.castling_rights);

        if (game.en_passant.has_value()) {
            size_t sq_index = bit_scan(game.en_passant.value());
            size_t file = sq_index % 8; 
            hash = toggle_en_passant(hash, file);
        }

        return hash;
    }
}
