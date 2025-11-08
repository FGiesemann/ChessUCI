/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/move.h"
#include <catch2/catch_test_macros.hpp>

#include <chesscore_io/move_io.h>

using namespace chessuci;
using namespace chesscore;

namespace {

auto check_move_conversion(const UCIMove &move, const Move &expected_move, const Position &position) -> void {
    auto converted = convert_move(move, position);
    REQUIRE(converted.has_value());
    REQUIRE(converted.value() == expected_move);
}

auto check_unchecked_conversion(const UCIMove &move, const Position &position) -> void {
    auto converted_opt = convert_legal_move(move, position);
    REQUIRE(converted_opt.has_value());
    const auto &converted = converted_opt.value();
    auto reference = convert_move(move, position).value_or(Move{});
    CHECK(converted == reference);
    CHECK(converted.castling_rights_before == reference.castling_rights_before);
    CHECK(converted.halfmove_clock_before == reference.halfmove_clock_before);
    CHECK(converted.en_passant_target_before == reference.en_passant_target_before);
}

} // namespace

TEST_CASE("Move.Conversion.Full", "[move_conversion]") {
    const auto position = Position{FenString{"r3k3/pp4p1/2nb1q1r/8/1Pp3B1/4N3/4P1p1/RN1QK2R b KQq b3 0 1"}};

    check_move_conversion(UCIMove{Square::D6, Square::H2}, Move{.from = Square::D6, .to = Square::H2, .piece = Piece::BlackBishop}, position);
    check_move_conversion(UCIMove{Square::C6, Square::B4}, Move{.from = Square::C6, .to = Square::B4, .piece = Piece::BlackKnight, .captured{Piece::WhitePawn}}, position);
    check_move_conversion(
        UCIMove{Square::G2, Square::G1, PieceType::Queen}, Move{.from = Square::G2, .to = Square::G1, .piece = Piece::BlackPawn, .promoted{Piece::BlackQueen}}, position
    );
    CHECK_FALSE(convert_move(UCIMove{Square::E6, Square::E4}, position).has_value());
}

TEST_CASE("Move.Conversion.Unchecked", "[move_conversion]") {
    const auto position = Position{FenString{"r3k3/pp4p1/2nb1q1r/8/1Pp3B1/4N3/4P1p1/RN1QK2R b KQq b3 0 1"}};

    check_unchecked_conversion(UCIMove{Square::D6, Square::H2}, position);
    check_unchecked_conversion(UCIMove{Square::C6, Square::B4}, position);
    check_unchecked_conversion(UCIMove{Square::G2, Square::G1, PieceType::Queen}, position);
    check_unchecked_conversion(UCIMove{Square::C4, Square::B3}, position);
}
