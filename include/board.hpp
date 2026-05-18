#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

using PiecePosition = uint64_t;

size_t bit_scan(uint64_t bit);

enum class Color { White, Black };
enum class PieceType { Pawn, Rook, Knight, Bishop, Queen, King };

struct Piece {
    PiecePosition position;
    Color color;
    PieceType piece_type;
    std::string to_string() const;
};

using Square = std::optional<size_t>;

enum class CastlingRights : uint8_t {
    NONE = 0,
    WHITEKINGSIDE  = 1 << 0,
    WHITEQUEENSIDE = 1 << 1,
    BLACKKINGSIDE  = 1 << 2,
    BLACKQUEENSIDE = 1 << 3,
    ALL = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)
};

constexpr CastlingRights operator|(CastlingRights a, CastlingRights b) {
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

struct Game {
    std::vector<Piece> pieces;
    std::vector<Square> squares;
    Color active_color;
    CastlingRights castling_rights;
    std::optional<PiecePosition> en_passant;
    size_t halfmove_clock;
    size_t fullmove_number;
    
    uint64_t hash; 

    uint64_t occupationBoard = 0x0000000000000000; //Default

    Game(); //past initalizer

    void push_piece_and_square(size_t position, Color color, PieceType piece_type, size_t& index);
    void push_empty_square();
    std::string to_string() const;
    static Game read_FEN(const std::string& fen);
    uint64_t getOccupationBoard();
};
