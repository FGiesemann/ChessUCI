/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/move.h"
#include <catch2/catch_test_macros.hpp>

using namespace chessuci;
using namespace chesscore;

TEST_CASE("Move.Matcher.Simple", "[move_matcher]") {
    CHECK(uci_move_matches(UCIMove{Square::E2, Square::E4}, Move{.from = Square::E2, .to = Square::E4, .piece = Piece::WhitePawn}));
    CHECK_FALSE(uci_move_matches(UCIMove{Square::A3, Square::E7}, Move{.from = Square::A4, .to = Square::B4, .piece = Piece::WhitePawn}));
}

TEST_CASE("Move.Matcher.Promotion", "[move_matcher]") {
    CHECK(
        uci_move_matches(UCIMove{Square::C2, Square::C1, PieceType::Queen}, Move{.from = Square::C2, .to = Square::C1, .piece = Piece::BlackPawn, .promoted = Piece::BlackQueen})
    );
    CHECK_FALSE(uci_move_matches(UCIMove{Square::D2, Square::D1}, Move{.from = Square::D2, .to = Square::D1, .piece = Piece::BlackPawn, .promoted = Piece::BlackQueen}));
    CHECK_FALSE(uci_move_matches(UCIMove{Square::E7, Square::E8, PieceType::Queen}, Move{.from = Square::E7, .to = Square::E8, .piece = Piece::WhiteRook}));
}
