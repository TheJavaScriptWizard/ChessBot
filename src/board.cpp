#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace std;

using PiecePosition = uint64_t;

const char COL_MAP[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

string index_to_position(size_t index) {
    size_t column = index % 8;
    size_t row = index / 8 + 1;
    return string(1, COL_MAP[column]) + to_string(row);
}

// Lookup table for MOD67 bit scanning
const size_t MOD67TABLE[67] = {
    64, 0, 1, 39, 2, 15, 40, 23,
    3, 12, 16, 59, 41, 19, 24, 54,
    4, 64, 13, 10, 17, 62, 60, 28,
    42, 30, 20, 51, 25, 44, 55, 47,
    5, 32, 64, 38, 14, 22, 11, 58,
    18, 53, 63, 9, 61, 27, 29, 50,
    43, 46, 31, 37, 21, 57, 52, 8,
    26, 49, 45, 36, 56, 7, 48, 35,
    6, 34, 33
};

size_t bit_scan(uint64_t bit) {
    size_t remainder = bit % 67;
    return MOD67TABLE[remainder];
}

string bit_to_position(PiecePosition bit) {
    if (bit == 0) {
        throw invalid_argument("No piece present!");
    } else {
        size_t onebit_index = bit_scan(bit);
        return index_to_position(onebit_index);
    }
}

enum class Color {
    White,
    Black
};

enum class PieceType {
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King
};

struct Piece {
    PiecePosition position;
    Color color;
    PieceType piece_type;

    string to_string() const {
        string result;
        switch (piece_type) {
            case PieceType::Pawn:   result = "p "; break;
            case PieceType::Rook:   result = "r "; break;
            case PieceType::Knight: result = "n "; break;
            case PieceType::Bishop: result = "b "; break;
            case PieceType::Queen:  result = "q "; break;
            case PieceType::King:   result = "k "; break;
        }

        if (color == Color::White) {
            result[0] = toupper(result[0]);
        }
        
        return result;
    }
};

// Square is either Empty (nullopt) or Occupied (size_t)
using Square = optional<size_t>;

enum class CastlingRights : uint8_t {
    NONE = 0,
    WHITEKINGSIDE  = 1 << 0,
    WHITEQUEENSIDE = 1 << 1,
    BLACKKINGSIDE  = 1 << 2,
    BLACKQUEENSIDE = 1 << 3,
    ALL = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)
};

// Allow bitwise OR on CastlingRights
constexpr CastlingRights operator|(CastlingRights a, CastlingRights b) {
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

// s="ABCDEF", sep='C' -> ("AB", "DEF")
pair<string, string> split_on(const string& s, char sep) {
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == sep) {
            return {s.substr(0, i), s.substr(i + 1)};
        }
    }
    return {s, ""};
}

// Game type to own the data
struct Game {
    vector<Piece> pieces;
    vector<Square> squares;
    Color active_color;
    CastlingRights castling_rights;
    optional<PiecePosition> en_passant;
    size_t halfmove_clock;
    size_t fullmove_number;

    void push_piece_and_square(size_t position, Color color, PieceType piece_type, size_t& index) {
        pieces.push_back({1ULL << position, color, piece_type});
        squares.push_back(index);
        index++;
    }

    void push_empty_square() {
        squares.push_back(nullopt);
    }

    static Game initialize() {
        Game game;
        game.active_color = Color::White;
        game.castling_rights = CastlingRights::ALL;
        game.en_passant = nullopt;
        game.halfmove_clock = 0;
        game.fullmove_number = 1;

        size_t piece_index = 0;
        Color color = Color::White;

        game.push_piece_and_square(0, color, PieceType::Rook, piece_index);
        game.push_piece_and_square(1, color, PieceType::Knight, piece_index);
        game.push_piece_and_square(2, color, PieceType::Bishop, piece_index);
        game.push_piece_and_square(3, color, PieceType::Queen, piece_index);
        game.push_piece_and_square(4, color, PieceType::King, piece_index);
        game.push_piece_and_square(5, color, PieceType::Bishop, piece_index);
        game.push_piece_and_square(6, color, PieceType::Knight, piece_index);
        game.push_piece_and_square(7, color, PieceType::Rook, piece_index);

        for (int i = 8; i < 16; ++i) {
            game.push_piece_and_square(i, color, PieceType::Pawn, piece_index);
        }

        for (int i = 16; i < 48; ++i) {
            game.push_empty_square();
        }

        color = Color::Black;

        for (int i = 48; i < 56; ++i) {
            game.push_piece_and_square(i, color, PieceType::Pawn, piece_index);
        }

        int offset = 56;
        game.push_piece_and_square(offset + 0, color, PieceType::Rook, piece_index);
        game.push_piece_and_square(offset + 1, color, PieceType::Knight, piece_index);
        game.push_piece_and_square(offset + 2, color, PieceType::Bishop, piece_index);
        game.push_piece_and_square(offset + 3, color, PieceType::Queen, piece_index);
        game.push_piece_and_square(offset + 4, color, PieceType::King, piece_index);
        game.push_piece_and_square(offset + 5, color, PieceType::Bishop, piece_index);
        game.push_piece_and_square(offset + 6, color, PieceType::Knight, piece_index);
        game.push_piece_and_square(offset + 7, color, PieceType::Rook, piece_index);

        return game;
    }

    string to_string() const {
        string board = "";
        string temp = "";

        for (size_t i = 0; i < squares.size(); ++i) {
            if (!squares[i].has_value()) {
                temp += index_to_position(i);
            } else {
                temp += pieces[squares[i].value()].to_string();
            }

            if ((i + 1) % 8 == 0) {
                temp += "\n";
                board.insert(0, temp); 
                temp.clear();
            }
        }
        board.insert(0, temp);

        return board;
    }

    static Game read_FEN(const string& fen) {
        Game game;
        game.active_color = Color::White;
        game.castling_rights = CastlingRights::ALL;
        game.en_passant = nullopt;
        game.halfmove_clock = 0;
        game.fullmove_number = 1;

        auto [position, rest] = split_on(fen, ' ');

        deque<Square> deque_squares;
        stringstream ss(position);
        string row;

        while (getline(ss, row, '/')) {
            cout << "row: '" << row << "'\n";
        }

        game.squares = vector<Square>(deque_squares.begin(), deque_squares.end());

        return game;
    }
};

int main() {
    string fen_str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    Game game = Game::read_FEN(fen_str);
    
    return 0;
}
