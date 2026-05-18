#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include "board.hpp"
#include "zobrist.hpp"

using namespace std;

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

// s="ABCDEF", sep='C' -> ("AB", "DEF")
pair<string, string> split_on(const string& s, char sep) {
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == sep) {
            return {s.substr(0, i), s.substr(i + 1)};
        }
    }
    return {s, ""};
}


string Piece::to_string() const {
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

void Game::push_piece_and_square(size_t position, Color color, PieceType piece_type, size_t& index) {
    pieces.push_back({1ULL << position, color, piece_type});
    squares.push_back(index);
    index++;
}

void Game::push_empty_square() {
    squares.push_back(nullopt);
}

Game Game::initialize() {
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

    game.hash = Zobrist::generate_full_hash(game);

    return game;
}

string Game::to_string() const {
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

Game Game::read_FEN(const string& fen) {
    Game game;
    game.active_color = Color::White;
    game.castling_rights = CastlingRights::ALL;
    game.en_passant = nullopt;
    game.halfmove_clock = 0;
    game.fullmove_number = 1;

    stringstream ss(fen);
    string positions, active_color_str, castling_str, en_passant_str, halfmove_str, fullmove_str;
    ss >> positions >> active_color_str >> castling_str >> en_passant_str >> halfmove_str >> fullmove_str;

    game.squares.assign(64, nullopt);
    size_t piece_index = 0;

    int rank = 7;
    int file = 0;

    for (char c : positions) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            int empty_squares = c - '0';
            file += empty_squares;
        } else {
            size_t square_index = rank * 8 + file;
            Color color = isupper(c) ? Color::White : Color::Black;

            char piece_char = tolower(c);
            PieceType type;

            switch (piece_char) {
                case 'p': type = PieceType::Pawn; break;
                case 'r': type = PieceType::Rook; break;
                case 'n': type = PieceType::Knight; break;
                case 'b': type = PieceType::Bishop; break;
                case 'q': type = PieceType::Queen; break;
                case 'k': type = PieceType::King; break;
                default:
                    throw invalid_argument("Invalid character in FEN string");
            }
            
            game.pieces.push_back({1ULL << square_index, color, type});
            game.squares[square_index] = piece_index;

            piece_index++;
            file++;
        }
    }
    
    if (active_color_str == "w") {
        game.active_color = Color::White;
    } else if (active_color_str == "b") {
        game.active_color = Color::Black;
    } else {
        throw invalid_argument("Invalid active color in FEN string");
    }
    
    game.castling_rights = CastlingRights::NONE; 
    
    if (castling_str != "-") {
        for (char c : castling_str) {
            switch (c) {
                case 'K': game.castling_rights = game.castling_rights | CastlingRights::WHITEKINGSIDE; break;
                case 'Q': game.castling_rights = game.castling_rights | CastlingRights::WHITEQUEENSIDE; break;
                case 'k': game.castling_rights = game.castling_rights | CastlingRights::BLACKKINGSIDE; break;
                case 'q': game.castling_rights = game.castling_rights | CastlingRights::BLACKQUEENSIDE; break;
                default: 
                    throw invalid_argument("Invalid castling character in FEN string");
            }
        }
    }
    
    if (en_passant_str == "-") {
        game.en_passant = nullopt;
    } else {
        int ep_file = en_passant_str[0] - 'a';
        int ep_rank = en_passant_str[1] - '1';
        size_t ep_index = ep_rank * 8 + ep_rank;

        game.en_passant = 1ULL << ep_index;
    }

    game.halfmove_clock = stoi(halfmove_str);
    game.fullmove_number = stoi(fullmove_str);

    game.hash = Zobrist::generate_full_hash(game);

    return game;
}


TEST(FENParserTest, StartingPosition) {
    string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    Game game = Game::read_FEN(starting_fen);
    
    EXPECT_EQ(game.active_color, Color::White);
    EXPECT_EQ(game.halfmove_clock, 0);
    EXPECT_EQ(game.fullmove_number, 1);
    EXPECT_EQ(game.castling_rights, CastlingRights::ALL);
    EXPECT_FALSE(game.en_passant.has_value());
    EXPECT_EQ(game.pieces.size(), 32);
}

int main(int argc, char **argv) {
    Zobrist::init();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
