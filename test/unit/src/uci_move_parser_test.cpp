/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/move.h"
#include <catch2/catch_test_macros.hpp>

using namespace chessuci;
using namespace chesscore;

TEST_CASE("Move.Parser", "[move_parser]") {
    CHECK(parse_uci_move("e2e4") == UCIMove{Square::E2, Square::E4});
    CHECK(parse_uci_move("b2g7") == UCIMove{Square::B2, Square::G7});
    CHECK(parse_uci_move("c2c1q") == UCIMove{Square::C2, Square::C1, PieceType::Queen});
    CHECK(parse_uci_move("e7f8n") == UCIMove{Square::E7, Square::F8, PieceType::Knight});

    CHECK(parse_uci_move("a4") == std::unexpected{UCIParserError{.type = UCIParserErrorType::MissingData, .uci_str = "a4"}});
    CHECK(parse_uci_move("e4xd5") == std::unexpected{UCIParserError{.type = UCIParserErrorType::InvalidFile, .uci_str = "e4xd5"}});
}
