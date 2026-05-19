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
#include "movement.hpp"

using namespace std;

 Game game; // auto calls game constructor - Your old Game::initalize

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

Game::Game() 
{
    active_color = Color::White;
    castling_rights = CastlingRights::ALL;
    en_passant = nullopt;
    halfmove_clock = 0;
    fullmove_number = 1;

    size_t piece_index = 0;
    Color color = Color::White;

    push_piece_and_square(0, color, PieceType::Rook, piece_index);
    push_piece_and_square(1, color, PieceType::Knight, piece_index);
    push_piece_and_square(2, color, PieceType::Bishop, piece_index);
    push_piece_and_square(3, color, PieceType::Queen, piece_index);
    push_piece_and_square(4, color, PieceType::King, piece_index);
    push_piece_and_square(5, color, PieceType::Bishop, piece_index);
    push_piece_and_square(6, color, PieceType::Knight, piece_index);
    push_piece_and_square(7, color, PieceType::Rook, piece_index);

    for (int i = 8; i < 16; ++i) {
        push_piece_and_square(i, color, PieceType::Pawn, piece_index);
    }

    for (int i = 16; i < 48; ++i) {
        push_empty_square();
    }

    color = Color::Black;

    for (int i = 48; i < 56; ++i) {
        push_piece_and_square(i, color, PieceType::Pawn, piece_index);
    }

    int offset = 56;
    push_piece_and_square(offset + 0, color, PieceType::Rook, piece_index);
    push_piece_and_square(offset + 1, color, PieceType::Knight, piece_index);
    push_piece_and_square(offset + 2, color, PieceType::Bishop, piece_index);
    push_piece_and_square(offset + 3, color, PieceType::Queen, piece_index);
    push_piece_and_square(offset + 4, color, PieceType::King, piece_index);
    push_piece_and_square(offset + 5, color, PieceType::Bishop, piece_index);
    push_piece_and_square(offset + 6, color, PieceType::Knight, piece_index);
    push_piece_and_square(offset + 7, color, PieceType::Rook, piece_index);

    hash = Zobrist::generate_full_hash(game);
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
Game Game::read_FEN(const string& fen) { //This will have future issues compiling once you call the function...
    Game game_FEN; // Calls constructor
    game_FEN.pieces.clear(); // So need to clear all default pieces for later pushback functions

    game_FEN.active_color = Color::White;
    game_FEN.castling_rights = CastlingRights::ALL;
    game_FEN.en_passant = nullopt;
    game_FEN.halfmove_clock = 0;
    game_FEN.fullmove_number = 1;

    stringstream ss(fen);
    string positions, active_color_str, castling_str, en_passant_str, halfmove_str, fullmove_str;
    ss >> positions >> active_color_str >> castling_str >> en_passant_str >> halfmove_str >> fullmove_str;

    game_FEN.squares.assign(64, nullopt);

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
            
            game_FEN.pieces.push_back({1ULL << square_index, color, type});
            game_FEN.squares[square_index] = piece_index;

            piece_index++;
            file++;
        }
    }
    
    if (active_color_str == "w") {
        game_FEN.active_color = Color::White;
    } else if (active_color_str == "b") {
        game_FEN.active_color = Color::Black;
    } else {
        throw invalid_argument("Invalid active color in FEN string");
    }
    
    game_FEN.castling_rights = CastlingRights::NONE; 
    
    if (castling_str != "-") {
        for (char c : castling_str) {
            switch (c) {
                case 'K': game_FEN.castling_rights = game_FEN.castling_rights | CastlingRights::WHITEKINGSIDE; break;
                case 'Q': game_FEN.castling_rights = game_FEN.castling_rights | CastlingRights::WHITEQUEENSIDE; break;
                case 'k': game_FEN.castling_rights = game_FEN.castling_rights | CastlingRights::BLACKKINGSIDE; break;
                case 'q': game_FEN.castling_rights = game_FEN.castling_rights | CastlingRights::BLACKQUEENSIDE; break;
                default: 
                    throw invalid_argument("Invalid castling character in FEN string");
            }
        }
    }
    
    if (en_passant_str == "-") {
        game_FEN.en_passant = nullopt;
    } else {
        int ep_file = en_passant_str[0] - 'a';
        int ep_rank = en_passant_str[1] - '1';
        size_t ep_index = ep_rank * 8 + ep_file;

        game_FEN.en_passant = 1ULL << ep_index;
    }

    game_FEN.halfmove_clock = stoi(halfmove_str);
    game_FEN.fullmove_number = stoi(fullmove_str);

    game_FEN.hash = Zobrist::generate_full_hash(game_FEN);

    return game_FEN;
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

uint64_t Game::getOccupationBoard() //later can be set to a updated function, as occupation board can be defaulted and then updated per move... later optimization as I practice bitboards better
{
    int referencePosition = 0;
    for (int i = 0; i < pieces.size(); i++)
    {
         occupationBoard = occupationBoard | pieces[i].position; // pieces position is stored in bitboard, or all bitboards together to get complete occupation table.
    }

    return occupationBoard;
}

class MovementTest : public ::testing::Test {
protected:
    void SetUp() override {
        initKnightAttacks();
        initKingAttacks();
        game = Game();
    }
};

TEST_F(MovementTest, KnightInCenterHasEightMoves) {
    uint64_t d4_knight = KnightAttacks[27];
    EXPECT_EQ(__builtin_popcountll(d4_knight), 8);
}

TEST_F(MovementTest, KnightInCornerHasTwoMoves) {
    uint64_t a1_knight = KnightAttacks[0];
    EXPECT_EQ(__builtin_popcountll(a1_knight), 2);
}

TEST_F(MovementTest, WhitePawnDiagonalCaptureInCenter) {
    PiecePosition e4_pawn = 1ULL << 28;
    uint64_t attacks = pawnAttacks(e4_pawn, Color::White);
    
    uint64_t expected_attacks = (1ULL << 35) | (1ULL << 37);
    EXPECT_EQ(attacks, expected_attacks);
}

TEST_F(MovementTest, WhitePawnDiagonalCaptureOnAFileEdge) {
    PiecePosition a2_pawn = 1ULL << 8;
    uint64_t attacks = pawnAttacks(a2_pawn, Color::White);
    
    uint64_t expected_attacks = (1ULL << 17);
    EXPECT_EQ(attacks, expected_attacks);
}

TEST_F(MovementTest, RookOnEmptyBoardCorner) {
    game.occupationBoard = 0ULL; 
    
    PiecePosition h1_rook = 1ULL << 7;
    uint64_t attacks = rookSlide(h1_rook);
    
    EXPECT_EQ(__builtin_popcountll(attacks), 14);
}

TEST(BoardStateTest, StartingOccupationBoardCount) {
    Game new_game = Game();
    uint64_t occ = new_game.getOccupationBoard();
    
    EXPECT_EQ(__builtin_popcountll(occ), 32);
}

int main(int argc, char **argv) {
    Zobrist::init();

    generateKnightAttacks(27);
    initKnightAttacks();
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
